/*
 * libopenraw - ifdfile.cpp
 *
 * Copyright (C) 2006-2015 Hubert Figuiere
 * Copyright (C) 2008 Novell, Inc.
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <stddef.h>

#include <algorithm>
#include <cstdint>
#include <exception>
#include <memory>
#include <numeric>
#include <string>

#include <libopenraw/consts.h>
#include <libopenraw/debug.h>
#include <libopenraw/metadata.h>
#include <libopenraw++/bitmapdata.h>
#include <libopenraw++/rawfile.h>
#include <libopenraw++/rawdata.h>

#include "trace.h"
#include "io/stream.h"
#include "io/streamclone.h"
#include "ifd.h"
#include "ifdentry.h"
#include "ifdfile.h"
#include "ifdfilecontainer.h"
#include "jfifcontainer.h"
#include "rawfile_private.h"
#include "neffile.h" // I wonder if this is smart as it break the abstraction.
#include "unpack.h"

using namespace Debug;

namespace OpenRaw {

class MetaValue;

namespace Internals {


IfdFile::IfdFile(const IO::Stream::Ptr &s, Type _type,
                 bool instantiateContainer)
  : RawFile(_type),
    m_io(s),
    m_container(nullptr)
{
  if(instantiateContainer) {
    m_container = new IfdFileContainer(m_io, 0);
  }
}

IfdFile::~IfdFile()
{
  delete m_container;
}

// this one seems to be pretty much the same for all the
// IFD based raw files
IfdDir::Ref  IfdFile::_locateExifIfd()
{
	const IfdDir::Ref & _mainIfd = mainIfd();
  if (!_mainIfd) {
    Trace(ERROR) << "IfdFile::_locateExifIfd() "
      "main IFD not found\n";
    return IfdDir::Ref();
  }
  return _mainIfd->getExifIFD();
}

MakerNoteDir::Ref  IfdFile::_locateMakerNoteIfd()
{
	const IfdDir::Ref & _exifIfd = exifIfd();
	if(_exifIfd) {
		// to not have a recursive declaration, getMakerNoteIfd() return an IfdDir.
		return std::dynamic_pointer_cast<MakerNoteDir>(_exifIfd->getMakerNoteIfd());
	}
	return MakerNoteDir::Ref();
}

void IfdFile::_identifyId()
{
	const IfdDir::Ref & _mainIfd = mainIfd();
  if(!_mainIfd) {
    Trace(ERROR) << "Main IFD not found to identify the file.\n";
    return;
  }
  std::string make, model;
  if(_mainIfd->getValue(IFD::EXIF_TAG_MAKE, make) && 
    _mainIfd->getValue(IFD::EXIF_TAG_MODEL, model)) {
    _setTypeId(_typeIdFromModel(make, model));
  }
}



::or_error IfdFile::_enumThumbnailSizes(std::vector<uint32_t> &list)
{
  ::or_error err = OR_ERROR_NONE;

  Trace(DEBUG1) << "_enumThumbnailSizes()\n";
  std::vector<IfdDir::Ref> & dirs = m_container->directories();

  Trace(DEBUG1) << "num of dirs " << dirs.size() << "\n";
  for(auto iter = dirs.begin(); iter != dirs.end(); ++iter)
  {
    IfdDir::Ref & dir = *iter;
    dir->load();
    or_error ret = _locateThumbnail(dir, list);
    if (ret == OR_ERROR_NONE)
    {
      Trace(DEBUG1) << "Found " << list.back() << " pixels\n";
    }
    std::vector<IfdDir::Ref> subdirs;
    if(dir->getSubIFDs(subdirs)) {
      Trace(DEBUG1) << "Iterating subdirs\n";
      for(auto iter2 = subdirs.begin(); iter2 != subdirs.end();
          ++iter2)
      {
        IfdDir::Ref & dir2 = *iter2;
        dir2->load();
        ret = _locateThumbnail(dir2, list);
        if (ret == OR_ERROR_NONE)
        {
          Trace(DEBUG1) << "Found " << list.back() << " pixels\n";
        }
      }
    }
  }
  if (list.size() <= 0) {
    err = OR_ERROR_NOT_FOUND;
  }
  return err;
}


::or_error IfdFile::_locateThumbnail(const IfdDir::Ref & dir,
                                     std::vector<uint32_t> &list)
{
  ::or_error ret = OR_ERROR_NOT_FOUND;
  bool got_it;
  uint32_t x = 0;
  uint32_t y = 0;
  ::or_data_type _type = OR_DATA_TYPE_NONE;
  uint32_t subtype = 0;

  Trace(DEBUG1) << "_locateThumbnail\n";

  got_it = dir->getValue(IFD::EXIF_TAG_NEW_SUBFILE_TYPE, subtype);
  Trace(DEBUG1) << "subtype " << subtype  << "\n";
  if(!got_it) {
    if(!m_cfaIfd) {
      m_cfaIfd = _locateCfaIfd();
    }
    if(m_cfaIfd == dir) {
      return OR_ERROR_NOT_FOUND;
    }
    else {
      subtype = 1;
    }
  }
  if (subtype == 1) {

    uint16_t photom_int = 0;
    got_it = dir->getValue(IFD::EXIF_TAG_PHOTOMETRIC_INTERPRETATION,
                           photom_int);

    if (got_it) {
      Trace(DEBUG1) << "photometric int " << photom_int  << "\n";
    }
    // photometric interpretation is RGB by default
    else {
      photom_int = IFD::EV_PI_RGB;
      Trace(DEBUG1) << "assume photometric int is RGB\n";
    }

    got_it = dir->getIntegerValue(IFD::EXIF_TAG_IMAGE_WIDTH, x);
    got_it = dir->getIntegerValue(IFD::EXIF_TAG_IMAGE_LENGTH, y);

    uint16_t compression = 0;
    got_it = dir->getValue(IFD::EXIF_TAG_COMPRESSION, compression);

    uint32_t offset = 0;
    uint32_t byte_count = 0;
    got_it = dir->getValue(IFD::EXIF_TAG_STRIP_BYTE_COUNTS, byte_count);
    got_it = dir->getValue(IFD::EXIF_TAG_STRIP_OFFSETS, offset);
    if (!got_it || (compression == 6) || (compression == 7)) {
      if(!got_it) {
        got_it = dir->getValue(IFD::EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LENGTH,
                               byte_count);
        got_it = dir->getValue(IFD::EXIF_TAG_JPEG_INTERCHANGE_FORMAT,
                               offset);
      }
      if (got_it) {
        // workaround for CR2 files where 8RGB data is marked
        // as JPEG. Check the real data size.
        if(x && y) {
          if(byte_count >= (x * y * 3)) {
            //_type = OR_DATA_TYPE_PIXMAP_8RGB;
            _type = OR_DATA_TYPE_NONE;
            // See bug 72270
            Trace(DEBUG1) << "8RGB as JPEG. Will ignore.\n";
            ret = OR_ERROR_INVALID_FORMAT;
          }
          else {
            _type = OR_DATA_TYPE_JPEG;
          }
        }
        else {
          _type = OR_DATA_TYPE_JPEG;
          Trace(DEBUG1) << "looking for JPEG at " << offset << "\n";
          if (x == 0 || y == 0) {
            IO::Stream::Ptr s(std::make_shared<IO::StreamClone>(
                                m_io, offset));
            std::unique_ptr<JfifContainer> jfif(new JfifContainer(s, 0));
            if (jfif->getDimensions(x,y)) {
              Trace(DEBUG1) << "JPEG dimensions x=" << x
                            << " y=" << y << "\n";
            }
            else {
              _type = OR_DATA_TYPE_NONE;
              Trace(WARNING) << "Couldn't get JPEG "
                "dimensions.\n";
            }
          }
          else {
            Trace(DEBUG1) << "JPEG (supposed) dimensions x=" << x
                          << " y=" << y << "\n";
          }
        }

      }
    }
    else if (photom_int == IFD::EV_PI_YCBCR) {
      Trace(WARNING) << "Unsupported YCbCr photometric "
        "interpretation in non JPEG.\n";
      ret = OR_ERROR_INVALID_FORMAT;
    }
    else {
      Trace(DEBUG1) << "found strip offsets\n";
      if (x != 0 && y != 0) {
        // See bug 72270 - some CR2 have 16 bpc RGB thumbnails.
        // by default it is RGB8. Unless stated otherwise.
        bool isRGB8 = true;
        try {
          IfdEntry::Ref entry = dir->getEntry(IFD::EXIF_TAG_BITS_PER_SAMPLE);
          std::vector<uint16_t> arr;
          entry->getArray(arr);
          for(auto i = arr.cbegin(); i != arr.cend(); ++i) {
            isRGB8 = *i == 8;
            if (!isRGB8) {
              Trace(DEBUG1) << "bpc != 8, not RGB8 " << *i << "\n";
              break;
            }
          }
        }
        catch(const std::exception & e) {
          Trace(DEBUG1) << "Exception getting BPS " << e.what() << "\n";
        }
        if (isRGB8) {
          _type = OR_DATA_TYPE_PIXMAP_8RGB;
        }
      }
    }
    if(_type != OR_DATA_TYPE_NONE) {
      uint32_t dim = std::max(x, y);
      offset += dir->container().offset();
      _addThumbnail(dim, ThumbDesc(x, y, _type,
                                   offset, byte_count));
      list.push_back(dim);
      ret = OR_ERROR_NONE;
    }
  }

  return ret;
}

RawContainer* IfdFile::getContainer() const
{
  return m_container;
}

uint32_t IfdFile::_getJpegThumbnailOffset(const IfdDir::Ref & dir, uint32_t & byte_length)
{
  uint32_t offset = 0;
  bool got_it = dir->getValue(IFD::EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LENGTH, byte_length);
  if(got_it) {
    got_it = dir->getValue(IFD::EXIF_TAG_JPEG_INTERCHANGE_FORMAT, offset);
  }
  else {
    // some case it is STRIP_OFFSETS for JPEG
    got_it = dir->getValue(IFD::EXIF_TAG_STRIP_OFFSETS, offset);
    got_it = dir->getValue(IFD::EXIF_TAG_STRIP_BYTE_COUNTS, byte_length);
  }
  return offset;
}



MetaValue *IfdFile::_getMetaValue(int32_t meta_index)
{
  MetaValue * val = nullptr;
  IfdDir::Ref ifd;
  if(META_INDEX_MASKOUT(meta_index) == META_NS_TIFF) {
    ifd = mainIfd();
  }
  else if(META_INDEX_MASKOUT(meta_index) == META_NS_EXIF) {
    ifd = exifIfd();
  }
  else {
    Trace(ERROR) << "Unknown Meta Namespace\n";
  }
  if(ifd) {
    Trace(DEBUG1) << "Meta value for "
                  << META_NS_MASKOUT(meta_index) << "\n";

    IfdEntry::Ref e = ifd->getEntry(META_NS_MASKOUT(meta_index));
    if(e) {
      val = e->make_meta_value();
    }
  }
  return val;
}

/** by default we don't translate the compression
 */
