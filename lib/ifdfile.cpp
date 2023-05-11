/*
 * libopenraw - ifdfile.cpp
 *
 * Copyright (C) 2006-2020 Hubert Figuière
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
#include "unpack.hpp"
#include "xtranspattern.hpp"

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

IfdDir::Ref IfdFile::_locateCfaIfd()
{
  // CFA IFD is the main IFD by default
  return mainIfd();
}

IfdDir::Ref IfdFile::_locateMainIfd()
{
  auto ifd = m_container->setDirectory(0);
  if (ifd) {
    ifd->setType(OR_IFD_MAIN);
  }
  return ifd;
}

namespace {


}

void IfdFile::_identifyId()
{
    // Identify from the vendor internal ID.
    IfdDir::Ref ifd;
    uint16_t index = 0;
    const ModelIdMap* model_map = nullptr;
    if (vendorCameraIdLocation(ifd, index, model_map) && ifd) {
        auto id = ifd->getIntegerValue(index);
        if (id) {
            auto id_value = id.value();
            auto type_id = modelid_to_typeid(*model_map, id_value);
            if (type_id != 0) {
                _setTypeId(type_id);
                return;
            }
            LOGERR("unknown model ID 0x%x (%u)\n", id_value, id_value);
        }
    }

    // Fallback on using strings.
    const IfdDir::Ref & _mainIfd = mainIfd();
    if (!_mainIfd) {
        LOGERR("Main IFD not found to identify the file.\n");
        return;
    }

    auto make = _mainIfd->getValue<std::string>(IFD::EXIF_TAG_MAKE);
    auto model = _mainIfd->getValue<std::string>(IFD::EXIF_TAG_MODEL);
    if (!model) {
        // BlackMagic CinemaDNG doesn't have Make and Model.
        model = _mainIfd->getValue<std::string>(IFD::DNG_TAG_UNIQUE_CAMERA_MODEL);
        if (!make) {
            make = model;
        }
    }
    if (make && model) {
        _setTypeId(_typeIdFromModel(make.value(), model.value()));
    }
}

::or_error IfdFile::_addThumbnailFromStream(uint32_t offset, uint32_t len,
                                            std::vector<uint32_t>& list)
{
  auto err = OR_ERROR_NOT_FOUND;
  LOGDBG1("fetching JPEG\n");
  IO::Stream::Ptr s = std::make_shared<IO::StreamClone>(m_io, offset);
  auto jfif = std::make_unique<JfifContainer>(s, 0);

  uint32_t x, y;
  x = y = 0;
  jfif->getDimensions(x, y);
  LOGDBG1("JPEG dimensions x=%d y=%d\n", x, y);

  uint32_t dim = std::max(x, y);
  // "Olympus" MakerNote carries a 160 px thubnail we might already have.
  // We don't check it is the same.
  if (dim && std::find(list.begin(), list.end(), dim) == list.end()) {
    _addThumbnail(dim, ThumbDesc(x, y, OR_DATA_TYPE_JPEG, offset, len));
    list.push_back(dim);
    err = OR_ERROR_NONE;
  }

  return err;
}

::or_error IfdFile::_addThumbnailFromEntry(const IfdEntry::Ref& e, off_t offset,
                                           std::vector<uint32_t>& list)
{
  ::or_error err = OR_ERROR_NOT_FOUND;
  if (e) {
    auto val_offset = e->offset();

    val_offset += offset;

    err =_addThumbnailFromStream(val_offset, e->count(), list);
  }
  return err;
}

::or_error
IfdFile::_enumThumbnailSizes(std::vector<uint32_t> &list)
{
  ::or_error err = OR_ERROR_NONE;

  LOGDBG1("_enumThumbnailSizes()\n");
  std::vector<IfdDir::Ref> & dirs = m_container->directories();

  LOGDBG1("num of dirs %lu\n", (LSIZE)dirs.size());
  for(auto dir : dirs)
  {
    dir->load();
    or_error ret = _locateThumbnail(dir, list);
    if (ret == OR_ERROR_NONE) {
      LOGDBG1("Found %u pixels\n", list.back());
    }
    auto result = dir->getSubIFDs();
    if (result) {
      std::vector<IfdDir::Ref> subdirs = result.value();
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
    if (cfaIfd() == dir) {
      return OR_ERROR_NOT_FOUND;
    }
    else {
      subtype = 1;
    }
  } else {
    subtype = result.value();
  }
  LOGDBG1("subtype %u\n", subtype);
  if (subtype == 1) {

    uint16_t photom_int =
      dir->getValue<uint16_t>(IFD::EXIF_TAG_PHOTOMETRIC_INTERPRETATION).value_or(IFD::EV_PI_RGB);

    uint32_t x = dir->getIntegerValue(IFD::EXIF_TAG_IMAGE_WIDTH).value_or(0);
    uint32_t y = dir->getIntegerValue(IFD::EXIF_TAG_IMAGE_LENGTH).value_or(0);

    uint16_t compression = dir->getValue<uint16_t>(IFD::EXIF_TAG_COMPRESSION).value_or(0);

    uint32_t offset = 0;
    uint32_t byte_count = dir->getValue<uint32_t>(IFD::EXIF_TAG_STRIP_BYTE_COUNTS).value_or(0);

    result = dir->getValue<uint32_t>(IFD::EXIF_TAG_STRIP_OFFSETS);
    bool got_it = result.has_value();
    if (got_it) {
      offset = result.value();
    }
    if (!got_it || (compression == 6) || (compression == 7)) {
      if (!got_it) {
        byte_count =
          dir->getValue<uint32_t>(IFD::EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LENGTH).value_or(0);
        result = dir->getValue<uint32_t>(IFD::EXIF_TAG_JPEG_INTERCHANGE_FORMAT);
        got_it = result.has_value();
        if (got_it) {
          offset = result.value();
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
            auto s =std::make_shared<IO::StreamClone>(m_io, offset);
            auto jfif = std::make_unique<JfifContainer>(s, 0);
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
        auto result2 = dir->getEntryArrayValue<uint16_t>(*entry);
        if (result2) {
          std::vector<uint16_t> arr = result2.value();
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
  if (result) {
    byte_length = result.value();
    return dir->getValue<uint32_t>(IFD::EXIF_TAG_JPEG_INTERCHANGE_FORMAT).value_or(0);
  }

  // some case it is STRIP_OFFSETS for JPEG
  byte_length = dir->getValue<uint32_t>(IFD::EXIF_TAG_STRIP_BYTE_COUNTS).value_or(0);
  return dir->getValue<uint32_t>(IFD::EXIF_TAG_STRIP_OFFSETS).value_or(0);
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
    if (e) {
      val = ifd->makeMetaValue(*e);
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

namespace {

const MosaicInfo *
_convertArrayToCfaPattern(const std::vector<uint8_t> &cfaPattern)
{
  ::or_cfa_pattern cfa_pattern = OR_CFA_PATTERN_NON_RGB22;
  if(cfaPattern.size() != 4) {
    if (cfaPattern.size() == 36) {
      // XXX don't assume this is X-Trans
      return XTransPattern::xtransPattern();
    }
    LOGWARN("Unsupported bayer pattern of size %lu\n", (LSIZE)cfaPattern.size());
  } else {
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
      case IFD::CFA_BLUE:
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
    return MosaicInfo::twoByTwoPattern(cfa_pattern);
  }
  return nullptr;
}

const MosaicInfo *_convertNewCfaPattern(const IfdDir::Ref& dir, IfdEntry& e)
{
  if (e.count() < 4) {
    return nullptr;
  }

  uint16_t hdim = dir->getEntryValue<uint16_t>(e, 0, true);
  uint16_t vdim = dir->getEntryValue<uint16_t>(e, 1, true);
  if(hdim != 2 && vdim != 2) {
    // cfa_pattern = OR_CFA_PATTERN_NON_RGB22;
    if (hdim != 6 && vdim != 6) {
      LOGWARN("CFA pattern dimension %dx%d are incompatible", hdim, vdim);
      return nullptr;
    }
    return XTransPattern::xtransPattern();
  } else {
    std::vector<uint8_t> cfaPattern;
    cfaPattern.push_back(dir->getEntryValue<uint8_t>(e, 4, true));
    cfaPattern.push_back(dir->getEntryValue<uint8_t>(e, 5, true));
    cfaPattern.push_back(dir->getEntryValue<uint8_t>(e, 6, true));
    cfaPattern.push_back(dir->getEntryValue<uint8_t>(e, 7, true));
    return _convertArrayToCfaPattern(cfaPattern);
  }
}


/** Extract the MosaicInfo from the entry */
const MosaicInfo *_convertCfaPattern(const IfdDir::Ref& dir, IfdEntry& e)
{
  LOGDBG1("%s\n", __FUNCTION__);
  auto result = dir->getEntryArrayValue<uint8_t>(e);
  if (result) {
    return _convertArrayToCfaPattern(result.value());
  }

  return nullptr;
}

