/* -*- Mode: C++; c-basic-offset:4; tab-width:4; indent-tabs-mode:nil; -*- */
/*
 * libopenraw - ciff/heap.cpp
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

#include "ciff/heap.hpp"
#include "ciffcontainer.hpp"

namespace OpenRaw {
namespace Internals {
namespace CIFF {

Heap::Heap(off_t start, off_t length, const CIFFContainer* _container)
    : m_start(start),
      m_length(length),
      m_container(_container),
      m_records()
{
    LOGDBG2("Heap @ %lld length = %lld\n", (long long int)start, (long long int)m_length);
}

RecordEntries& Heap::records()
{
    if (m_records.size() == 0) {
        _loadRecords();
    }
    return m_records;
}


bool Heap::_loadRecords()
{
    auto file = m_container->file();
    auto endian = m_container->endian();
    file->seek(m_start + m_length - 4, SEEK_SET);

    auto result = m_container->readInt32(file, endian);

    if (result) {
        int32_t record_offset = result.value();

        m_records.clear();
        file->seek(m_start + record_offset, SEEK_SET);
        auto result16 = m_container->readInt16(file, endian);
        if (result16.empty()) {
            LOGDBG1("read numRecords failed\n");
            return false;
        }
        int16_t numRecords = result16.value();
        LOGDBG2("numRecords %d\n", numRecords);

        for (int16_t i = 0; i < numRecords; i++) {
            RecordEntry entry;
            entry.readFrom(m_container);
            m_records.insert(std::make_pair(TAGCODE(entry.typeCode), entry));
        }
        return true;
    }
    return false;
}

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
        auto result32 = container->readUInt32(file, endian);
        if (result32) {
            headerLength = result32.value();
            ret = true;
        }
        if (ret) {
            ret = (file->read(type, 4) == 4);
        }
        if (ret) {
            ret = (file->read(subType, 4) == 4);
        }
        if (ret) {
            result32 = container->readUInt32(file, endian);
            if (result32) {
                version = result32.value();
                ret = true;
            }
        }
    }
    return ret;
}

}
}
}
