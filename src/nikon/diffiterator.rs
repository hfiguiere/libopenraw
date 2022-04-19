/*
 * libopenraw - nikon/diffiterator.rs
 *
 * Copyright (C) 2022 Hubert Figuière
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

use bitreader::BitReader;

use super::huffman::{HuffmanDecoder, HuffmanNode};
use crate::Result;

// 00              5
// 010             4
// 011             3
// 100             6
// 101             2
// 110             7
// 1110            1
// 11110           0
// 111110          8
// 1111110         9
// 11111110        11
// 111111110       10
// 1111111110      12
// 1111111111      0
pub const LOSSY_12BIT: [HuffmanNode; 27] = [
    /* 0  root       */ (false, 6),
    /* 1  0          */ (false, 3),
    /* 2  00         */ (true, 5),
    /* 3  01         */ (false, 5),
    /* 4  010        */ (true, 4),
    /* 5  011        */ (true, 3),
    /* 6  1          */ (false, 10),
    /* 7  10         */ (false, 9),
    /* 8  100        */ (true, 6),
    /* 9  101        */ (true, 2),
    /* 10 11         */ (false, 12),
    /* 11 110        */ (true, 7),
    /* 12 111        */ (false, 14),
    /* 13 1110       */ (true, 1),
    /* 14 1111       */ (false, 16),
    /* 15 11110      */ (true, 0),
    /* 16 11111      */ (false, 18),
    /* 17 111110     */ (true, 8),
    /* 18 111111     */ (false, 20),
    /* 19 1111110    */ (true, 9),
    /* 20 1111111    */ (false, 22),
    /* 21 11111110   */ (true, 11),
    /* 22 11111111   */ (false, 24),
    /* 23 111111110  */ (true, 10),
    /* 24 111111111  */ (false, 26),
    /* 25 1111111110 */ (true, 12),
    /* 26 1111111111 */ (true, 0),
];

// 00              5
// 010             6
// 011             4
// 100             7
// 101             8
// 1100            3
// 1101            9
// 11100           2
// 11101           1
// 111100          0
// 111101          10
// 111110          11
// 1111110         12
// 11111110        13
// 11111111        14
pub const LOSSY_14BIT: [HuffmanNode; 29] = [
    /* 0  root     */ (false, 6),
    /* 1  0        */ (false, 3),
    /* 2  00       */ (true, 5),
    /* 3  01       */ (false, 5),
    /* 4  010      */ (true, 6),
    /* 5  011      */ (true, 4),
    /* 6  1        */ (false, 10),
    /* 7  10       */ (false, 9),
    /* 8  100      */ (true, 7),
    /* 9  101      */ (true, 8),
    /* 10 11       */ (false, 14),
    /* 11 110      */ (false, 13),
    /* 12 1100     */ (true, 3),
    /* 13 1101     */ (true, 9),
    /* 14 111      */ (false, 18),
    /* 15 1110     */ (false, 17),
    /* 16 11100    */ (true, 2),
    /* 17 11101    */ (true, 1),
    /* 18 1111     */ (false, 22),
    /* 19 11110    */ (false, 21),
    /* 20 111100   */ (true, 0),
    /* 21 111101   */ (true, 10),
    /* 22 11111    */ (false, 24),
    /* 23 111110   */ (true, 11),
    /* 24 111111   */ (false, 26),
    /* 25 1111110  */ (true, 12),
    /* 26 1111111  */ (false, 28),
    /* 27 11111110 */ (true, 13),
    /* 28 11111111 */ (true, 14),
];

// 00              7
// 010             6
// 011             8
// 100             5
// 101             9
// 1100            4
// 1101            10
// 11100           3
// 11101           11
// 111100          12
// 111101          2
// 111110          0
// 1111110         1
// 11111110        13
// 11111111        14
pub const LOSSLESS_14BIT: [HuffmanNode; 29] = [
    /* 0  root     */ (false, 6),
    /* 1  0        */ (false, 3),
    /* 2  00       */ (true, 7),
    /* 3  01       */ (false, 5),
    /* 4  010      */ (true, 6),
    /* 5  011      */ (true, 8),
    /* 6  1        */ (false, 10),
    /* 7  10       */ (false, 9),
    /* 8  100      */ (true, 5),
    /* 9  101      */ (true, 9),
    /* 10 11       */ (false, 14),
    /* 11 110      */ (false, 13),
    /* 12 1100     */ (true, 4),
    /* 13 1101     */ (true, 10),
    /* 14 111      */ (false, 18),
    /* 15 1110     */ (false, 17),
    /* 16 11100    */ (true, 3),
    /* 17 11101    */ (true, 11),
    /* 18 1111     */ (false, 22),
    /* 19 11110    */ (false, 21),
    /* 20 111100   */ (true, 12),
    /* 21 111101   */ (true, 2),
    /* 22 11111    */ (false, 24),
    /* 23 111110   */ (true, 0),
    /* 24 111111   */ (false, 26),
    /* 25 1111110  */ (true, 1),
    /* 26 1111111  */ (false, 28),
    /* 27 11111110 */ (true, 13),
    /* 28 11111111 */ (true, 14),
];

pub(super) struct DiffIterator<'a> {
    decoder: HuffmanDecoder,
    bitreader: BitReader<'a>,
}

impl<'a> DiffIterator<'a> {
    pub fn new(table: &'static [HuffmanNode], bytes: &'a [u8]) -> DiffIterator<'a> {
        DiffIterator {
            decoder: HuffmanDecoder::new(table),
            bitreader: BitReader::new(bytes),
        }
    }

    pub fn get(&mut self) -> Result<i32> {
        let t = self.decoder.decode(&mut self.bitreader)?;
        let len = t & 15;
        let shl = t >> 4;

        let bits = self.bitreader.read_u32((len - shl) as u8)?;

        // casting as i32 allow get the signed int using the 1-complement
        // This was checked with asm comparison with the C code.
        let mut diff = ((((bits) << 1) + 1) << shl >> 1) as i32;
        // XXX the C++ code let the bit shift happen with len == 0.
        if len > 0 && (diff & (1 << (len - 1))) == 0 {
            // The original C code use bool to int implicit conversion for shl.
            diff -= (1 << len) - if shl == 0 { 1 } else { 0 };
        }

        Ok(diff)
    }
}

pub(super) struct CfaIterator<'a> {
    diffs: DiffIterator<'a>,
    columns: usize,
    row: usize,
    column: usize,
    vpred: [[u16; 2]; 2],
    hpred: [u16; 2],
}

impl<'a> CfaIterator<'a> {
    pub fn new(diffs: DiffIterator<'a>, columns: usize, init: [[u16; 2]; 2]) -> CfaIterator<'a> {
        let mut hpred = [0_u16; 2];
        let mut vpred = [[0_u16; 2]; 2];
        for i in 0..2 {
            for j in 0..2 {
                vpred[i][j] = init[i][j];
            }
            hpred[i] = 0x148;
        }

        CfaIterator::<'a> {
            diffs,
            columns,
            row: 0,
            column: 0,
            vpred,
            hpred,
        }
    }

    pub fn get(&mut self) -> Result<u16> {
        let diff = self.diffs.get()?;
        let ret = if self.column < 2 {
            let v = self.vpred[self.row & 1][self.column] as i32 + diff;
            self.vpred[self.row & 1][self.column] = v as u16;
            v
        } else {
            self.hpred[self.column & 1] as i32 + diff
        };

        self.hpred[self.column & 1] = ret as u16;

        self.column += 1;
        if self.column == self.columns {
            self.column = 0;
            self.row += 1;
        }

        Ok(ret as u16)
    }
}
