// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * libopenraw - pentax/decompress.rs
 *
 * Adapted from rawloader:
 * Copyright (C) Pedro Côrte-Real <pedro@pedrocr.net>
 *
 * Copyright (C) 2024 Hubert Figuière
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

//! Pentax decompression
//!
//! Adapted from <https://github.com/pedrocr/rawloader/blob/master/src/decoders/pef.rs>

use std::io::BufRead;

use crate::container::Endian;
use crate::Result;

const DECODE_CACHE_BITS: u32 = 13;

pub(super) fn decompress(
    src: &[u8],
    huff: Option<(&[u8], Endian)>,
    width: usize,
    height: usize,
) -> Result<Vec<u16>> {
    let mut out = vec![0_u16; width * height];
    let mut htable = HuffTable::default();

    /* Attempt to read huffman table, if found in MakerNote */
    if let Some((huff, endian)) = huff {
        let mut cursor = std::io::Cursor::new(huff);
        let depth: usize = (endian.read_u16_from(&mut cursor)? as usize + 12) & 0xf;
        // XXX depth > 16 will cause issues.

        cursor.consume(12);

        let mut v0 = [0_u32; 16];
        for value in v0.iter_mut().take(depth) {
            *value = endian.read_u16_from(&mut cursor)? as u32;
        }

        let mut v1 = [0_u32; 16];
        let pos = cursor.position() as usize;
        for (i, value) in huff[pos..].iter().enumerate().take(depth) {
            v1[i] = *value as u32;
        }

        // Calculate codes and store bitcounts
        let mut v2: [u32; 16] = [0; 16];
        for c in 0..depth {
            v2[c] = v0[c] >> (12 - v1[c]);
            htable.bits[v1[c] as usize] += 1;
        }

        // Find smallest
        for i in 0..depth {
            let mut sm_val: u32 = 0xfffffff;
            let mut sm_num: u32 = 0xff;
            for (j, value) in v2.iter().enumerate().take(depth) {
                if *value <= sm_val {
                    sm_num = j as u32;
                    sm_val = *value;
                }
            }
            htable.huffval[i] = sm_num;
            v2[sm_num as usize] = 0xffffffff;
        }
    } else {
        // Initialize with legacy data
        let pentax_tree: [u8; 29] = [
            0, 2, 3, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 3, 4, 2, 5, 1, 6, 0, 7, 8, 9, 10, 11,
            12,
        ];
        let mut acc: usize = 0;
        for (i, value) in pentax_tree.iter().enumerate().take(16) {
            htable.bits[i + 1] = *value as u32;
            acc += htable.bits[i + 1] as usize;
        }
        for i in 0..acc {
            htable.huffval[i] = pentax_tree[i + 16] as u32;
        }
    }

    htable.initialize()?;

    let mut pump = BitPumpMSB::new(src);
    let mut pred_up1: [i32; 2] = [0, 0];
    let mut pred_up2: [i32; 2] = [0, 0];
    let mut pred_left1: i32;
    let mut pred_left2: i32;

    for row in 0..height {
        pred_up1[row & 1] += htable.huff_decode(&mut pump)?;
        pred_up2[row & 1] += htable.huff_decode(&mut pump)?;
        pred_left1 = pred_up1[row & 1];
        pred_left2 = pred_up2[row & 1];
        out[row * width] = pred_left1 as u16;
        out[row * width + 1] = pred_left2 as u16;
        for col in (2..width).step_by(2) {
            pred_left1 += htable.huff_decode(&mut pump)?;
            pred_left2 += htable.huff_decode(&mut pump)?;
            out[row * width + col] = pred_left1 as u16;
            out[row * width + col + 1] = pred_left2 as u16;
        }
    }
    Ok(out)
}

pub(super) struct HuffTable {
    pub bits: [u32; 17],
    pub huffval: [u32; 256],
    nbits: u32,
    hufftable: Vec<(u8, u8)>,
    decodecache: [Option<(u8, i16)>; 1 << DECODE_CACHE_BITS],
}

impl Default for HuffTable {
    fn default() -> HuffTable {
        HuffTable {
            bits: [0; 17],
            huffval: [0; 256],
            nbits: 0,
            hufftable: vec![],
            decodecache: [None; 1 << DECODE_CACHE_BITS],
        }
    }
}

