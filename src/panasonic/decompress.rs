// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - panasonic/decompress.rs
 *
 * Copyright (C) 2022-2024 Hubert Figuière
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

/*! The algorithm from dcraw, turned into a readable and analyzeable form.

Additional, human-readable reference from https://www.dpreview.com/forums/post/40154581 (but this doesn't mention that the first *nonzero* pixel is lossless).
*/

use crate::{Error, Result};

#[derive(Debug, Clone)]
struct ReverseBits(pub [u8; 16]);

impl ReverseBits {
    /// Gets up to 8 bits from the group. Starting with the last byte. Most significant bits of each byte go first into most significant bits of output. See test if this is confusing.
    pub fn get(&self, bit_index: usize, count: u8) -> u8 {
        let (data, _byte_index, bit_offset) = self.get_internal(bit_index, count);
        let mask = !(!0u16 << count) as u8;
        (data >> bit_offset) as u8 & mask
    }

    fn get_internal(&self, bit_index: usize, count: u8) -> (u16, usize, usize) {
        let bit_index = 16 * 8 - bit_index - count as usize;
        let byte_index = bit_index / 8;
        let data =
            (*self.0.get(byte_index + 1).unwrap_or(&0) as u16) << 8 | self.0[byte_index] as u16;
        let bit_offset = bit_index % 8;
        (data, byte_index, bit_offset)
    }
}

struct BitsCursor {
    pos: usize,
    bits: ReverseBits,
}

impl BitsCursor {
    fn new(bits: ReverseBits) -> Self {
        Self { pos: 0, bits }
    }
    fn next(&mut self, count: u8) -> u8 {
        let ret = self.bits.get(self.pos, count);
        self.pos += count as usize;
        ret
    }
}

fn decode_next_px(j: u8, shift: u8, prev: u16) -> u16 {
    let magnitude = 0x80 << shift;
    if j != 0 {
        let j = j as u16;
        // This is the lossy part.
        if magnitude > prev || shift == 4 {
            // If shift > 0 then previous pixel data gets replaced, accidental LSBs get carried from old value.
            (j << shift) | (prev & !(!0 << shift))
        } else {
            // If shift > 0 then the encoder dropped the LSBs.

            // Pretty-print for the actual difference value.
            // I'm not using this exact calculation to stay in u16
            //dbg!((j << shift) as i16 - magnitude as i16);
            prev - magnitude + (j << shift)
        }
    } else {
        prev
    }
}

/// Returns pixels and shift anomaly for each chunk.
/// Every chunk contains pixels of 2 colors, with the initial pixels stored losslessly.
/// Pixels are composed of either 8 or 12 bits.
/// Zero pixels are 8 bits.
/// Then, the first nonzero pixel of each color is 12 bits, requiring an additional read of 4 bits.
/// Following pixels are 8 bits.
fn decode_chunk(bits: ReverseBits) -> [u16; 14] {
    /* This is written in a streaming, mutable fashion: every access to bits must be in order.
     * The bit fields within a chunk are not constant and depend on previous reads.
     */
    let mut bits = BitsCursor::new(bits);
    // Tracks whether a nonzero pixel was encountered in this color. Colors alternate.
    let mut color_nonzero = [false, false];
    let mut out = [0u16; 14];

    fn load_initial_px(color_idx: usize, nonzero: &mut [bool; 2], bits: &mut BitsCursor) -> u16 {
        let top = bits.next(8);
        let bottom = if top == 0 && !nonzero[color_idx] {
            0
        } else {
            nonzero[color_idx] = true;
            bits.next(4) as u16
        };
        (top as u16) << 4 | bottom
    }

    // 2 initial pixels
    out[0] = load_initial_px(0, &mut color_nonzero, &mut bits);
    out[1] = load_initial_px(1, &mut color_nonzero, &mut bits);
    // 4 independent differential groups in every chunk
    for diffidx in 0..4 {
        let shift = bits.next(2);
        let shift = 4 >> (3 - shift);
        // 3 pixels in every group, chained to the previous pixel of the same color
        for pxidx in 0..3 {
            let px_allidx = 2 + diffidx * 3 + pxidx;
            let coloridx = px_allidx & 1;
            let prev = out[px_allidx - 2];
            let px = if color_nonzero[coloridx] {
                decode_next_px(bits.next(8), shift, prev)
            } else {
                load_initial_px(coloridx, &mut color_nonzero, &mut bits)
            };
            /* TODO (done? this seems to be the "first nonzero pixel is lossless" problem):
             * dcraw code does an odd thing:
             * it will read extra 4 bits for the last 2 pixels if there's all 0's in the chunk. This should send the stream out of whack.
             * The pana_bits reader strongly suggests that the stream of data is separated into 16-byte chunks, so reading another byte (or half-byte if interrupted) would contradict it.
             */
            out[px_allidx] = px;
        }
    }
    out
}

