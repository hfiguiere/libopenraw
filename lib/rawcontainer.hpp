/* -*- Mode: C++; c-basic-offset:4; tab-width:4; indent-tabs-mode:nil -*- */
/*
 * libopenraw - rawcontainer.h
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

#ifndef OR_INTERNALS_RAWCONTAINER_H_
#define OR_INTERNALS_RAWCONTAINER_H_

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#include <vector>

#include "io/stream.hpp"
#include "option.hpp"

namespace OpenRaw {

namespace Internals {

/**
   Generic interface for the RAW file container
*/
class RawContainer {
public:
    /** define the endian of the container */
    typedef enum {
        ENDIAN_NULL = 0, /** no endian found: means invalid file */
        ENDIAN_BIG,      /** big endian found */
        ENDIAN_LITTLE    /** little endian found */
    } EndianType;

    /**
        @param file the stream to read from
        @param offset the offset since starting the
        beginning of the file for the container
    */
    RawContainer(const IO::Stream::Ptr &_file, off_t offset);
    /** destructor */
    virtual ~RawContainer();

    const IO::Stream::Ptr &file() { return m_file; }
    EndianType endian() const { return m_endian; }
    off_t offset() const { return m_offset; }

    bool skip(off_t offset);
    Option<int8_t> readInt8(const IO::Stream::Ptr &f);
    Option<uint8_t> readUInt8(const IO::Stream::Ptr &f);
    /** Read an int16 following the m_endian set */
    Option<int16_t> readInt16(const IO::Stream::Ptr &f);
    /** Read an int32 following the m_endian set */
    Option<int32_t> readInt32(const IO::Stream::Ptr &f);
    /** Read an uint16 following the m_endian set */
    Option<uint16_t> readUInt16(const IO::Stream::Ptr &f);
    /** Read an array of uint16 following the m_endian set.
     * @param v the vector to fill. Will be resized if too small.
     * @param count the number of elements to read
     * @return the number of element read. `count` if success.
     */
    size_t readUInt16Array(const IO::Stream::Ptr &f, std::vector<uint16_t> &v, size_t count);
    /** Read an uint32 following the m_endian set */
    Option<uint32_t> readUInt32(const IO::Stream::Ptr &f);
    /**
     * Fetch the data chunk from the file
     * @param buf the buffer to load into
     * @param offset the offset
     * @param buf_size the size of the data to fetch
     * @return the size retrieved, <= buf_size likely equal
     */
    size_t fetchData(void *buf, off_t offset, size_t buf_size);

    /**
     * Return the effective size of the container.
     */
    off_t size() const;
protected:
    RawContainer(const RawContainer &) = delete;
    RawContainer &operator=(const RawContainer &) = delete;

    void setEndian(EndianType _endian) { m_endian = _endian; }

    /** the file handle */
    IO::Stream::Ptr m_file;
    /** the offset from the beginning of the file */
    off_t m_offset;
    EndianType m_endian;
};
}
}

#endif
