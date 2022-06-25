/*
 * libopenraw - olympusdecompressor.cpp
 *
 * Copyright (C) 2011-2022 Hubert Figuière
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

static void decompressOlympus(const uint8_t* buffer, size_t size, uint16_t* data16,
                              uint32_t w, uint32_t h);

// decompression ported from RawSpeed.
static void decompressOlympus(const uint8_t* buffer, size_t size, uint16_t* data16,
                              uint32_t w, uint32_t h)
{
    int nbits, sign, low, high, n;
    // These are for handling even and odd rows.
    int wo[2] = { 0, 0 };
    int nw[2] = { 0, 0 };
    int acarry[2][3], pred, diff;

    // The pitch is for the predictor: two row up.
    int pitch = w * 2;

    /* Build a table to quickly look up "high" value */
    char bittable[4096];
    for (int i = 0; i < 4096; i++) {
        int b = i;
        for (high = 0; high < 12; high++) {
            if ((b >> (11 - high)) & 1) {
                break;
            }
        }
        bittable[i] = high;
    }

    buffer += 7;

    BitIterator bits(buffer, size - 7);

    for (uint32_t y = 0; y < h; y++) {
        memset(acarry, 0, sizeof acarry);

        uint16_t* dest = &data16[(y * pitch)/2];
        for (uint32_t x = 0; x < w / 2; x++) {
            auto col = x * 2;
            for (int p = 0; p < 2; p++) {
                int i = 2 * (acarry[p][2] < 3);
                for (nbits = 2 + i; (uint16_t)acarry[p][0] >> (nbits + i); nbits++) {
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

                acarry[p][0] = (high << nbits) | bits.get(nbits);
                diff = (acarry[p][0] ^ sign) + acarry[p][1];
                acarry[p][1] = (diff * 3 + acarry[p][1]) >> 5;
                acarry[p][2] = acarry[p][0] > 16 ? 0 : acarry[p][2] + 1;

                if (y < 2 || col < 2) {
                    if (y < 2 && col < 2) {
                        pred = 0;
                    } else if (y < 2) {
                        pred = wo[p];
                    } else {
                        // The (int) cast is required as col is unsigned
                        // and cause type promotion of the negative index.
                        pred = dest[-pitch + (int)(col + p)];
                        nw[p] = pred;
                    }
                    dest[col + p] = pred + ((diff << 2) | low);
                    // Set predictor
                    wo[p] = dest[col + p];
                } else {
                    // See above for the cast.
                    n = dest[-pitch + (int)(col + p)];
                    if (((wo[p] < nw[p]) & (nw[p] < n)) | ((n < nw[p]) & (nw[p] < wo[p]))) {
                        if (abs(wo[p] - nw[p]) > 32 || abs(n - nw[p]) > 32) {
                            pred = wo[p] + n - nw[p];
                        } else {
                            pred = (wo[p] + n) >> 1;
                        }
                    } else {
                        pred = abs(wo[p] - nw[p]) > abs(n - nw[p]) ? wo[p] : n;
                    }

                    dest[col + p] = pred + ((diff << 2) | low);
                    // Set predictors
                    wo[p] = dest[col + p];
                    nw[p] = n;
                }
            }
        }
    }
}

RawDataPtr OlympusDecompressor::decompress()
{
    RawDataPtr output(new RawData);

    output->allocData(m_w * m_h * 2);
    decompressOlympus(m_buffer, m_size, (uint16_t*)output->data(), m_w, m_h);

    // hardcoded 12bits values
    output->setBpc(12);
    output->setWhiteLevel((1 << 12) - 1);

    return output;
}

}
}
