// SPDX-License-Identifier: LGPL-3.0-or-later AND IJG
/*
 * libopenraw - decompress/bit_reader.rs
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

use std::io::SeekFrom;

use byteorder::ReadBytesExt;

use crate::rawfile::ReadAndSeek;
use crate::{Error, Result};

const BITS_PER_LONG: u8 = 8 * std::mem::size_of::<i32>() as u8;
const MIN_GET_BITS: u8 = BITS_PER_LONG - 7; // max value for long get_buffer

// BMASK[n] is mask for n rightmost bits
const BMASK: [u16; 17] = [
    0x0000, 0x0001, 0x0003, 0x0007, 0x000F, 0x001F, 0x003F, 0x007F, 0x00FF, 0x01FF, 0x03FF, 0x07FF,
    0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF,
];

/// Bit reader state
pub(crate) struct BitReader {
    bits_left: u8,
    buffer: u32,
}

impl BitReader {
    pub fn new() -> BitReader {
        BitReader {
            bits_left: 0,
            buffer: 0,
        }
    }

    /// Discard current bits and return the number of whole bytes that were
    /// left
    #[inline]
    pub(crate) fn discard_bits(&mut self) -> u8 {
        let nbytes = self.bits_left / 8;
        self.bits_left = 0;

        nbytes
    }

    #[inline]
    pub(crate) fn show_bits8(&mut self, reader: &mut dyn ReadAndSeek) -> Result<u16> {
        if self.bits_left < 8 {
            self.fill_bit_buffer(reader, 8)?;
        }

        Ok(((self.buffer >> (self.bits_left - 8)) & 0xff) as u16)
    }

    #[inline]
    pub(crate) fn flush_bits(&mut self, nbits: u8) {
        self.bits_left -= nbits;
    }

    #[inline]
    pub(crate) fn get_bits(&mut self, reader: &mut dyn ReadAndSeek, nbits: u8) -> Result<u16> {
        if nbits >= 17 {
            return Err(Error::Decompression(format!(
                "LJPEG: Tried to request {nbits} bits (max 16), JPEG is likely corrupt"
            )));
        }
        if self.bits_left < nbits {
            self.fill_bit_buffer(reader, nbits)?;
        }
        self.bits_left -= nbits;
        Ok((self.buffer >> self.bits_left) as u16 & BMASK[nbits as usize])
    }

    #[inline]
    pub(crate) fn get_bit(&mut self, reader: &mut dyn ReadAndSeek) -> Result<u16> {
        if self.bits_left == 0 {
            self.fill_bit_buffer(reader, 1)?;
        }

        self.bits_left -= 1;
        Ok(((self.buffer >> self.bits_left) & 1) as u16)
    }

    // Load up the bit buffer with at least nbits
    // Process any stuffed bytes at this time.
    fn fill_bit_buffer(&mut self, reader: &mut dyn ReadAndSeek, nbits: u8) -> Result<()> {
        while self.bits_left < MIN_GET_BITS {
            let mut c = reader.read_u8()?;
            // If it's 0xFF, check and discard stuffed zero byte
            if c == 0xff {
                let c2 = reader.read_u8()?;
                if c2 != 0 {
                    // Oops, it's actually a marker indicating end of
                    // compressed data.  Better put it back for use later.
                    reader.seek(SeekFrom::Current(-2))?;
                    // There should be enough bits still left in the data
                    // segment; if so, just break out of the while loop.
                    if self.bits_left > nbits {
                        break;
                    }
                    // Uh-oh.  Corrupted data: stuff zeroes into the data
                    // stream, since this sometimes occurs when we are on the
                    // last show_bits(8) during decoding of the Huffman
                    // segment.
                    c = 0;
                }
            }
            // OK, load c into getBuffer
            self.buffer = (self.buffer << 8) | c as u32;
            self.bits_left += 8;
        }

        Ok(())
    }
}

#[cfg(test)]
mod test {
    use super::{BitReader, BITS_PER_LONG, MIN_GET_BITS};

    #[test]
    fn test_bit_reader() {
        // Note: don't use 0b1111_1111 for a byte as fill_bit_buffer
        // will use it to mark the end of the stream
        let bits = vec![
            0b1010_1010,
            0b0101_0101,
            0b1101_1011,
            0b0011_0011,
            0b1010_1010,
            0b0101_0101,
            0b1101_1011,
            0b0011_0011,
            0b1010_1010,
            0b0101_0101,
            0b1101_1011,
            0b0011_0011,
            0b1010_1010,
            0b0101_0101,
            0b1101_1011,
            0b0011_0011,
        ];

        let mut io = std::io::Cursor::new(bits);
        let mut br = BitReader::new();

        assert_eq!(BITS_PER_LONG, 32);
        assert_eq!(MIN_GET_BITS, 25);

        assert_eq!(br.buffer, 0);
        assert_eq!(br.bits_left, 0);
        assert_eq!(br.discard_bits(), 0);
        assert_eq!(br.buffer, 0);
        assert_eq!(br.bits_left, 0);

        assert!(matches!(br.show_bits8(&mut io), Ok(0b1010_1010)));
        assert_eq!(br.bits_left, 32);
        assert_eq!(br.buffer, 0b1010_1010_0101_0101_1101_1011_0011_0011);
        assert!(matches!(br.show_bits8(&mut io), Ok(0b1010_1010)));

        assert_eq!(br.discard_bits(), 4);
        // this doesn't clear the buffer
        assert_eq!(br.bits_left, 0);

        assert!(matches!(br.fill_bit_buffer(&mut io, 8), Ok(())));
        assert_eq!(br.bits_left, 32);
        assert_eq!(br.buffer, 0b1010_1010_0101_0101_1101_1011_0011_0011);
        assert!(matches!(br.show_bits8(&mut io), Ok(0b1010_1010)));

        assert!(matches!(br.get_bits(&mut io, 8), Ok(0b1010_1010)));
        assert_eq!(br.bits_left, 24);
        assert_eq!(br.buffer, 0b1010_1010_0101_0101_1101_1011_0011_0011);

        assert!(matches!(br.get_bit(&mut io), Ok(0)));
        assert_eq!(br.bits_left, 23);
        assert!(matches!(br.get_bit(&mut io), Ok(1)));
        assert_eq!(br.bits_left, 22);

        // XXX test fill_bit_buffer encountering 0xff
    }
}
