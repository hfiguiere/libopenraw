// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - canon/crw/decompress.rs
 *
 * Copyright (C) 2022-2023 Hubert Figuière
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

use std::io::SeekFrom;

use byteorder::ReadBytesExt;

use crate::mosaic::Pattern;
use crate::rawfile::ReadAndSeek;
use crate::{DataType, RawImage, Result};

const FIRST_TREE: [[u8; 29]; 3] = [
    [
        0, 1, 4, 2, 3, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x04, 0x03, 0x05, 0x06, 0x02, 0x07, 0x01,
        0x08, 0x09, 0x00, 0x0a, 0x0b, 0xff,
    ],
    [
        0, 2, 2, 3, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0x03, 0x02, 0x04, 0x01, 0x05, 0x00, 0x06,
        0x07, 0x09, 0x08, 0x0a, 0x0b, 0xff,
    ],
    [
        0, 0, 6, 3, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x06, 0x05, 0x07, 0x04, 0x08, 0x03, 0x09,
        0x02, 0x00, 0x0a, 0x01, 0x0b, 0xff,
    ],
];

const SECOND_TREE: [[u8; 180]; 3] = [
    [
        0, 2, 2, 2, 1, 4, 2, 1, 2, 5, 1, 1, 0, 0, 0, 139, 0x03, 0x04, 0x02, 0x05, 0x01, 0x06, 0x07,
        0x08, 0x12, 0x13, 0x11, 0x14, 0x09, 0x15, 0x22, 0x00, 0x21, 0x16, 0x0a, 0xf0, 0x23, 0x17,
        0x24, 0x31, 0x32, 0x18, 0x19, 0x33, 0x25, 0x41, 0x34, 0x42, 0x35, 0x51, 0x36, 0x37, 0x38,
        0x29, 0x79, 0x26, 0x1a, 0x39, 0x56, 0x57, 0x28, 0x27, 0x52, 0x55, 0x58, 0x43, 0x76, 0x59,
        0x77, 0x54, 0x61, 0xf9, 0x71, 0x78, 0x75, 0x96, 0x97, 0x49, 0xb7, 0x53, 0xd7, 0x74, 0xb6,
        0x98, 0x47, 0x48, 0x95, 0x69, 0x99, 0x91, 0xfa, 0xb8, 0x68, 0xb5, 0xb9, 0xd6, 0xf7, 0xd8,
        0x67, 0x46, 0x45, 0x94, 0x89, 0xf8, 0x81, 0xd5, 0xf6, 0xb4, 0x88, 0xb1, 0x2a, 0x44, 0x72,
        0xd9, 0x87, 0x66, 0xd4, 0xf5, 0x3a, 0xa7, 0x73, 0xa9, 0xa8, 0x86, 0x62, 0xc7, 0x65, 0xc8,
        0xc9, 0xa1, 0xf4, 0xd1, 0xe9, 0x5a, 0x92, 0x85, 0xa6, 0xe7, 0x93, 0xe8, 0xc1, 0xc6, 0x7a,
        0x64, 0xe1, 0x4a, 0x6a, 0xe6, 0xb3, 0xf1, 0xd3, 0xa5, 0x8a, 0xb2, 0x9a, 0xba, 0x84, 0xa4,
        0x63, 0xe5, 0xc5, 0xf3, 0xd2, 0xc4, 0x82, 0xaa, 0xda, 0xe4, 0xf2, 0xca, 0x83, 0xa3, 0xa2,
        0xc3, 0xea, 0xc2, 0xe2, 0xe3, 0xff, 0xff,
    ],
    [
        0, 2, 2, 1, 4, 1, 4, 1, 3, 3, 1, 0, 0, 0, 0, 140, 0x02, 0x03, 0x01, 0x04, 0x05, 0x12, 0x11,
        0x06, 0x13, 0x07, 0x08, 0x14, 0x22, 0x09, 0x21, 0x00, 0x23, 0x15, 0x31, 0x32, 0x0a, 0x16,
        0xf0, 0x24, 0x33, 0x41, 0x42, 0x19, 0x17, 0x25, 0x18, 0x51, 0x34, 0x43, 0x52, 0x29, 0x35,
        0x61, 0x39, 0x71, 0x62, 0x36, 0x53, 0x26, 0x38, 0x1a, 0x37, 0x81, 0x27, 0x91, 0x79, 0x55,
        0x45, 0x28, 0x72, 0x59, 0xa1, 0xb1, 0x44, 0x69, 0x54, 0x58, 0xd1, 0xfa, 0x57, 0xe1, 0xf1,
        0xb9, 0x49, 0x47, 0x63, 0x6a, 0xf9, 0x56, 0x46, 0xa8, 0x2a, 0x4a, 0x78, 0x99, 0x3a, 0x75,
        0x74, 0x86, 0x65, 0xc1, 0x76, 0xb6, 0x96, 0xd6, 0x89, 0x85, 0xc9, 0xf5, 0x95, 0xb4, 0xc7,
        0xf7, 0x8a, 0x97, 0xb8, 0x73, 0xb7, 0xd8, 0xd9, 0x87, 0xa7, 0x7a, 0x48, 0x82, 0x84, 0xea,
        0xf4, 0xa6, 0xc5, 0x5a, 0x94, 0xa4, 0xc6, 0x92, 0xc3, 0x68, 0xb5, 0xc8, 0xe4, 0xe5, 0xe6,
        0xe9, 0xa2, 0xa3, 0xe3, 0xc2, 0x66, 0x67, 0x93, 0xaa, 0xd4, 0xd5, 0xe7, 0xf8, 0x88, 0x9a,
        0xd7, 0x77, 0xc4, 0x64, 0xe2, 0x98, 0xa5, 0xca, 0xda, 0xe8, 0xf3, 0xf6, 0xa9, 0xb2, 0xb3,
        0xf2, 0xd2, 0x83, 0xba, 0xd3, 0xff, 0xff,
    ],
    [
        0, 0, 6, 2, 1, 3, 3, 2, 5, 1, 2, 2, 8, 10, 0, 117, 0x04, 0x05, 0x03, 0x06, 0x02, 0x07,
        0x01, 0x08, 0x09, 0x12, 0x13, 0x14, 0x11, 0x15, 0x0a, 0x16, 0x17, 0xf0, 0x00, 0x22, 0x21,
        0x18, 0x23, 0x19, 0x24, 0x32, 0x31, 0x25, 0x33, 0x38, 0x37, 0x34, 0x35, 0x36, 0x39, 0x79,
        0x57, 0x58, 0x59, 0x28, 0x56, 0x78, 0x27, 0x41, 0x29, 0x77, 0x26, 0x42, 0x76, 0x99, 0x1a,
        0x55, 0x98, 0x97, 0xf9, 0x48, 0x54, 0x96, 0x89, 0x47, 0xb7, 0x49, 0xfa, 0x75, 0x68, 0xb6,
        0x67, 0x69, 0xb9, 0xb8, 0xd8, 0x52, 0xd7, 0x88, 0xb5, 0x74, 0x51, 0x46, 0xd9, 0xf8, 0x3a,
        0xd6, 0x87, 0x45, 0x7a, 0x95, 0xd5, 0xf6, 0x86, 0xb4, 0xa9, 0x94, 0x53, 0x2a, 0xa8, 0x43,
        0xf5, 0xf7, 0xd4, 0x66, 0xa7, 0x5a, 0x44, 0x8a, 0xc9, 0xe8, 0xc8, 0xe7, 0x9a, 0x6a, 0x73,
        0x4a, 0x61, 0xc7, 0xf4, 0xc6, 0x65, 0xe9, 0x72, 0xe6, 0x71, 0x91, 0x93, 0xa6, 0xda, 0x92,
        0x85, 0x62, 0xf3, 0xc5, 0xb2, 0xa4, 0x84, 0xba, 0x64, 0xa5, 0xb3, 0xd2, 0x81, 0xe5, 0xd3,
        0xaa, 0xc4, 0xca, 0xf2, 0xb1, 0xe4, 0xd1, 0x83, 0x63, 0xea, 0xc3, 0xe2, 0x82, 0xf1, 0xa3,
        0xc2, 0xa1, 0xc1, 0xe3, 0xa2, 0xe1, 0xff, 0xff,
    ],
];