impl HuffTable {
    pub(super) fn initialize(&mut self) -> Result<()> {
        // XXX
        // Find out the max code length and allocate a table with that size
        self.nbits = 16;
        for i in 0..16 {
            if self.bits[16 - i] != 0 {
                break;
            }
            self.nbits -= 1;
        }
        self.hufftable = vec![(0, 0); 1 << self.nbits];

        // Fill in the table itself
        let mut h = 0;
        let mut pos = 0;
        for len in 0..self.nbits {
            for _ in 0..self.bits[len as usize + 1] {
                for _ in 0..(1 << (self.nbits - len - 1)) {
                    self.hufftable[h] = (len as u8 + 1, self.huffval[pos] as u8);
                    h += 1;
                }
                pos += 1;
            }
        }

        // Create the decode cache by running the slow code over all the possible
        // values DECODE_CACHE_BITS wide
        let mut pump = MockPump::default();
        let mut i = 0;
        loop {
            pump.set(i, DECODE_CACHE_BITS);
            let (bits, decode) = self.huff_decode_slow(&mut pump);
            if pump.validbits() >= 0 {
                self.decodecache[i as usize] = Some((bits, decode as i16));
            }
            i += 1;
            if i >= 1 << DECODE_CACHE_BITS {
                break;
            }
        }

        Ok(())
    }

    pub(super) fn huff_decode(&self, pump: &mut dyn BitPump) -> Result<i32> {
        let code = pump.peek_bits(DECODE_CACHE_BITS) as usize;
        if let Some((bits, decode)) = self.decodecache[code] {
            pump.consume_bits(bits as u32);
            Ok(decode as i32)
        } else {
            let decode = self.huff_decode_slow(pump);
            Ok(decode.1)
        }
    }

    fn huff_decode_slow(&self, pump: &mut dyn BitPump) -> (u8, i32) {
        let len = self.huff_len(pump);
        (len.0 + len.1, self.huff_diff(pump, len))
    }

    fn huff_len(&self, pump: &mut dyn BitPump) -> (u8, u8) {
        let code = pump.peek_bits(self.nbits) as usize;
        let (bits, len) = self.hufftable[code];
        pump.consume_bits(bits as u32);
        (bits, len)
    }

    fn huff_diff(&self, pump: &mut dyn BitPump, input: (u8, u8)) -> i32 {
        let (_, len) = input;

        match len {
            0 => 0,
            16 => -32768,
            len => {
                // decode the difference and extend sign bit
                let fulllen: i32 = len as i32;
                let bits = pump.get_bits(len as u32) as i32;
                let mut diff: i32 = ((bits << 1) + 1) >> 1;
                if (diff & (1 << (fulllen - 1))) == 0 {
                    diff -= (1 << fulllen) - 1;
                }
                diff
            }
        }
    }
}

pub trait BitPump {
    fn peek_bits(&mut self, num: u32) -> u32;
    fn consume_bits(&mut self, num: u32);

    #[inline(always)]
    fn get_bits(&mut self, num: u32) -> u32 {
        if num == 0 {
            return 0;
        }

        let val = self.peek_bits(num);
        self.consume_bits(num);

        val
    }
}

#[derive(Default)]
struct MockPump {
    bits: u64,
    nbits: u32,
}

impl MockPump {
    pub fn set(&mut self, bits: u32, nbits: u32) {
        self.bits = (bits as u64) << 32;
        self.nbits = nbits + 32;
    }

    pub fn validbits(&self) -> i32 {
        self.nbits as i32 - 32
    }
}

impl BitPump for MockPump {
    fn peek_bits(&mut self, num: u32) -> u32 {
        (self.bits >> (self.nbits - num)) as u32
    }

    fn consume_bits(&mut self, num: u32) {
        self.nbits -= num;
        self.bits &= (1 << self.nbits) - 1;
    }
}

pub struct BitPumpMSB<'a> {
    buffer: &'a [u8],
    pos: usize,
    bits: u64,
    nbits: u32,
}

impl<'a> BitPumpMSB<'a> {
    pub fn new(src: &'a [u8]) -> BitPumpMSB<'a> {
        BitPumpMSB {
            buffer: src,
            pos: 0,
            bits: 0,
            nbits: 0,
        }
    }
}

impl BitPump for BitPumpMSB<'_> {
    #[inline(always)]
    fn peek_bits(&mut self, num: u32) -> u32 {
        if num > self.nbits {
            let inbits = if self.pos + 4 >= self.buffer.len() {
                0_u64
            } else {
                Endian::Big.read_u32(&self.buffer[self.pos..]) as u64
            };
            self.bits = (self.bits << 32) | inbits;
            self.pos += 4;
            self.nbits += 32;
        }
        (self.bits >> (self.nbits - num)) as u32
    }

    #[inline(always)]
    fn consume_bits(&mut self, num: u32) {
        self.nbits -= num;
        self.bits &= (1 << self.nbits) - 1;
    }
}
