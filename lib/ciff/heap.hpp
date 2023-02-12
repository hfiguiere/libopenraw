/* -*- Mode: C++; c-basic-offset:4; tab-width:4; indent-tabs-mode:nil; -*- */
/*
 * libopenraw - ciff/heap.hpp
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

#include <memory>

#include "rawcontainer.hpp"
#include "ciff/recordentry.hpp"

namespace OpenRaw {
namespace Internals {

class CIFFContainer;

namespace CIFF {
/** @addtogroup canon
 * @{ */

class Heap;

/** @brief Shared ptr to Heap */
typedef std::shared_ptr<Heap> HeapRef;

/** @brief A CIFF Heap */
class Heap
{
public:

    /** @brief Construct a heap from a location in the container
     * @param start the begin address relative to the container.
     * @param length the length in bytes
     * @param container the container to read from
     */
    Heap(off_t start, off_t length, const CIFFContainer* container);

    Heap(const Heap &) = delete;
    /** @brief Move constructor */
    Heap(Heap&&) = default;
    Heap & operator=(const Heap &) = delete;

    /** @brief Return the records from the heap. Load them as needed. */
    RecordEntries& records();
    /** @brief Return the containing container */
    const CIFFContainer* container() const
        {
            return m_container;
        }
    /** @brief Return the offset from the begining of the container. */
    off_t offset() const
        {
            return m_start;
        }
private:
    /** @brief Read the Heap from file and parse the records */
    bool _loadRecords();

    off_t m_start;
    off_t m_length;
    const CIFFContainer* m_container;
    RecordEntries m_records;
};

/** @brief Header of heap CIFF file
 */
class HeapFileHeader
{
public:
    /** @brief Read the header from the container
     * @return false in case of error.
     */
    bool readFrom(CIFFContainer *);
    char       byteOrder[2]; /**< byte order 'MM' for Motorola,'II' for Intel */
    uint32_t   headerLength; /**< length of header (in bytes) */
    char       type[4]; /**< File type. Value is 'HEAP' */
    char       subType[4]; /**< Sub type. Value is 'CCDR' */
    uint32_t   version; /**< version higher word: 0x0001, Lower word: 0x0002. */
    /* uint32_t   reserved1; */
    /* uint32_t   reserved2; */
    RawContainer::EndianType endian; /**< The endian, based on %byteOrder */
};

/** @} */
}
}
}
