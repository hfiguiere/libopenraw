// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - olympus/decompress.rs
 *
 * Copyright (C) 2022 Hubert Figuière
 * Olympus Decompression ported to Rust from RawSpeed (via libopenraw)
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

//! Olympus decompression

use bitreader::BitReader;

use crate::{Error, Result};

pub fn decompress_olympus(input: &[u8], w: usize, h: usize) -> Result<Vec<u16>> {
    if input.len() < 8 {
        return Err(Error::Decompression(
            "ORF: Compressed data too small.".into(),
        ));
    }
    let mut output: Vec<u16> = vec![0; h * w];
    let data = output.as_mut_slice();

    // XXX can we avoid the initialization to 0.
    // XXX this can be computed at compile time.
    let mut bittable = [0_u8; 4096];
    for (i, value) in bittable.iter_mut().enumerate() {
        *value = 12;
        for high in 0..12 {
            if ((i >> (11 - high)) & 1) != 0 {
                *value = high;
                break;
            }
        }
    }
    let mut wo = [0_i32; 2];
    let mut nw = [0_i32; 2];

    // Skip the first 7. No idea why.
    let input = &input[7..];

    let mut bits = BitReader::new(input);

    for y in 0..h {
        let mut acarry = [[0_i32; 3]; 2];

        for x in 0..(w / 2) {
            let col: usize = x * 2;
            // p is for parity. We go even and odds raw.
            for p in 0..2 {
                let i = if acarry[p][2] < 3 { 2 } else { 0 };
                let mut nbits = 2 + i;
                while (nbits + i) < 16 && acarry[p][0] as u16 >> (nbits + i) != 0 {
                    nbits += 1;
                }
                assert!(nbits <= 16);

                // There is no peek i32. But we ware getting 15 bits,
                // so this should fit.
                let b: i32 = bits
                    .peek_u16(15)
                    .or_else(|_| {
                        // peek_u16() will return an error if there is not enough bits.
                        let remaining = bits.remaining();
                        // We expect 15 bits, but there isn't enough
                        // so we have to pad / shift.
                        bits.peek_u16(remaining as u8)
                            .map(|v| v << (15 - remaining))
                    })
                    .map_err(|e| {
                        log::error!("1 bits.peek_u32(15)");
                        e
                    })? as i32;

                let sign: i32 = -(b >> 14);
                let low: i32 = (b >> 12) & 3;
                let mut high: i32 = bittable[(b & 4095) as usize] as i32;
                // Skip bits used above.
                bits.skip(std::cmp::min(12 + 3, high + 4) as u64)
                    .map_err(|e| {
                        log::error!("2 bits.skip({})", std::cmp::min(12 + 3, high + 4));
                        e
                    })?;

                if high == 12 {
                    high = (bits.read_u16(16 - nbits).map_err(|e| {
                        log::error!("3 bits.skip(16 - {})", nbits);
                        e
                    })? >> 1) as i32;
                }
                let ibits = bits.read_u16(nbits)? as i32;
                acarry[p][0] = (high << nbits) | ibits;
                let diff: i32 = (acarry[p][0] ^ sign) + acarry[p][1];
                acarry[p][1] = (diff * 3 + acarry[p][1]) >> 5;
                acarry[p][2] = if acarry[p][0] > 16 {
                    0
                } else {
                    acarry[p][2] + 1
                };

                data[y * w + col + p] = if y < 2 || col < 2 {
                    let pred = if y < 2 && col < 2 {
                        0
                    } else if y < 2 {
                        wo[p]
                    } else {
                        nw[p] = data[(y - 2) * w + col + p] as i32;
                        nw[p]
                    };
                    // Set predictor
                    wo[p] = pred + ((diff << 2) | low);
                    wo[p] as u16
                } else {
                    let n: i32 = data[(y - 2) * w + col + p] as i32;
                    let pred =
                        if ((wo[p] < nw[p]) && (nw[p] < n)) || ((n < nw[p]) && (nw[p] < wo[p])) {
                            if (wo[p] - nw[p]).abs() > 32 || (n - nw[p]).abs() > 32 {
                                wo[p] + n - nw[p]
                            } else {
                                (wo[p] + n) >> 1
                            }
                        } else if (wo[p] - nw[p]).abs() > (n - nw[p]).abs() {
                            wo[p]
                        } else {
                            n
                        };

                    nw[p] = n;
                    // Set predictors
                    wo[p] = pred + ((diff << 2) | low);
                    wo[p] as u16
                };
            }
        }
    }

    Ok(output)
}
