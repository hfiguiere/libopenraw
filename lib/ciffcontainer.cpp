/*
 * libopenraw - ciffcontainer.cpp
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

#include <fcntl.h>
#include <algorithm>
#include <cstring>

#include "ciffcontainer.hpp"
#include "trace.hpp"

using namespace Debug;

namespace OpenRaw {
namespace Internals {

namespace CIFF {


bool ImageSpec::readFrom(off_t offset, CIFFContainer *container)
{
    auto file = container->file();
    auto endian = container->endian();
    file->seek(offset, SEEK_SET);

    auto result_u32 = container->readUInt32(file, endian);
    if (result_u32.empty()) {
        return false;
    }
    imageWidth = result_u32.value();
    result_u32 = container->readUInt32(file, endian);
    if (result_u32.empty()) {
        return false;
    }
    imageHeight = result_u32.value();
    result_u32 = container->readUInt32(file, endian);
    if (result_u32.empty()) {
        return false;
    }
    pixelAspectRatio = result_u32.value();
    auto result_32 = container->readInt32(file, endian);
    if (result_32.empty()) {
        return false;
    }
    rotationAngle = result_32.value();
    result_u32 = container->readUInt32(file, endian);
    if (result_u32.empty()) {
        return false;
    }
    componentBitDepth = result_u32.value();
    result_u32 = container->readUInt32(file, endian);
    if (result_u32.empty()) {
        return false;
    }
    colorBitDepth = result_u32.value();
    result_u32 = container->readUInt32(file, endian);
    if (result_u32.empty()) {
        return false;
    }
    colorBW = result_u32.value();
    return true;
}

int32_t ImageSpec::exifOrientation() const
{
    int32_t orientation = 0;
    switch(rotationAngle) {
    case 0:
        orientation = 1;
        break;
    case 90:
        orientation = 6;
        break;
    case 180:
        orientation = 3;
        break;
    case 270:
        orientation = 8;
        break;
    }
    return orientation;
}


#if 0
class OffsetTable {
    uint16_t numRecords;/* the number tblArray elements */
    RecordEntry tblArray[1];/* Array of the record entries */
};
#endif

}

CIFFContainer::CIFFContainer(const IO::Stream::Ptr &_file)
    : RawContainer(_file, 0),
      m_hdr(),
      m_heap(nullptr),
      m_hasImageSpec(false)
{
    m_endian = _readHeader();
}

CIFFContainer::~CIFFContainer()
{
}

CIFF::HeapRef CIFFContainer::heap()
{
    if (m_heap == nullptr) {
        _loadHeap();
    }
    return m_heap;
}

bool CIFFContainer::_loadHeap()
{
    bool ret = false;
    if (m_heap) {
        return false;
    }
    if (m_endian != ENDIAN_NULL) {
        off_t heapLength = m_file->filesize() - m_hdr.headerLength;

        LOGDBG1("heap len %lld\n", (long long int)heapLength);
        m_heap = std::make_shared<CIFF::Heap>(m_hdr.headerLength,
                                              heapLength, this);

        ret = true;
    }
    else {
        LOGDBG1("Unknown endian\n");
    }

    return ret;
}


RawContainer::EndianType CIFFContainer::_readHeader()
{
    EndianType _endian = ENDIAN_NULL;
    m_hdr.readFrom(this);
    if ((::strncmp(m_hdr.type, "HEAP", 4) == 0)
        && (::strncmp(m_hdr.subType, "CCDR", 4) == 0)) {
        _endian = m_hdr.endian;
    }
    return _endian;
}

CIFF::HeapRef CIFFContainer::getImageProps()
{
    if(!m_imageprops) {
        if(!heap()) {
            return CIFF::HeapRef();
        }

        auto & records = m_heap->records();

        // locate the properties
        auto iter = std::find_if(records.cbegin(), records.cend(),
                                 [](const CIFF::RecordEntry& e) {
                                     return e.isA(static_cast<uint16_t>(CIFF::TAG_IMAGEPROPS));
                                 });
        if (iter == records.end()) {
            LOGERR("Couldn't find the image properties.\n");
            return CIFF::HeapRef();
        }

        m_imageprops = std::make_shared<CIFF::Heap>(
            iter->offset() + m_heap->offset(), iter->length(), this);
    }
    return m_imageprops;
}

