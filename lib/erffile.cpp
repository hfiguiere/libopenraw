/*
 * libopenraw - erffile.cpp
 *
 * Copyright (C) 2006-2020 Hubert Figuière
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


#include <libopenraw/cameraids.h>

#include "ifddir.hpp"
#include "rawfile_private.hpp"
#include "erffile.hpp"
#include "rawdata.hpp"
#include "thumbnail.hpp"

using namespace Debug;

namespace OpenRaw {

class RawData;

namespace Internals {

/* taken from dcraw, by default */
static const BuiltinColourMatrix s_matrices[] = {
    { OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_EPSON, OR_TYPEID_EPSON_RD1), 0, 0,
      { 6827,-1878,-732,-8429,16012,2564,-704,592,7145 } },
    { OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_EPSON, OR_TYPEID_EPSON_RD1S), 0, 0,
      { 6827,-1878,-732,-8429,16012,2564,-704,592,7145 } },
    { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } }
};

const IfdFile::camera_ids_t ERFFile::s_def[] = {
    { "R-D1", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_EPSON,
                                  OR_TYPEID_EPSON_RD1) },
    { "R-D1s", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_EPSON,
                                   OR_TYPEID_EPSON_RD1S) },			{ 0, 0 }
};

RawFile *ERFFile::factory(const IO::Stream::Ptr &s)
{
    return new ERFFile(s);
}

ERFFile::ERFFile(const IO::Stream::Ptr &s)
    : TiffEpFile(s, OR_RAWFILE_TYPE_ERF)
{
    _setIdMap(s_def);
    _setMatrices(s_matrices);
}

ERFFile::~ERFFile()
{
}

::or_error ERFFile::getMakerNoteThumbnail(Thumbnail& thumbnail)
{
    auto mnote = std::dynamic_pointer_cast<MakerNoteDir>(makerNoteIfd());
    if (!mnote) {
        LOGERR("Couldn't find the MakerNote.");
        return OR_ERROR_NOT_FOUND;
    }
    auto thumb = mnote->getEntry(ERF_TAG_PREVIEW_IMAGE);
    if (!thumb) {
        LOGERR("Couldn't find the preview image.");
        return OR_ERROR_NOT_FOUND;
    }

    auto count = thumb->count();
    void *p = thumbnail.allocData(count);
    auto size = mnote->getEntryData(*thumb, (uint8_t*)p, count);
    if (size != count) {
        LOGERR("Couldn't load the preview image. Read only %lu bytes, expected %d", (LSIZE)size, count);
        return OR_ERROR_NOT_FOUND;
    }

    // The data start by 0xee instead of 0xff for a JPEG. Not sure why.
    ((uint8_t*)p)[0] = 0xff;

    // It is 640x424 (3:2 aspect ratio)
    thumbnail.setDataType(OR_DATA_TYPE_JPEG);
    thumbnail.setDimensions(640, 424);

    return OR_ERROR_NONE;
}

::or_error ERFFile::_enumThumbnailSizes(std::vector<uint32_t>& list)
{
    auto err = this->TiffEpFile::_enumThumbnailSizes(list);
    if (err == OR_ERROR_NONE) {
        // EPSON R-D1 and R-D1s have a 640 pixel JPEG in the MakerNote
        // We don't need to bother detecting, there won't be a new file format.
        list.push_back(640);
    }
    return err;
}

::or_error ERFFile::_getThumbnail(uint32_t size, Thumbnail& thumbnail)
{
    if (size == 640) {
        getMakerNoteThumbnail(thumbnail);
        return OR_ERROR_NONE;
    } else {
        return TiffEpFile::_getThumbnail(size, thumbnail);
    }
}

::or_error ERFFile::_getRawData(RawData & data, uint32_t /*options*/)
{
    ::or_error err;
    const IfdDir::Ref & _cfaIfd = cfaIfd();
    if(_cfaIfd) {
        err = _getRawDataFromDir(data, _cfaIfd);
        auto mnote = makerNoteIfd();
        auto sensor_area = mnote->getEntry(MNOTE_EPSON_SENSORAREA);
        if (sensor_area) {
            auto x = mnote->getEntryValue<uint16_t>(*sensor_area, 0, true);
            auto y = mnote->getEntryValue<uint16_t>(*sensor_area, 1, true);
            auto w = mnote->getEntryValue<uint16_t>(*sensor_area, 2, true);
            auto h = mnote->getEntryValue<uint16_t>(*sensor_area, 3, true);
            data.setActiveArea(x, y, w, h);
        }
    }
    else {
        err = OR_ERROR_NOT_FOUND;
    }
    return err;
}

}
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
