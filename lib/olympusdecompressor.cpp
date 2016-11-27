/*
 * libopenraw - olympusdecompressor.cpp
 *
 * Copyright (C) 2011-2016 Hubert Figuiere
 * Olympus Decompression copied from RawSpeed
 * Copyright (C) 2009 Klaus Post
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

#include <stdlib.h>
#include <string.h>

#include <algorithm>

#include "rawdata.hpp"
#include "olympusdecompressor.hpp"
#include "bititerator.hpp"

namespace OpenRaw {
namespace Internals {

static void decompressOlympus(const uint8_t* buffer, size_t size, uint8_t* data,
                              uint32_t w, uint32_t h);

// decompression ported from RawSpeed.
static void decompressOlympus(const uint8_t* buffer, size_t size, uint8_t* data,
                              uint32_t w, uint32_t h)
{
    int nbits, sign, low, high, i, wo0, n, nw0, wo1, nw1;
    int acarry0[3], acarry1[3], pred, diff;

    int pitch = w * 2; //(((w * 2/*bpp*/) + 15) / 16) * 16; // TODO make that
                       //part of the outer datas

    /* Build a table to quickly look up "high" value */
    char bittable[4096];
    for (i = 0; i < 4096; i++) {
        int b = i;
        for (high = 0; high < 12; high++) {
            if ((b >> (11 - high)) & 1) {
                break;
            }
        }
        bittable[i] = high;
    }
    wo0 = nw0 = wo1 = nw1 = 0;
    buffer += 7;

    BitIterator bits(buffer, size - 7);

    for (uint32_t y = 0; y < h; y++) {
        memset(acarry0, 0, sizeof acarry0);
        memset(acarry1, 0, sizeof acarry1);
        uint16_t* dest = (uint16_t*)&data[y * pitch];
        for (uint32_t x = 0; x < w; x++) {
            //			bits.checkPos();
            //			bits.fill();
            i = 2 * (acarry0[2] < 3);
            for (nbits = 2 + i; (uint16_t)acarry0[0] >> (nbits + i); nbits++) {
            }

            uint32_t b = bits.peek(15);
            sign = (b >> 14) * -1;
            low = (b >> 12) & 3;
            high = bittable[b & 4095];
            // Skip bits used above.
            bits.skip(std::min(12 + 3, high + 1 + 3));

            if (high == 12) {
                high = bits.get(16 - nbits) >> 1;
            }

            acarry0[0] = (high << nbits) | bits.get(nbits);
            diff = (acarry0[0] ^ sign) + acarry0[1];
            acarry0[1] = (diff * 3 + acarry0[1]) >> 5;
            acarry0[2] = acarry0[0] > 16 ? 0 : acarry0[2] + 1;

            if (y < 2 || x < 2) {
                if (y < 2 && x < 2) {
                    pred = 0;
                } else if (y < 2) {
                    pred = wo0;
                } else {
                    pred = dest[-pitch + ((int)x)];
                    nw0 = pred;
                }
                dest[x] = pred + ((diff << 2) | low);
                // Set predictor
                wo0 = dest[x];
            } else {
                n = dest[-pitch + ((int)x)];
                if (((wo0 < nw0) & (nw0 < n)) | ((n < nw0) & (nw0 < wo0))) {
                    if (abs(wo0 - nw0) > 32 || abs(n - nw0) > 32) {
                        pred = wo0 + n - nw0;
                    } else {
                        pred = (wo0 + n) >> 1;
                    }
                } else {
                    pred = abs(wo0 - nw0) > abs(n - nw0) ? wo0 : n;
                }

                dest[x] = pred + ((diff << 2) | low);
                // Set predictors
                wo0 = dest[x];
                nw0 = n;
            }
            //      _ASSERTE(0 == dest[x] >> 12) ;

            // ODD PIXELS
            x += 1;
            //			bits.checkPos();
            //			bits.fill();
            i = 2 * (acarry1[2] < 3);
            for (nbits = 2 + i; (uint16_t)acarry1[0] >> (nbits + i); nbits++) {
            }
            b = bits.peek(15);
            sign = (b >> 14) * -1;
            low = (b >> 12) & 3;
            high = bittable[b & 4095];
            // Skip bits used above.
            bits.skip(std::min(12 + 3, high + 1 + 3));

            if (high == 12) {
                high = bits.get(16 - nbits) >> 1;
            }

            acarry1[0] = (high << nbits) | bits.get(nbits);
            diff = (acarry1[0] ^ sign) + acarry1[1];
            acarry1[1] = (diff * 3 + acarry1[1]) >> 5;
            acarry1[2] = acarry1[0] > 16 ? 0 : acarry1[2] + 1;

            if (y < 2 || x < 2) {
                if (y < 2 && x < 2) {
                    pred = 0;
                } else if (y < 2) {
                    pred = wo1;
                } else {
                    pred = dest[-pitch + ((int)x)];
                    nw1 = pred;
                }
                dest[x] = pred + ((diff << 2) | low);
                // Set predictor
                wo1 = dest[x];
            } else {
                n = dest[-pitch + ((int)x)];
                if (((wo1 < nw1) & (nw1 < n)) | ((n < nw1) & (nw1 < wo1))) {
                    if (abs(wo1 - nw1) > 32 || abs(n - nw1) > 32) {
                        pred = wo1 + n - nw1;
                    } else {
                        pred = (wo1 + n) >> 1;
                    }
                } else {
                    pred = abs(wo1 - nw1) > abs(n - nw1) ? wo1 : n;
                }

                dest[x] = pred + ((diff << 2) | low);

                // Set predictors
                wo1 = dest[x];
                nw1 = n;
            }
            //      _ASSERTE(0 == dest[x] >> 12) ;
        }
    }
}

RawDataPtr OlympusDecompressor::decompress()
{
    RawDataPtr output(new RawData);

    output->allocData(m_w * m_h * 2);
    decompressOlympus(m_buffer, m_size, (uint8_t*)output->data(), m_w, m_h);

    // hardcoded 12bits values
    output->setBpc(12);
    output->setWhiteLevel((1 << 12) - 1);

    return output;
}

}
}