const CIFF::ImageSpec * CIFFContainer::getImageSpec()
{
    if(!m_hasImageSpec) {
        CIFF::HeapRef props = getImageProps();

        if(!props) {
            return nullptr;
        }
        auto & propsRecs = props->records();
        auto iter = std::find_if(propsRecs.cbegin(), propsRecs.cend(),
                                 [] (const CIFF::RecordEntry &e) {
                                     return e.isA(static_cast<uint16_t>(CIFF::TAG_IMAGEINFO));
                                 });
        if (iter == propsRecs.end()) {
            LOGERR("Couldn't find the image info.\n");
            return nullptr;
        }
        m_imagespec.readFrom(iter->offset() + props->offset(), this);
        m_hasImageSpec = true;
    }
    return &m_imagespec;
}

const CIFF::HeapRef CIFFContainer::getCameraProps()
{
    if(!m_cameraprops) {
        CIFF::HeapRef props = getImageProps();

        if(!props) {
            return CIFF::HeapRef();
        }
        auto & propsRecs = props->records();
        auto iter = std::find_if(propsRecs.cbegin(), propsRecs.cend(),
                                 [] (const CIFF::RecordEntry & e) {
                                     return e.isA(static_cast<uint16_t>(CIFF::TAG_CAMERAOBJECT));
                                 });
        if (iter == propsRecs.end()) {
            LOGERR("Couldn't find the camera props.\n");
            return CIFF::HeapRef();
        }
        m_cameraprops = std::make_shared<CIFF::Heap>(
            iter->offset() + props->offset(), iter->length(), this);
    }
    return m_cameraprops;
}

CIFF::HeapRef CIFFContainer::getExifInfo() const
{
    CIFF::HeapRef props = m_imageprops;

    if (!props) {
        return CIFF::HeapRef();
    }
    auto& propsRecs = props->records();
    auto iter = std::find_if(propsRecs.cbegin(), propsRecs.cend(),
                             [] (const CIFF::RecordEntry & e) {
                                 return e.isA(static_cast<uint16_t>(CIFF::TAG_EXIFINFORMATION));
                             });
    if (iter == propsRecs.end()) {
        LOGERR("Couldn't find the Exif information.\n");
        return CIFF::HeapRef();
    }
    return std::make_shared<CIFF::Heap>(
        iter->offset() + props->offset(), iter->length(), this);
}

CIFF::CameraSettings CIFFContainer::getCameraSettings() const
{
    auto exifInfo = getExifInfo();
    auto& propsRecs = exifInfo->records();
    auto iter = std::find_if(propsRecs.cbegin(), propsRecs.cend(),
                             [] (const CIFF::RecordEntry & e) {
                                 return e.isA(static_cast<uint16_t>(CIFF::TAG_CAMERASETTINGS));
                             });
    if (iter == propsRecs.end()) {
        LOGERR("Couldn't find the camera settings.\n");
        return CIFF::CameraSettings();
    }
    auto count = iter->count();
    CIFF::CameraSettings settings;
    file()->seek(exifInfo->offset() + iter->offset(), SEEK_SET);
    size_t countRead = readUInt16Array(file(), settings, count);
    if (count != countRead) {
        LOGERR("Not enough data for camera settings\n");
    }

    return settings;
}

const CIFF::RecordEntry * CIFFContainer::getRawDataRecord() const
{
    if(!m_heap) {
        return nullptr;
    }
    auto & records = m_heap->records();
    // locate the RAW data
    auto iter = std::find_if(records.cbegin(), records.cend(),
                             [] (const CIFF::RecordEntry &e) {
                                 return e.isA(static_cast<uint16_t>(CIFF::TAG_RAWIMAGEDATA));
                             });

    if (iter != records.end()) {
        return &(*iter);
    }
    return nullptr;
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
