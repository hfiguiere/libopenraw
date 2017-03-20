/*
 * libopenraw - ciffcontainer.cpp
 *
 * Copyright (C) 2006-2017 Hubert Figuière
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
    file->seek(offset, SEEK_SET);

    auto result_u32 = container->readUInt32(file);
    if (result_u32.empty()) {
        return false;
    }
    imageWidth = result_u32.unwrap();
    result_u32 = container->readUInt32(file);
    if (result_u32.empty()) {
        return false;
    }
    imageHeight = result_u32.unwrap();
    result_u32 = container->readUInt32(file);
    if (result_u32.empty()) {
        return false;
    }
    pixelAspectRatio = result_u32.unwrap();
    auto result_32 = container->readInt32(file);
    if (result_32.empty()) {
        return false;
    }
    rotationAngle = result_32.unwrap();
    result_u32 = container->readUInt32(file);
    if (result_u32.empty()) {
        return false;
    }
    componentBitDepth = result_u32.unwrap();
    result_u32 = container->readUInt32(file);
    if (result_u32.empty()) {
        return false;
    }
    colorBitDepth = result_u32.unwrap();
    result_u32 = container->readUInt32(file);
    if (result_u32.empty()) {
        return false;
    }
    colorBW = result_u32.unwrap();
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

RecordEntry::RecordEntry()
    : typeCode(0), length(0), offset(0)
{
}

bool RecordEntry::readFrom(CIFFContainer *container)
{
    auto file = container->file();
    auto result_16 = container->readUInt16(file);
    if (result_16.empty()) {
        return false;
    }
    typeCode = result_16.unwrap();
    auto result_32 = container->readUInt32(file);
    if (result_32.empty()) {
        return false;
    }
    length = result_32.unwrap();
    result_32 = container->readUInt32(file);
    if (result_32.empty()) {
        return false;
    }
    offset = result_32.unwrap();
    return true;
}

size_t RecordEntry::fetchData(Heap* heap, void* buf, size_t size) const
{
    return heap->container()->fetchData(buf,
                                        offset + heap->offset(), size);
}


Heap::Heap(off_t start, off_t length, CIFFContainer * _container)
    : m_start(start),
      m_length(length),
      m_container(_container),
      m_records()
{
    LOGDBG2("Heap @ %ld length = %ld\n", start, m_length);
}

std::vector<RecordEntry> & Heap::records()
{
    if (m_records.size() == 0) {
        _loadRecords();
    }
    return m_records;
}


bool Heap::_loadRecords()
{
    auto file = m_container->file();
    file->seek(m_start + m_length - 4, SEEK_SET);

    auto result = m_container->readInt32(file);

    if (result.ok()) {
        int32_t record_offset = result.unwrap();

        m_records.clear();
        file->seek(m_start + record_offset, SEEK_SET);
        auto result16 = m_container->readInt16(file);
        if (result16.empty()) {
            LOGDBG1("read numRecords failed\n");
            return false;
        }
        int16_t numRecords = result16.unwrap();
        LOGDBG2("numRecords %d\n", numRecords);

        m_records.reserve(numRecords);
        for (int16_t i = 0; i < numRecords; i++) {
            m_records.push_back(RecordEntry());
            m_records.back().readFrom(m_container);
        }
        return true;
    }
    return false;
}


#if 0
class OffsetTable {
    uint16_t numRecords;/* the number tblArray elements */
    RecordEntry tblArray[1];/* Array of the record entries */
};
#endif


bool HeapFileHeader::readFrom(CIFFContainer *container)
{
    endian = RawContainer::ENDIAN_NULL;
    bool ret = false;
    auto file = container->file();
    int s = file->read(byteOrder, 2);
    if (s == 2) {
        if((byteOrder[0] == 'I') && (byteOrder[1] == 'I')) {
            endian = RawContainer::ENDIAN_LITTLE;
        }
        else if((byteOrder[0] == 'M') && (byteOrder[1] == 'M')) {
            endian = RawContainer::ENDIAN_BIG;
        }
        container->setEndian(endian);
        auto result32 = container->readUInt32(file);
        if (result32.ok()) {
            headerLength = result32.unwrap();
            ret = true;
        }
        if (ret) {
            ret = (file->read(type, 4) == 4);
        }
        if (ret) {
            ret = (file->read(subType, 4) == 4);
        }
        if (ret) {
            result32 = container->readUInt32(file);
            if (result32.ok()) {
                version = result32.unwrap();
                ret = true;
            }
        }
    }
    return ret;
}

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

CIFF::Heap::Ref CIFFContainer::heap()
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

        LOGDBG1("heap len %ld\n", heapLength);
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

CIFF::Heap::Ref CIFFContainer::getImageProps()
{
    if(!m_imageprops) {
        if(!heap()) {
            return CIFF::Heap::Ref();
        }

        auto & records = m_heap->records();

        // locate the properties
        auto iter = std::find_if(records.cbegin(), records.cend(),
                                 [](const CIFF::RecordEntry& e) {
                                     return e.isA(static_cast<uint16_t>(CIFF::TAG_IMAGEPROPS));
                                 });
        if (iter == records.end()) {
            LOGERR("Couldn't find the image properties.\n");
            return CIFF::Heap::Ref();
        }

        m_imageprops = std::make_shared<CIFF::Heap>(
            iter->offset + m_heap->offset(), iter->length, this);
    }
    return m_imageprops;
}

const CIFF::ImageSpec * CIFFContainer::getImageSpec()
{
    if(!m_hasImageSpec) {
        CIFF::Heap::Ref props = getImageProps();

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
        m_imagespec.readFrom(iter->offset + props->offset(), this);
        m_hasImageSpec = true;
    }
    return &m_imagespec;
}

const CIFF::Heap::Ref CIFFContainer::getCameraProps()
{
    if(!m_cameraprops) {
        CIFF::Heap::Ref props = getImageProps();

        if(!props) {
            return CIFF::Heap::Ref();
        }
        auto & propsRecs = props->records();
        auto iter = std::find_if(propsRecs.cbegin(), propsRecs.cend(),
                                 [] (const CIFF::RecordEntry & e) {
                                     return e.isA(static_cast<uint16_t>(CIFF::TAG_CAMERAOBJECT));
                                 });
        if (iter == propsRecs.end()) {
            LOGERR("Couldn't find the camera props.\n");
            return CIFF::Heap::Ref();
        }
        m_cameraprops = std::make_shared<CIFF::Heap>(
            iter->offset + props->offset(), iter->length, this);
    }
    return m_cameraprops;
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
