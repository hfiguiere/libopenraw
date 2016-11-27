/* -*- Mode: C++ -*- */
/*
 * libopenraw - crwdecompressor.h
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

#ifndef OR_INTERNALS_CRWDECOMPRESS_H_
#define OR_INTERNALS_CRWDECOMPRESS_H_

#include <stddef.h>
#include <stdint.h>

#include "decompressor.hpp"

namespace OpenRaw {

class RawData;

namespace IO {
class Stream;
}

namespace Internals {

class RawContainer;

class CrwDecompressor : public Decompressor {
public:
    CrwDecompressor(IO::Stream *stream, RawContainer *container);
    virtual ~CrwDecompressor();

    /** decompress the bitmapdata and return a new bitmap
     * @return the new bitmap decompressed. NULL is failure.
     */
    virtual RawDataPtr decompress() override;
    void setDecoderTable(uint32_t t) { m_table = t; }
    void setOutputDimensions(uint32_t x, uint32_t y) {
        m_height = y;
        m_width = x;
    }

private:
    struct decode_t {
        decode_t *branch[2];
        int leaf;
    };

    uint32_t getbits(IO::Stream *s, int nbits);
    void make_decoder(decode_t *dest, const uint8_t *source, int level);
    void init_tables(uint32_t table_idx);

    uint32_t m_table;
    uint32_t m_height, m_width;

    decode_t m_first_decode[32];
    decode_t m_second_decode[512];
    // for make_decoder
    decode_t *m_free; /* Next unused node */
    int m_leaf;       /* no. of leaves already added */
    // for getbits
    uint32_t m_bitbuf;
    int m_vbits;
};
}
}

#endif
