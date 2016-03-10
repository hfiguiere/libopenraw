/*
 * libopenraw - decompressor.h
 *
 * Copyright (C) 2007-2015 Hubert Figuiere
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

#ifndef OR_INTERNALS_DECOMPRESS_H_
#define OR_INTERNALS_DECOMPRESS_H_

#include <stddef.h>

namespace OpenRaw {

class RawData;

namespace IO {
class Stream;
}

namespace Internals {

class RawContainer;

class Decompressor {
public:
    Decompressor(IO::Stream *stream, RawContainer *container);
    virtual ~Decompressor();

    // non copyable
    Decompressor(const Decompressor &) = delete;
    Decompressor &operator=(const Decompressor &) = delete;

    /** decompress the bitmapdata and return a new bitmap
     * @param in a preallocated BitmapData instance
     * or NULL if decompress has to allocate it.
     * @return the new bitmap decompressed. NULL is failure.
     * Caller owns it.
     * @todo use a shared_ptr here, or something
     */
    virtual RawData *decompress(RawData *in = NULL) = 0;

protected:
    IO::Stream *m_stream;
    RawContainer *m_container;
};
}
}

#endif
