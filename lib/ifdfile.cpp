/*
 * libopenraw - ifdfile.cpp
 *
 * Copyright (C) 2006-2017 Hubert Figuière
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

#include "bitmapdata.hpp"
#include "rawfile.hpp"
#include "rawdata.hpp"
#include "trace.hpp"
#include "io/stream.hpp"
#include "io/streamclone.hpp"
#include "ifd.hpp"
#include "ifdentry.hpp"
#include "ifdfile.hpp"
#include "ifdfilecontainer.hpp"
#include "jfifcontainer.hpp"
#include "rawfile_private.hpp"
#include "neffile.hpp" // I wonder if this is smart as it break the abstraction.
#include "unpack.hpp"

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
    LOGERR("IfdFile::_locateExifIfd() main IFD not found\n");
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
  if (!_mainIfd) {
    LOGERR("Main IFD not found to identify the file.\n");
    return;
  }

  auto make = _mainIfd->getValue<std::string>(IFD::EXIF_TAG_MAKE);
  auto model = _mainIfd->getValue<std::string>(IFD::EXIF_TAG_MODEL);
  if (make.ok() && model.ok()) {
    _setTypeId(_typeIdFromModel(make.unwrap(), model.unwrap()));
  }
}



::or_error
IfdFile::_enumThumbnailSizes(std::vector<uint32_t> &list)
{
  ::or_error err = OR_ERROR_NONE;

  LOGDBG1("_enumThumbnailSizes()\n");
  std::vector<IfdDir::Ref> & dirs = m_container->directories();

  LOGDBG1("num of dirs %lu\n", dirs.size());
  for(auto dir : dirs)
  {
    dir->load();
    or_error ret = _locateThumbnail(dir, list);
    if (ret == OR_ERROR_NONE) {
      LOGDBG1("Found %u pixels\n", list.back());
    }
    auto result = dir->getSubIFDs();
    if (result.ok()) {
      std::vector<IfdDir::Ref> subdirs = result.unwrap();
      LOGDBG1("Iterating subdirs\n");
      for(auto dir2 : subdirs)
      {
        dir2->load();
        ret = _locateThumbnail(dir2, list);
        if (ret == OR_ERROR_NONE) {
          LOGDBG1("Found %u pixels\n", list.back());
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
  ::or_data_type _type = OR_DATA_TYPE_NONE;
  uint32_t subtype = 0;

  LOGDBG1("_locateThumbnail\n");

  auto result = dir->getValue<uint32_t>(IFD::EXIF_TAG_NEW_SUBFILE_TYPE);
  if (result.empty()) {
    if(!m_cfaIfd) {
      m_cfaIfd = _locateCfaIfd();
    }
    if(m_cfaIfd == dir) {
      return OR_ERROR_NOT_FOUND;
    }
    else {
      subtype = 1;
    }
  } else {
    subtype = result.unwrap();
  }
  LOGDBG1("subtype %u\n", subtype);
  if (subtype == 1) {

    uint16_t photom_int =
      dir->getValue<uint16_t>(IFD::EXIF_TAG_PHOTOMETRIC_INTERPRETATION).unwrap_or(IFD::EV_PI_RGB);

    uint32_t x = dir->getIntegerValue(IFD::EXIF_TAG_IMAGE_WIDTH).unwrap_or(0);
    uint32_t y = dir->getIntegerValue(IFD::EXIF_TAG_IMAGE_LENGTH).unwrap_or(0);

    uint16_t compression = dir->getValue<uint16_t>(IFD::EXIF_TAG_COMPRESSION).unwrap_or(0);

    uint32_t offset = 0;
    uint32_t byte_count = dir->getValue<uint32_t>(IFD::EXIF_TAG_STRIP_BYTE_COUNTS).unwrap_or(0);

    result = dir->getValue<uint32_t>(IFD::EXIF_TAG_STRIP_OFFSETS);
    bool got_it = result.ok();
    if (result.ok()) {
      offset = result.unwrap();
    }
    if (!got_it || (compression == 6) || (compression == 7)) {
      if (!got_it) {
        byte_count =
          dir->getValue<uint32_t>(IFD::EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LENGTH).unwrap_or(0);
        result = dir->getValue<uint32_t>(IFD::EXIF_TAG_JPEG_INTERCHANGE_FORMAT);
        got_it = result.ok();
        if (got_it) {
          offset = result.unwrap();
        }
      }
      if (got_it) {
        // workaround for CR2 files where 8RGB data is marked
        // as JPEG. Check the real data size.
        if(x && y) {
          if(byte_count >= (x * y * 3)) {
            //_type = OR_DATA_TYPE_PIXMAP_8RGB;
            _type = OR_DATA_TYPE_NONE;
            // See bug 72270
            LOGDBG1("8RGB as JPEG. Will ignore.\n");
            ret = OR_ERROR_INVALID_FORMAT;
          }
          else {
            _type = OR_DATA_TYPE_JPEG;
          }
        }
        else {
          _type = OR_DATA_TYPE_JPEG;
          LOGDBG1("looking for JPEG at %u\n", offset);
          if (x == 0 || y == 0) {
            IO::Stream::Ptr s(std::make_shared<IO::StreamClone>(
                                m_io, offset));
            std::unique_ptr<JfifContainer> jfif(new JfifContainer(s, 0));
            if (jfif->getDimensions(x,y)) {
              LOGDBG1("JPEG dimensions x=%u y=%u\n", x, y);
            }
            else {
              _type = OR_DATA_TYPE_NONE;
              LOGWARN("Couldn't get JPEG dimensions.\n");
            }
          }
          else {
            LOGDBG1("JPEG (supposed) dimensions x=%u y=%u\n", x, y);
          }
        }

      }
    }
    else if (photom_int == IFD::EV_PI_YCBCR) {
      LOGWARN("Unsupported YCbCr photometric interpretation in non JPEG.\n");
      ret = OR_ERROR_INVALID_FORMAT;
    }
    else {
      LOGDBG1("found strip offsets\n");
      if (x != 0 && y != 0) {
        // See bug 72270 - some CR2 have 16 bpc RGB thumbnails.
        // by default it is RGB8. Unless stated otherwise.
        bool isRGB8 = true;
        IfdEntry::Ref entry = dir->getEntry(IFD::EXIF_TAG_BITS_PER_SAMPLE);
        auto result2 = entry->getArray<uint16_t>();
        if (result2.ok()) {
          std::vector<uint16_t> arr = result2.unwrap();
          for(auto bpc : arr) {
            isRGB8 = (bpc == 8);
            if (!isRGB8) {
              LOGDBG1("bpc != 8, not RGB8 %u\n", bpc);
              break;
            }
          }
        } else {
          LOGDBG1("Error getting BPS\n");
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
  auto result = dir->getValue<uint32_t>(IFD::EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LENGTH);
  if (result.ok()) {
    byte_length = result.unwrap();
    return dir->getValue<uint32_t>(IFD::EXIF_TAG_JPEG_INTERCHANGE_FORMAT).unwrap_or(0);
  }

  // some case it is STRIP_OFFSETS for JPEG
  byte_length = dir->getValue<uint32_t>(IFD::EXIF_TAG_STRIP_BYTE_COUNTS).unwrap_or(0);
  return dir->getValue<uint32_t>(IFD::EXIF_TAG_STRIP_OFFSETS).unwrap_or(0);
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
    LOGERR("Unknown Meta Namespace\n");
  }
  if(ifd) {
    LOGDBG1("Meta value for %u\n", META_NS_MASKOUT(meta_index));

    IfdEntry::Ref e = ifd->getEntry(META_NS_MASKOUT(meta_index));
    if(e) {
      val = e->make_meta_value();
    }
  }
  return val;
}

/** by default we don't translate the compression
 */