uint32_t IfdFile::_translateCompressionType(IFD::TiffCompress tiffCompression)
{
	return (uint32_t)tiffCompression;
}



const IfdDir::Ref & IfdFile::cfaIfd()
{
	if(!m_cfaIfd) {
		m_cfaIfd = _locateCfaIfd();
	}
	return m_cfaIfd;
}


const IfdDir::Ref & IfdFile::mainIfd()
{
	if(!m_mainIfd) {
		m_mainIfd = _locateMainIfd();
	}
	return m_mainIfd;
}


const IfdDir::Ref & IfdFile::exifIfd()
{
	if(!m_exifIfd) {
		m_exifIfd = _locateExifIfd();
	}
	return m_exifIfd;
}


const MakerNoteDir::Ref & IfdFile::makerNoteIfd()
{
	if(!m_makerNoteIfd) {
		m_makerNoteIfd = _locateMakerNoteIfd();
	}
	return m_makerNoteIfd;
}


namespace {

::or_cfa_pattern
_convertArrayToCfaPattern(const std::vector<uint8_t> &cfaPattern)
{
  ::or_cfa_pattern cfa_pattern = OR_CFA_PATTERN_NON_RGB22;
  if(cfaPattern.size() != 4) {
    Trace(WARNING) << "Unsupported bayer pattern\n";
  }
  else {
    Trace(DEBUG2) << "patter is = " << cfaPattern[0] << ", "
                  << cfaPattern[1] << ", " << cfaPattern[2]
                  << ", " << cfaPattern[3] << "\n";
    switch(cfaPattern[0]) {
    case IFD::CFA_RED:
      switch(cfaPattern[1]) {
      case IFD::CFA_GREEN:
        if((cfaPattern[2] == IFD::CFA_GREEN)
           && (cfaPattern[3] == IFD::CFA_BLUE))
        {
          cfa_pattern = OR_CFA_PATTERN_RGGB;
        }
        break;
      }
      break;
    case IFD::CFA_GREEN:
      switch(cfaPattern[1]) {
      case IFD::CFA_RED:
        if((cfaPattern[2] == 2)
           && (cfaPattern[3] == IFD::CFA_GREEN))
        {
          cfa_pattern = OR_CFA_PATTERN_GRBG;
        }
        break;
      case 2:
        if((cfaPattern[2] == IFD::CFA_RED)
           && (cfaPattern[3] == IFD::CFA_GREEN))
        {
          cfa_pattern = OR_CFA_PATTERN_GBRG;
        }
        break;
      }
      break;
    case IFD::CFA_BLUE:
      switch(cfaPattern[1]) {
      case IFD::CFA_GREEN:
        if((cfaPattern[2] == IFD::CFA_GREEN)
           && (cfaPattern[3] == IFD::CFA_RED))
        {
          cfa_pattern = OR_CFA_PATTERN_BGGR;
        }
        break;
      }
      break;
    }
    //
  }
  return cfa_pattern;
}

::or_cfa_pattern _convertNewCfaPattern(const IfdEntry::Ref & e)
{
  ::or_cfa_pattern cfa_pattern = OR_CFA_PATTERN_NONE;
  if(!e || (e->count() < 4)) {
    return cfa_pattern;
  }

  uint16_t hdim = IfdTypeTrait<uint16_t>::get(*e, 0, true);
  uint16_t vdim = IfdTypeTrait<uint16_t>::get(*e, 1, true);
  if(hdim != 2 && vdim != 2) {
    cfa_pattern = OR_CFA_PATTERN_NON_RGB22;
  }
  else {
    std::vector<uint8_t> cfaPattern;
    cfaPattern.push_back(IfdTypeTrait<uint8_t>::get(*e, 4, true));
    cfaPattern.push_back(IfdTypeTrait<uint8_t>::get(*e, 5, true));
    cfaPattern.push_back(IfdTypeTrait<uint8_t>::get(*e, 6, true));
    cfaPattern.push_back(IfdTypeTrait<uint8_t>::get(*e, 7, true));
    cfa_pattern = _convertArrayToCfaPattern(cfaPattern);
  }
  return cfa_pattern;
}


/** convert the CFA Pattern as stored in the entry */
::or_cfa_pattern _convertCfaPattern(const IfdEntry::Ref & e)
{
  std::vector<uint8_t> cfaPattern;
  ::or_cfa_pattern cfa_pattern = OR_CFA_PATTERN_NONE;

  e->getArray(cfaPattern);
  if(!cfaPattern.empty()) {
    cfa_pattern = _convertArrayToCfaPattern(cfaPattern);
  }
  return cfa_pattern;
}

/** get the CFA Pattern out of the directory
 * @param dir the directory
 * @return the cfa_pattern value. %OR_CFA_PATTERN_NONE mean that
 * nothing has been found.
 */
static ::or_cfa_pattern _getCfaPattern(const IfdDir::Ref & dir)
{
  Trace(DEBUG1) << __FUNCTION__ << "\n";
  ::or_cfa_pattern cfa_pattern = OR_CFA_PATTERN_NONE;
  try {
    IfdEntry::Ref e = dir->getEntry(IFD::EXIF_TAG_CFA_PATTERN);
    if(e) {
      cfa_pattern = _convertCfaPattern(e);
    }
    else {
      e = dir->getEntry(IFD::EXIF_TAG_NEW_CFA_PATTERN);
      if(e)  {
        cfa_pattern = _convertNewCfaPattern(e);
      }
    }
  }
  catch(...)
  {
    Trace(ERROR) << "Exception in _getCfaPattern().\n";
  }
  return cfa_pattern;
}

} // end anon namespace