/// Converts chunk index to first byte offset within block
fn chunk_to_offset(idx: usize) -> usize {
    if idx > 0x200 {
        idx * 16 - 0x2008
    } else {
        idx * 16 + 0x1ff8
    }
}

/// Each block of 0x4000 bytes is split into 16-byte groups. First group starts in the middle of the block, reaching the end the groups wrap back to start of the block, splitting the boundary one into two halves.
fn block_get_chunk(data: &[u8], chunk_idx: usize) -> [u8; 16] {
    let block_idx = chunk_idx * 16 / 0x4000;
    let block = &data[block_idx * 0x4000..][..0x4000];
    let chunks_in_block = 0x4000 / 16;
    let chunk_idx = chunk_idx % chunks_in_block;
    let data_offset = chunk_to_offset(chunk_idx);
    let mut out = [0; 16];
    if data_offset == 0x3ff8 {
        out[0..8].copy_from_slice(&block[data_offset..][..8]);
        out[8..16].copy_from_slice(&block[0..8]);
    } else {
        out[0..16].copy_from_slice(&block[data_offset..][..16]);
    }
    out
}

fn iter_chunks(data: &[u8]) -> impl Iterator<Item = [u8; 16]> + '_ {
    (0..(data.len() / 16)).map(|i| block_get_chunk(data, i))
}

pub fn panasonic_raw1(data: &[u8]) -> Result<Vec<u16>> {
    if data.len() % 0x4000 == 0 {
        let mut out: Vec<u16> = vec![0; data.len() * 14 / 16];
        iter_chunks(data)
            .map(|chunk| decode_chunk(ReverseBits(chunk)))
            .zip(out.chunks_exact_mut(14))
            .for_each(|(decoded, out)| out.copy_from_slice(&decoded[..]));
        Ok(out)
    } else {
        Err(Error::UnexpectedEOF)
    }
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn revbits() {
        let mut ar = [0; 16];
        ar[15] = 0x0b;
        ar[14] = 0xf0;
        ar[13] = 0xc6;
        ar[12] = 0x20;
        ar[11] = 0x1f;
        let ar = ReverseBits(ar);
        assert_eq!(ar.get(0, 8), 0x0b);
        assert_eq!(ar.get(8, 4), 0xf);
        assert_eq!(ar.get(12, 8), 0x0c);
        assert_eq!(ar.get(20, 4), 0x6);
        assert_eq!(ar.get(24, 2), 0x0);
        assert_eq!(ar.get(26, 8), 0x80);
    }

    #[test]
    fn cto() {
        assert_eq!(chunk_to_offset(0), 0x1ff8);
        assert_eq!(chunk_to_offset(0x200), 0x3ff8);
        assert_eq!(chunk_to_offset(0x201), 0x8);
        assert_eq!(chunk_to_offset(0x3ff), 0x1fe8);
    }

    #[test]
    fn decode1() {
        let ar = ReverseBits([
            0x90, 0x7A, 0x8A, 0x18, 0x02, 0x26, 0x92, 0xC7, 0xB7, 0x48, 0x20, 0x1F, 0x20, 0xC6,
            0xF0, 0x0B,
        ]);
        let pixels = decode_chunk(ar);
        assert_eq!(
            pixels,
            [0xbf, 0xc6, 0xbf, 0xc2, 0xc0, 0xcd, 0xbc, 0xc6, 0xc5, 0xc6, 0xcb, 0xd0, 0xc5, 0xe0],
            "{:#x?}",
            &pixels,
        );
    }

    #[test]
    fn decode2() {
        let ar = ReverseBits([
            0x66, 0x73, 0xd2, 0x21, 0x22, 0x1d, 0xc9, 0x24, 0xd2, 0x55, 0x9a, 0x70, 0x7a, 0x4b,
            0xf1, 0x17,
        ]);
        let pixels = decode_chunk(ar);
        assert_eq!(
            pixels,
            [
                0x17f, 0x14b, 0x251, 0x1cf, 0x223, 0x189, 0x167, 0x121, 0x11f, 0x121, 0x223, 0x1c5,
                0x209, 0x191
            ],
            "{:#x?}",
            &pixels,
        );
    }

    #[test]
    fn decode3() {
        let ar = ReverseBits([
            0x73, 0x81, 0x7f, 0x40, 0x9a, 0xce, 0xf1, 0x64, 0x0a, 0xcd, 0x1a, 0x82, 0xe8, 0x01,
            0x90, 0x14,
        ]);
        let pixels = decode_chunk(ar);
        assert_eq!(
            pixels,
            [
                0x149, 0x0, 0x143, 0x208, 0x12e, 0x258, 0x154, 0x227, 0x147, 0x24d, 0x157, 0x24c,
                0x158, 0x23f
            ],
            "{:#x?}",
            &pixels,
        );
    }

    #[test]
    fn iter_chunks_test() {
        assert_eq!(iter_chunks(&[0; 0x4000]).count(), 0x4000 / 16);
        assert_eq!(iter_chunks(&[0; 0x8000]).count(), 0x8000 / 16);
    }
}