#[derive(Copy, Clone, Default, Debug)]
struct DecoderNode {
    branch: [usize; 2],
    leaf: u8,
}

// XXX see about replacing it with the crate
#[derive(Default)]
struct BitPump {
    bitbuf: u32,
    vbits: i32,
}

impl BitPump {
    // if nbits = -1 it will initialise the buffer.
    // nbit should be 0 <= n <= 25
    fn get_bits(&mut self, reader: &mut dyn ReadAndSeek, nbits: i16) -> Result<i32> {
        let ret;

        if nbits == 0 {
            return Ok(0);
        }
        if nbits == -1 {
            ret = 0;
            self.bitbuf = 0;
            self.vbits = 0;
        } else {
            ret = (self.bitbuf << (32 - self.vbits) >> (32 - nbits)) as i32;
            self.vbits -= nbits as i32;
        }
        while self.vbits < 25 {
            let c = reader.read_u8()?;
            self.bitbuf = (self.bitbuf << 8) + c as u32;
            if c == 0xff {
                // there is always 00 after ff
                reader.read_u8()?;
            }
            self.vbits += 8;
        }
        Ok(ret)
    }
}

pub(super) struct Decompress {
    width: u32,
    height: u32,
    first_decoder: [DecoderNode; 32],
    second_decoder: [DecoderNode; 512],
}