::or_error IfdFile::_getRawData(RawData & data, uint32_t options)
{
  ::or_error ret = OR_ERROR_NONE;
  const IfdDir::Ref & _cfaIfd = cfaIfd();
  Trace(DEBUG1) << "_getRawData()\n";

  if(_cfaIfd) {
    ret = _getRawDataFromDir(data, _cfaIfd);
    if (ret != OR_ERROR_NONE) {
      return ret;
    }
    ret = _decompressIfNeeded(data, options);
  }
  else {
    ret = OR_ERROR_NOT_FOUND;
  }
  return ret;
}

::or_error IfdFile::_decompressIfNeeded(RawData&, uint32_t)
{
  return OR_ERROR_NONE;
}


::or_error IfdFile::_getRawDataFromDir(RawData & data, const IfdDir::Ref & dir)
{
  ::or_error ret = OR_ERROR_NONE;

  uint16_t bpc = 0;
  uint32_t offset = 0;
  uint32_t byte_length = 0;
  bool got_it;
  uint32_t x, y;
  x = 0;
  y = 0;

  if(!dir) {
    Trace(ERROR) << "dir is NULL\n";
    return OR_ERROR_NOT_FOUND;
  }
  got_it = dir->getValue(IFD::EXIF_TAG_BITS_PER_SAMPLE, bpc);
  if(!got_it) {
    Trace(ERROR) << "unable to guess Bits per sample\n";
  }

  got_it = dir->getValue(IFD::EXIF_TAG_STRIP_OFFSETS, offset);
  if(got_it) {
    IfdEntry::Ref e = dir->getEntry(IFD::EXIF_TAG_STRIP_BYTE_COUNTS);
    if(!e) {
      Trace(DEBUG1) << "byte len not found\n";
      return OR_ERROR_NOT_FOUND;
    }
    std::vector<uint32_t> counts;
    e->getArray(counts);
    Trace(DEBUG1) << "counting tiles\n";
    byte_length = std::accumulate(counts.cbegin(), counts.cend(), 0);
  }
  else {
    // the tile are individual JPEGS....
    // TODO extract all of them.
    IfdEntry::Ref e = dir->getEntry(IFD::TIFF_TAG_TILE_OFFSETS);
    if(!e) {
      Trace(DEBUG1) << "tile offsets empty\n";
      return OR_ERROR_NOT_FOUND;
    }
    std::vector<uint32_t> offsets;
    e->getArray(offsets);
    if(offsets.size() == 0) {
      Trace(DEBUG1) << "tile offsets not found\n";
      return OR_ERROR_NOT_FOUND;
    }
    offset = offsets[0];
    e = dir->getEntry(IFD::TIFF_TAG_TILE_BYTECOUNTS);
    if(!e) {
      Trace(DEBUG1) << "tile byte counts not found\n";
      return OR_ERROR_NOT_FOUND;
    }
    std::vector<uint32_t> counts;
    e->getArray(counts);
    Trace(DEBUG1) << "counting tiles\n";
    byte_length = std::accumulate(counts.cbegin(), counts.cend(), 0);
  }
  got_it = dir->getIntegerValue(IFD::EXIF_TAG_IMAGE_WIDTH, x);
  if(!got_it) {
    Trace(DEBUG1) << "X not found\n";
    return OR_ERROR_NOT_FOUND;
  }
  got_it = dir->getIntegerValue(IFD::EXIF_TAG_IMAGE_LENGTH, y);
  if(!got_it) {
    Trace(DEBUG1) << "Y not found\n";
    return OR_ERROR_NOT_FOUND;
  }

  uint32_t photo_int = 0;
  got_it = dir->getIntegerValue(IFD::EXIF_TAG_PHOTOMETRIC_INTERPRETATION,
                                photo_int);
  if(!got_it) {
    // Default is CFA.
    photo_int = IFD::EV_PI_CFA;
  }

  uint16_t tiffCompression = 0;
  got_it = dir->getValue(IFD::EXIF_TAG_COMPRESSION, tiffCompression);
  if(!got_it) {
    Trace(DEBUG1) << "Compression type not found\n";
  }
  BitmapData::DataType data_type = OR_DATA_TYPE_NONE;

	uint32_t compression = _translateCompressionType((IFD::TiffCompress)tiffCompression);
  switch(compression)
  {
  case IFD::COMPRESS_NONE:
    data_type = OR_DATA_TYPE_RAW;
    break;
  case IFD::COMPRESS_NIKON_PACK:
    data_type = OR_DATA_TYPE_RAW;
    break;
  case IFD::COMPRESS_NIKON_QUANTIZED:
    // must check whether it is really compressed
    // only for D100
    if( !NefFile::isCompressed(*m_container, offset) ) {
      compression = IFD::COMPRESS_NIKON_PACK;
      data_type = OR_DATA_TYPE_RAW;
      // this is a hack. we should check if
      // we have a D100 instead, but that case is already
      // a D100 corner case. WILL BREAK on compressed files.
      // according to dcraw we must increase the size by 6.
      x += 6;
      break;
    }
  default:
    data_type = OR_DATA_TYPE_COMPRESSED_RAW;
    break;
  }

  Trace(DEBUG1) << "RAW Compression is " << compression << "\n";

  ::or_cfa_pattern cfa_pattern = _getCfaPattern(dir);
  if(cfa_pattern == OR_CFA_PATTERN_NONE) {
    // some file have it in the exif IFD instead.
    if(!m_exifIfd) {
      m_exifIfd = _locateExifIfd();
    }
    cfa_pattern = _getCfaPattern(m_exifIfd);
  }


  if((bpc == 12 || bpc == 14) && (compression == 1)
     && (byte_length == (x * y * 2)))
  {
    Trace(DEBUG1) << "setting bpc from " << bpc
                  << " to 16\n";
    bpc = 16;
  }
  if((bpc == 16) || (data_type == OR_DATA_TYPE_COMPRESSED_RAW)) {
    void *p = data.allocData(byte_length);
    size_t real_size = m_container->fetchData(p, offset,
                                              byte_length);
    if (real_size < byte_length) {
      Trace(WARNING) << "Size mismatch for data: ignoring.\n";
    }
  }
  else if((bpc == 12) || (bpc == 8)) {
    ret = _unpackData(bpc, compression, data, x, y, offset, byte_length);
    Trace(DEBUG1) << "unpack result " << ret << "\n";
  }
  else {
    Trace(ERROR) << "Unsupported bpc " << bpc << "\n";
    return OR_ERROR_INVALID_FORMAT;
  }
  data.setCfaPatternType(cfa_pattern);
  data.setDataType(data_type);
  data.setCompression(data_type == OR_DATA_TYPE_COMPRESSED_RAW
                      ? compression : 1);
  data.setPhotometricInterpretation((ExifPhotometricInterpretation)photo_int);
  if((data_type == OR_DATA_TYPE_RAW) && (data.whiteLevel() == 0)) {
    data.setWhiteLevel((1 << bpc) - 1);
  }
  data.setDimensions(x, y);

  return ret;
}