/** get the mosaic info out of the directory
 * @param dir the directory
 * @return the %MosaicInfo* value. %nullptr mean that
 * nothing has been found.
 */
const MosaicInfo *_getMosaicInfo(const IfdDir::Ref & dir)
{
  LOGDBG1("%s\n", __FUNCTION__);
  const MosaicInfo *mosaic_info = nullptr;
  try {
    IfdEntry::Ref e = dir->getEntry(IFD::EXIF_TAG_CFA_PATTERN);
    if (e) {
      mosaic_info = _convertCfaPattern(dir, *e);
    } else {
      e = dir->getEntry(IFD::EXIF_TAG_NEW_CFA_PATTERN);
      if (e)  {
        mosaic_info = _convertNewCfaPattern(dir, *e);
      }
    }
  }
  catch(...)
  {
    LOGERR("Exception in _getCfaPattern().\n");
  }
  return mosaic_info;
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
  uint16_t bpc = result.value_or(0);

  auto result2 = dir->getValue<uint32_t>(IFD::EXIF_TAG_STRIP_OFFSETS);
  if(result2) {
    offset = result2.value();
    IfdEntry::Ref e = dir->getEntry(IFD::EXIF_TAG_STRIP_BYTE_COUNTS);
    if(!e) {
      LOGDBG1("byte len not found\n");
      return OR_ERROR_NOT_FOUND;
    }
    auto result3 = dir->getEntryArrayValue<uint32_t>(*e);
    if (result3) {
      std::vector<uint32_t> counts = result3.value();
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
    auto result3 = dir->getEntryArrayValue<uint32_t>(*e);
    if (!result3) {
      LOGDBG1("tile offsets not found\n");
      return OR_ERROR_NOT_FOUND;
    }
    std::vector<uint32_t> offsets = result3.value();
    offset = offsets[0];
    e = dir->getEntry(IFD::TIFF_TAG_TILE_BYTECOUNTS);
    if(!e) {
      LOGDBG1("tile byte counts not found\n");
      return OR_ERROR_NOT_FOUND;
    }
    result3 = dir->getEntryArrayValue<uint32_t>(*e);
    if (result3) {
      std::vector<uint32_t> counts = result3.value();
      LOGDBG1("counting tiles\n");
      byte_length = std::accumulate(counts.cbegin(), counts.cend(), 0);
    }
  }

  result2 = dir->getIntegerValue(IFD::EXIF_TAG_IMAGE_WIDTH);
  if(result2.empty()) {
    LOGDBG1("X not found\n");
    return OR_ERROR_NOT_FOUND;
  }
  uint32_t x = result2.value();

  result2 = dir->getIntegerValue(IFD::EXIF_TAG_IMAGE_LENGTH);
  if(!result2) {
    LOGDBG1("Y not found\n");
    return OR_ERROR_NOT_FOUND;
  }
  uint32_t y = result2.value();

  uint32_t photo_int
    = dir->getIntegerValue(IFD::EXIF_TAG_PHOTOMETRIC_INTERPRETATION)
    .value_or(IFD::EV_PI_CFA);

  BitmapData::DataType data_type = OR_DATA_TYPE_NONE;

  result = dir->getValue<uint16_t>(IFD::EXIF_TAG_COMPRESSION);
  if(!result) {
    LOGDBG1("Compression type not found\n");
  }
	uint32_t compression = _translateCompressionType(
    static_cast<IFD::TiffCompress>(result.value_or(0)));

  switch(compression)
  {
  case IFD::COMPRESS_NONE:
    data_type = OR_DATA_TYPE_RAW;
    break;
  case IFD::COMPRESS_NIKON_PACK:
  case IFD::COMPRESS_PENTAX_PACK:
    data_type = OR_DATA_TYPE_RAW;
    break;
  default:
    data_type = OR_DATA_TYPE_COMPRESSED_RAW;
    break;
  }

  LOGDBG1("RAW Compression is %u\n", compression);
  LOGDBG1("bpc is %u\n", bpc);

  const MosaicInfo *mosaic_info = _getMosaicInfo(dir);
  if (!mosaic_info) {
    // some file have it in the exif IFD instead.
    mosaic_info = _getMosaicInfo(exifIfd());
  }


  // This is the actual bit per component since we might readjust
  // for padding.
  auto actual_bpc = bpc;
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
  data.setMosaicInfo(mosaic_info);
  data.setDataType(data_type);
  data.setBpc(bpc);
  data.setCompression(data_type == OR_DATA_TYPE_COMPRESSED_RAW
                      ? compression : 1);
  data.setPhotometricInterpretation((ExifPhotometricInterpretation)photo_int);
  if(data.whiteLevel() == 0) {
    data.setWhiteLevel((1 << actual_bpc) - 1);
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
  LOGDBG1("Block size = %lu\n", (LSIZE)blocksize);
  LOGDBG1("dimensions (x, y) %u, %u\n", x, y);
  auto block = std::make_unique<uint8_t[]>(blocksize);
  size_t outsize = x * y * 2;
  uint16_t* outdata = (uint16_t*)data.allocData(outsize);
  size_t got;
  LOGDBG1("offset of RAW data = %u\n", current_offset);
  do {
    got = m_container->fetchData (block.get(),
                                  current_offset, blocksize);
    fetched += got;
    offset += got;
    current_offset += got;
    if (got) {
      if (bpc == 12) {
        size_t out;
        ret = unpack.unpack_be12to16(outdata, outsize,
                                     block.get(),
                                     got, out);
        outdata += out / 2;
        outsize -= out;
        if (ret != OR_ERROR_NONE) {
          break;
        }
      } else {
        std::copy(block.get(), block.get() + got, outdata);
        outdata += got;
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
  tab-width:4
  c-basic-offset:4
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