#[derive(Default)]
struct State {
    free: usize,
    leaf: usize,
}

/*
  A rough description of Canon's compression algorithm:

  +  Each pixel outputs a 10-bit sample, from 0 to 1023.
  +  Split the data into blocks of 64 samples each.
  +  Subtract from each sample the value of the sample two positions
  to the left, which has the same color filter.  From the two
  leftmost samples in each row, subtract 512.
  +  For each nonzero sample, make a token consisting of two four-bit
  numbers.  The low nibble is the number of bits required to
  represent the sample, and the high nibble is the number of
  zero samples preceding this sample.
  +  Output this token as a variable-length bitstring using
  one of three tablesets.  Follow it with a fixed-length
  bitstring containing the sample.

  The "first_decode" table is used for the first sample in each
  block, and the "second_decode" table is used for the others.
*/

/*
  Construct a decode tree according the specification in *source.
  The first 16 bytes specify how many codes should be 1-bit, 2-bit
  3-bit, etc.  Bytes after that are the leaf values.

  For example, if the source is

  { 0,1,4,2,3,1,2,0,0,0,0,0,0,0,0,0,
  0x04,0x03,0x05,0x06,0x02,0x07,0x01,0x08,0x09,0x00,0x0a,0x0b,0xff  },

  then the code is

  00		0x04
  010		0x03
  011		0x05
  100		0x06
  101		0x02
  1100		0x07
  1101		0x01
  11100		0x08
  11101		0x09
  11110		0x00
  111110		0x0a
  1111110		0x0b
  1111111		0xff
*/
fn make_decoder(
    state: &mut State,
    dest: &mut [DecoderNode],
    idx: usize,
    table: &[u8],
    level: usize,
) {
    state.free += 1;

    let mut i = 0_usize;
    let mut next = 0;
    while i <= state.leaf && next < 16 {
        i += table[next] as usize;
        next += 1;
    }

    if i > state.leaf {
        if level < next {
            dest[idx].branch[0] = state.free;
            make_decoder(state, dest, state.free, table, level + 1);
            dest[idx].branch[1] = state.free;
            make_decoder(state, dest, state.free, table, level + 1);
        } else {
            dest[idx].leaf = table[16 + state.leaf];
            state.leaf += 1;
        }
    }
}

fn canon_has_lowbits(reader: &mut dyn ReadAndSeek) -> Result<bool> {
    let mut test = [0_u8; 0x4000 - 26];
    let mut ret = false;
    reader.rewind()?;
    reader.read_exact(&mut test)?;
    for i in 514..test.len() - 1 {
        if test[i] == 0xff {
            if test[i + 1] != 0 {
                return Ok(true);
            }
            ret = false;
        }
    }

    Ok(ret)
}