uint32_t IfdFile::_translateCompressionType(IFD::TiffCompress tiff_compression)
{
	return (uint32_t)tiff_compression;
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
    LOGWARN("Unsupported bayer pattern\n");
  }
  else {
    LOGDBG2("pattern is = %d, %d, %d, %d\n", cfaPattern[0],
            cfaPattern[1], cfaPattern[2], cfaPattern[3]);
    switch(cfaPattern[0]) {
    case IFD::CFA_RED:
      if ((cfaPattern[1] == IFD::CFA_GREEN)
          && (cfaPattern[2] == IFD::CFA_GREEN)
          && (cfaPattern[3] == IFD::CFA_BLUE)) {
        cfa_pattern = OR_CFA_PATTERN_RGGB;
      }
      break;
    case IFD::CFA_GREEN:
      switch(cfaPattern[1]) {
      case IFD::CFA_RED:
        if ((cfaPattern[2] == IFD::CFA_BLUE)
            && (cfaPattern[3] == IFD::CFA_GREEN)) {
          cfa_pattern = OR_CFA_PATTERN_GRBG;
        }
        break;
      case 2:
        if ((cfaPattern[2] == IFD::CFA_RED)
            && (cfaPattern[3] == IFD::CFA_GREEN)) {
          cfa_pattern = OR_CFA_PATTERN_GBRG;
        }
        break;
      }
      break;
    case IFD::CFA_BLUE:
      if ((cfaPattern[1] ==IFD::CFA_GREEN)
          && (cfaPattern[2] == IFD::CFA_GREEN)
          && (cfaPattern[3] == IFD::CFA_RED)) {
        cfa_pattern = OR_CFA_PATTERN_BGGR;
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
  ::or_cfa_pattern cfa_pattern = OR_CFA_PATTERN_NONE;

  auto result = e->getArray<uint8_t>();
  if (result.ok()) {
    cfa_pattern = _convertArrayToCfaPattern(result.unwrap());
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
  LOGDBG1("%s\n", __FUNCTION__);
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
    LOGERR("Exception in _getCfaPattern().\n");
  }
  return cfa_pattern;
}

} // end anon namespace


::or_error IfdFile::_getRawData(RawData & data, uint32_t options)
{
  ::or_error ret = OR_ERROR_NONE;
  const IfdDir::Ref & _cfaIfd = cfaIfd();
  LOGDBG1("_getRawData()\n");

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

  uint32_t offset = 0;
  uint32_t byte_length = 0;

  if(!dir) {
    LOGERR("dir is NULL\n");
    return OR_ERROR_NOT_FOUND;
  }
  auto result = dir->getValue<uint16_t>(IFD::EXIF_TAG_BITS_PER_SAMPLE);
  if(result.empty()) {
    LOGERR("unable to guess Bits per sample\n");
  }
  uint16_t bpc = result.unwrap_or(0);

  auto result2 = dir->getValue<uint32_t>(IFD::EXIF_TAG_STRIP_OFFSETS);
  if(result2.ok()) {
    offset = result2.unwrap();
    IfdEntry::Ref e = dir->getEntry(IFD::EXIF_TAG_STRIP_BYTE_COUNTS);
    if(!e) {
      LOGDBG1("byte len not found\n");
      return OR_ERROR_NOT_FOUND;
    }
    auto result3 = e->getArray<uint32_t>();
    if (result3.ok()) {
      std::vector<uint32_t> counts = result3.unwrap();
      LOGDBG1("counting tiles\n");
      byte_length = std::accumulate(counts.cbegin(), counts.cend(), 0);
    }
  }
  else {
    // the tile are individual JPEGS....
    // TODO extract all of them.
    IfdEntry::Ref e = dir->getEntry(IFD::TIFF_TAG_TILE_OFFSETS);
    if(!e) {
      LOGDBG1("tile offsets empty\n");
      return OR_ERROR_NOT_FOUND;
    }
    auto result3 = e->getArray<uint32_t>();
    if (result3.empty()) {
      LOGDBG1("tile offsets not found\n");
      return OR_ERROR_NOT_FOUND;
    }
    std::vector<uint32_t> offsets = result3.unwrap();
    offset = offsets[0];
    e = dir->getEntry(IFD::TIFF_TAG_TILE_BYTECOUNTS);
    if(!e) {
      LOGDBG1("tile byte counts not found\n");
      return OR_ERROR_NOT_FOUND;
    }
    result3 = e->getArray<uint32_t>();
    if (result3.ok()) {
      std::vector<uint32_t> counts = result3.unwrap();
      LOGDBG1("counting tiles\n");
      byte_length = std::accumulate(counts.cbegin(), counts.cend(), 0);
    }
  }

  result2 = dir->getIntegerValue(IFD::EXIF_TAG_IMAGE_WIDTH);
  if(result2.empty()) {
    LOGDBG1("X not found\n");
    return OR_ERROR_NOT_FOUND;
  }
  uint32_t x = result2.unwrap();

  result2 = dir->getIntegerValue(IFD::EXIF_TAG_IMAGE_LENGTH);
  if(result2.empty()) {
    LOGDBG1("Y not found\n");
    return OR_ERROR_NOT_FOUND;
  }
  uint32_t y = result2.unwrap();

  uint32_t photo_int
    = dir->getIntegerValue(IFD::EXIF_TAG_PHOTOMETRIC_INTERPRETATION)
    .unwrap_or(IFD::EV_PI_CFA);

  BitmapData::DataType data_type = OR_DATA_TYPE_NONE;

  result = dir->getValue<uint16_t>(IFD::EXIF_TAG_COMPRESSION);
  if(result.empty()) {
    LOGDBG1("Compression type not found\n");
  }
	uint32_t compression = _translateCompressionType(
    static_cast<IFD::TiffCompress>(result.unwrap_or(0)));

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
    if (!NefFile::isCompressed(*m_container, offset)) {
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

  LOGDBG1("RAW Compression is %u\n", compression);
  LOGDBG1("bpc is %u\n", bpc);

  ::or_cfa_pattern cfa_pattern = _getCfaPattern(dir);
  if(cfa_pattern == OR_CFA_PATTERN_NONE) {
    // some file have it in the exif IFD instead.
    if(!m_exifIfd) {
      m_exifIfd = _locateExifIfd();
    }
    cfa_pattern = _getCfaPattern(m_exifIfd);
  }


  if((bpc == 12 || bpc == 14) && (compression == IFD::COMPRESS_NONE)
     && (byte_length == (x * y * 2))) {
    // We turn this to a 16-bits per sample. MSB are 0
    LOGDBG1("setting bpc from %u to 16\n", bpc);
    bpc = 16;
  }
  if((bpc == 16) || (data_type == OR_DATA_TYPE_COMPRESSED_RAW)) {
    void *p = data.allocData(byte_length);
    size_t real_size = m_container->fetchData(p, offset,
                                              byte_length);
    if (real_size < byte_length) {
      LOGWARN("Size mismatch for data: ignoring.\n");
    }
  }
  else if((bpc == 12) || (bpc == 8)) {
    ret = _unpackData(bpc, compression, data, x, y, offset, byte_length);
    LOGDBG1("unpack result %d\n", ret);
  }
  else {
    LOGERR("Unsupported bpc %u\n", bpc);
    return OR_ERROR_INVALID_FORMAT;
  }
  data.setCfaPatternType(cfa_pattern);
  data.setDataType(data_type);
  data.setBpc(bpc);
  data.setCompression(data_type == OR_DATA_TYPE_COMPRESSED_RAW
                      ? compression : 1);
  data.setPhotometricInterpretation((ExifPhotometricInterpretation)photo_int);
  if((data_type == OR_DATA_TYPE_RAW) && (data.whiteLevel() == 0)) {
    data.setWhiteLevel((1 << bpc) - 1);
  }
  data.setDimensions(x, y);

  return ret;
}


::or_error
IfdFile::_unpackData(uint16_t bpc, uint32_t compression, RawData & data,
                     uint32_t x, uint32_t y, uint32_t offset, uint32_t byte_length)
{
  ::or_error ret = OR_ERROR_NONE;
  size_t fetched = 0;
  uint32_t current_offset = offset;
  Unpack unpack(x, compression);
  const size_t blocksize = (bpc == 8 ? x : unpack.block_size());
  LOGDBG1("Block size = %lu\n", blocksize);
  LOGDBG1("dimensions (x, y) %u, %u\n", x, y);
  std::unique_ptr<uint8_t[]> block(new uint8_t[blocksize]);
  size_t outsize = x * y * 2;
  uint8_t * outdata = (uint8_t*)data.allocData(outsize);
  size_t got;
  LOGDBG1("offset of RAW data = %u\n", current_offset);
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