::or_error IfdFile::_unpackData(uint16_t bpc, uint32_t compression, RawData & data, uint32_t x, uint32_t y, uint32_t offset, uint32_t byte_length)
{
  ::or_error ret = OR_ERROR_NONE;
  size_t fetched = 0;
  uint32_t current_offset = offset;
  Unpack unpack(x, compression);
  const size_t blocksize = (bpc == 8 ? x : unpack.block_size());
  Trace(DEBUG1) << "Block size = " << blocksize << "\n";
  Trace(DEBUG1) << "dimensions (x, y) " << x << ", "
                << y << "\n";
  std::unique_ptr<uint8_t[]> block(new uint8_t[blocksize]);
  size_t outsize = x * y * 2;
  uint8_t * outdata = (uint8_t*)data.allocData(outsize);
  size_t got;
  Trace(DEBUG1) << "offset of RAW data = " << current_offset << "\n";
  do {
    got = m_container->fetchData (block.get(),
                                  current_offset, blocksize);
    fetched += got;
    offset += got;
    current_offset += got;
    if(got) {
      if(bpc == 12) {
        size_t out;
        ret = unpack.unpack_be12to16(outdata, outsize,
                                     block.get(),
                                     got, out);
        outdata += out;
        outsize -= out;
        if(ret != OR_ERROR_NONE) {
          break;
        }
      }
      else {
        // outdata point to uint16_t
        std::copy(block.get(), block.get()+got,
                  (uint16_t*)outdata);
        outdata += (got << 1);
      }
    }
  } while((got != 0) && (fetched < byte_length));

  return ret;
}

}
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  tab-width:2
  c-basic-offset:2
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
