// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - nikon/huffman.rs
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

//! Huffman decoding

use bitreader::BitReader;

use crate::Result;

/// Huffman node in the tree
/// If the decoded bit is 1, read `.1` to know the next index,
/// otherwise move to the next index.
/// If `.0` is true, then `.1` is a decoded value.
/// XXX see about making it 32bits long.
pub type HuffmanNode = (bool, u32);

pub struct HuffmanDecoder {
    table: &'static [HuffmanNode],
}

impl HuffmanDecoder {
    pub fn new(table: &'static [HuffmanNode]) -> HuffmanDecoder {
        HuffmanDecoder { table }
    }

    /// Decode the next value from the bit reader.
    pub fn decode(&self, bitreader: &mut BitReader) -> Result<u32> {
        let mut cur = 0_u32;
        while !self.table[cur as usize].0 {
            let bit = bitreader.read_u8(1)?;
            if bit != 0 {
                cur = self.table[cur as usize].1;
            } else {
                cur += 1;
            }
        }
        Ok(self.table[cur as usize].1)
    }
}

#[cfg(test)]
mod test {

    use super::{HuffmanDecoder, HuffmanNode};

    const LOSSY_12BIT: [HuffmanNode; 27] = [
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

    #[test]
    fn test_huffman() {
        let bits = vec![0b0001_0011, 0b1001_0111, 0b0011_1000];

        let decoder = HuffmanDecoder::new(&LOSSY_12BIT);
        let mut bitreader = bitreader::BitReader::new(&bits);

        assert_eq!(decoder.decode(&mut bitreader), Ok(5));
        assert_eq!(decoder.decode(&mut bitreader), Ok(4));
        assert_eq!(decoder.decode(&mut bitreader), Ok(3));
        assert_eq!(decoder.decode(&mut bitreader), Ok(6));
        assert_eq!(decoder.decode(&mut bitreader), Ok(2));
        assert_eq!(decoder.decode(&mut bitreader), Ok(7));
        assert_eq!(decoder.decode(&mut bitreader), Ok(3));
        assert_eq!(decoder.decode(&mut bitreader), Ok(6));
    }
}
