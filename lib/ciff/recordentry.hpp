/* -*- Mode: C++; c-basic-offset:4; tab-width:4; indent-tabs-mode:nil; -*- */
/*
 * libopenraw - ciff/recordentry.hpp
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

#pragma once

#include <stdint.h>
#include <map>

#include <boost/array.hpp>
#include <boost/variant.hpp>

#include "trace.hpp"

namespace OpenRaw {
namespace Internals {

class CIFFContainer;

namespace CIFF {

class Heap;
class RecordEntry;

/** mask for the typeCode */
enum {
    STORAGELOC_MASK = 0xc000, /**< storage location bit mask */
    FORMAT_MASK = 0x3800,     /**< format of the data */
    TAGCODE_MASK = 0x3fff  /**< include the format, because the last
                            * part is non significant */
};

// Remove the storage location bits.
#define TAGCODE(x) ((x) & TAGCODE_MASK)

typedef enum {
    TYPE_BYTE = 0x0000,
    TYPE_ASCII = 0x0800,
    TYPE_WORD = 0x1000, // 16-bits?
    TYPE_DWORD = 0x1800, // 32-bits?
    TYPE_BYTE2 = 0x2000, // arbitrary structure
    TYPE_HEAP1 = 0x2800,
    TYPE_HEAP2 = 0x3000,
} CIFFType;

typedef std::map<uint16_t, RecordEntry> RecordEntries;

class RecordEntry
{
public:

    RecordEntry();

    /** load record from container
     * @param container the container
     * @return true if success
     */
    bool readFrom(const CIFFContainer* container);
    /** fetch data define by the record from the heap
     * @param heap the heap to load from
     * @param buf the allocated buffer to load into
     * @param size the size of the allocated buffer
     * @return the size actually fetched. MIN(size, this->length);
     */
    size_t fetchData(Heap* heap, void* buf, size_t size) const;

    bool isHeap() const
        {
            auto t = type();
            return (t == TYPE_HEAP1 || t == TYPE_HEAP2);
        }
    CIFFType type() const
        {
            return (CIFFType)(typeCode & (uint16_t)FORMAT_MASK);
        }

    /** Create a heap from the RecordEntry */
    Heap heap(Heap& h, const CIFFContainer* container) const;
    /** Number of unit count, derived from byte size */
    uint32_t count() const;
    /** Equivalent exif type */
    uint16_t exifType() const;
    /** Offset from begining of container */
    uint32_t containerOffset(const Heap& heap) const;

    std::string getString(Heap& heap) const;

    typedef boost::array<uint8_t, 8> InRec;
    struct InHeap
    {
        InHeap();
        InHeap(uint32_t _length, uint32_t _offset);
        uint32_t length;/* record length */
        uint32_t offset;/* offset of the record in the heap*/
    };

    bool inRecord() const
        {
            return (typeCode & STORAGELOC_MASK) != 0;
        }
    uint32_t length() const
        {
            if (!inRecord()) {
                return boost::get<InHeap>(data).length;
            }
            LOGERR("length failed\n");
            return 0;
        }
    uint32_t offset() const
        {
            if (!inRecord()) {
                return boost::get<InHeap>(data).offset;
            }
            LOGERR("offset failed\n");
            return 0;
        }
    uint16_t typeCode;/* type code of the record */
    boost::variant<InRec, InHeap> data;
};

}
}
}
