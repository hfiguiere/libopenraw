/*
 * libopenraw - rawfile.cpp
 *
 * Copyright (C) 2007-2016 Hubert Figuiere
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
/* @brief C api for rawfile
 */

#include <stddef.h>
#include <stdint.h>

#include <boost/checked_delete.hpp>

#include <libopenraw/consts.h>
#include <libopenraw/thumbnails.h>
#include <libopenraw/types.h>

#include "rawfile.hpp"

namespace OpenRaw {
class BitmapData;
class RawData;
class Thumbnail;
}

using OpenRaw::RawFile;
using OpenRaw::RawData;
using OpenRaw::BitmapData;
using OpenRaw::Thumbnail;

extern "C" {

/** check pointer validity */
#define CHECK_PTR(p, r)                                                        \
    if (p == nullptr) {                                                           \
        return r;                                                              \
    }

const char **or_get_file_extensions()
{
    return RawFile::fileExtensions();
}

ORRawFileRef or_rawfile_new(const char *filename, or_rawfile_type type)
{
    CHECK_PTR(filename, NULL);
    RawFile *rawfile = RawFile::newRawFile(filename, type);
    return reinterpret_cast<ORRawFileRef>(rawfile);
}

ORRawFileRef or_rawfile_new_from_memory(const uint8_t *buffer, uint32_t len,
                                        or_rawfile_type type)
{
    CHECK_PTR(buffer, NULL);
    RawFile *rawfile = RawFile::newRawFileFromMemory(buffer, len, type);
    return reinterpret_cast<ORRawFileRef>(rawfile);
}

or_error or_rawfile_release(ORRawFileRef rawfile)
{
    CHECK_PTR(rawfile, OR_ERROR_NOTAREF);
    boost::checked_delete(reinterpret_cast<RawFile *>(rawfile));
    return OR_ERROR_NONE;
}

or_rawfile_type or_rawfile_get_type(ORRawFileRef rawfile)
{
    CHECK_PTR(rawfile, OR_RAWFILE_TYPE_UNKNOWN);
    RawFile *prawfile = reinterpret_cast<RawFile *>(rawfile);
    return prawfile->type();
}

or_rawfile_typeid or_rawfile_get_typeid(ORRawFileRef rawfile)
{
    CHECK_PTR(rawfile, OR_RAWFILE_TYPE_UNKNOWN);
    RawFile *prawfile = reinterpret_cast<RawFile *>(rawfile);
    return prawfile->typeId();
}

const uint32_t *
or_rawfile_get_thumbnail_sizes(ORRawFileRef  rawfile,
                               size_t       *size)
{
    CHECK_PTR(rawfile, nullptr);
    CHECK_PTR(size, nullptr);
    RawFile *prawfile = reinterpret_cast<RawFile *>(rawfile);
    const auto & v = prawfile->listThumbnailSizes();
    if (v.empty()) {
      *size = 0;
      return nullptr;
    }
    *size = v.size();
    // we return a pointer to the storage
    // C++11 spec says it has to be contiguous.
    return &(*v.begin());
}

or_error or_rawfile_get_thumbnail(ORRawFileRef rawfile,
                                  uint32_t _preferred_size,
                                  ORThumbnailRef thumb)
{
    CHECK_PTR(rawfile, OR_ERROR_NOTAREF);
    RawFile *prawfile = reinterpret_cast<RawFile *>(rawfile);
    return prawfile->getThumbnail(_preferred_size,
                                  *reinterpret_cast<Thumbnail *>(thumb));
}

or_error or_rawfile_get_rawdata(ORRawFileRef rawfile, ORRawDataRef rawdata,
                                uint32_t options)
{
    RawFile *prawfile = reinterpret_cast<RawFile *>(rawfile);
    CHECK_PTR(rawfile, OR_ERROR_NOTAREF);
    return prawfile->getRawData(*reinterpret_cast<RawData *>(rawdata), options);
}

or_error or_rawfile_get_rendered_image(ORRawFileRef rawfile,
                                       ORBitmapDataRef bitmapdata,
                                       uint32_t options)
{
    RawFile *prawfile = reinterpret_cast<RawFile *>(rawfile);
    CHECK_PTR(rawfile, OR_ERROR_NOTAREF);
    return prawfile->getRenderedImage(
        *reinterpret_cast<BitmapData *>(bitmapdata), options);
}

int32_t or_rawfile_get_orientation(ORRawFileRef rawfile)
{
    RawFile *prawfile = reinterpret_cast<RawFile *>(rawfile);
    CHECK_PTR(rawfile, 0);
    return prawfile->getOrientation();
}

or_error or_rawfile_get_colourmatrix1(ORRawFileRef rawfile, double *matrix,
                                      uint32_t *size)
{
    RawFile *prawfile = reinterpret_cast<RawFile *>(rawfile);
    CHECK_PTR(rawfile, OR_ERROR_NOTAREF);
    CHECK_PTR(size, OR_ERROR_INVALID_PARAM);
    return prawfile->getColourMatrix1(matrix, *size);
}

or_error or_rawfile_get_colourmatrix2(ORRawFileRef rawfile, double *matrix,
                                      uint32_t *size)
{
    RawFile *prawfile = reinterpret_cast<RawFile *>(rawfile);
    CHECK_PTR(rawfile, OR_ERROR_NOTAREF);
    CHECK_PTR(size, OR_ERROR_INVALID_PARAM);
    return prawfile->getColourMatrix2(matrix, *size);
}

ExifLightsourceValue or_rawfile_get_calibration_illuminant1(ORRawFileRef rawfile)
{
    RawFile *prawfile = reinterpret_cast<RawFile *>(rawfile);
    CHECK_PTR(rawfile, (ExifLightsourceValue)0);
    return prawfile->getCalibrationIlluminant1();
}

ExifLightsourceValue or_rawfile_get_calibration_illuminant2(ORRawFileRef rawfile)
{
    RawFile *prawfile = reinterpret_cast<RawFile *>(rawfile);
    CHECK_PTR(rawfile, (ExifLightsourceValue)0);
    return prawfile->getCalibrationIlluminant2();
}

ORConstMetaValueRef
or_rawfile_get_metavalue(ORRawFileRef rawfile, int32_t meta_index)
{
    RawFile *prawfile = reinterpret_cast<RawFile *>(rawfile);
    CHECK_PTR(rawfile, nullptr);
    return reinterpret_cast<ORConstMetaValueRef>(prawfile->getMetaValue(meta_index));
}

}