impl Decompress {
    pub(super) fn new(table: usize, width: u32, height: u32) -> Decompress {
        let mut decompressor = Decompress {
            height,
            width,
            first_decoder: [DecoderNode::default(); 32],
            second_decoder: [DecoderNode::default(); 512],
        };
        decompressor.init_tables(table);

        decompressor
    }

    pub(super) fn decompress(&mut self, reader: &mut dyn ReadAndSeek) -> Result<RawImage> {
        let mut data = Vec::with_capacity(self.width as usize * self.height as usize);

        let lowbits = if canon_has_lowbits(reader)? { 1 } else { 0 };
        reader.seek(SeekFrom::Start(
            514 + lowbits * self.height as u64 * self.width as u64 / 4,
        ))?;

        let mut pump = BitPump::default();
        pump.get_bits(reader, -1)?;

        let mut column = 0_u32;
        let mut base = [0_i32; 2];
        let mut carry = 0_i32;
        let mut outbuf = [0_u16; 64];

        while column < self.width * self.height {
            let mut diffbuf = [0_i32; 64];
            let mut decoder: &[DecoderNode] = &self.first_decoder;
            let mut i = 0_usize;
            while i < 64 {
                let mut dindex = 0_usize;
                while decoder[dindex].branch[0] != 0 {
                    let bit = pump.get_bits(reader, 1)?;
                    dindex = decoder[dindex].branch[bit as usize];
                }
                let leaf = decoder[dindex].leaf;
                decoder = &self.second_decoder;

                if leaf == 0 && i != 0 {
                    break;
                }
                if leaf != 0xff {
                    i += (leaf >> 4) as usize;
                    let len = leaf & 15;
                    if len != 0 {
                        let mut diff = pump.get_bits(reader, len as i16)?;
                        if diff & (1 << (len - 1)) == 0 {
                            diff -= (1 << len) - 1;
                        }
                        if i < 64 {
                            diffbuf[i] = diff;
                        }
                    }
                }

                i += 1;
            }
            diffbuf[0] += carry;
            carry = diffbuf[0];
            for i in 0..64 {
                if column % self.width == 0 {
                    base[0] = 512;
                    base[1] = 512;
                }
                column += 1;
                base[i & 1] += diffbuf[i];
                outbuf[i] = base[i & 1] as u16;
            }
            if lowbits != 0 {
                let save = reader.stream_position()?;
                reader.seek(SeekFrom::Start((column as u64 - 64) / 4))?;
                let mut i = 0;
                for _ in 0..16 {
                    let c = reader.read_u8()? as u16;
                    for r in 0..4 {
                        // outbuf is 64, so we must check for it to not
                        // overflow (read out of bounds)
                        let next = if i < 63 { outbuf[i + 1] } else { 0 };
                        outbuf[i] = (next << 2) + ((c >> (r * 2)) & 3);

                        i += 1;
                    }
                }
                reader.seek(SeekFrom::Start(save))?;
            }
            data.extend_from_slice(&outbuf);
        }

        Ok(RawImage::with_data16(
            self.width,
            self.height,
            10,
            DataType::Raw,
            data,
            Pattern::default(),
        ))
    }

    fn init_tables(&mut self, table: usize) {
        let table = if table > 2 { 2 } else { table };

        let mut state = State::default();
        make_decoder(
            &mut state,
            &mut self.first_decoder,
            0,
            &FIRST_TREE[table],
            0,
        );
        let mut state = State::default();
        make_decoder(
            &mut state,
            &mut self.second_decoder,
            0,
            &SECOND_TREE[table],
            0,
        );
    }
}

#[cfg(test)]
mod test {
    use super::{make_decoder, DecoderNode, State};

    #[test]
    fn test_decode() {
        let table: [u8; 29] = [
            0, 1, 4, 2, 3, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x04, 0x03, 0x05, 0x06, 0x02, 0x07,
            0x01, 0x08, 0x09, 0x00, 0x0a, 0x0b, 0xff,
        ];

        let mut decoder = [DecoderNode::default(); 32];
        let mut state = State::default();
        make_decoder(&mut state, &mut decoder, 0, &table, 0);
        println!("tree {decoder:?}");
    }
}
