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

use byteorder::{BigEndian, ByteOrder, LittleEndian, ReadBytesExt};

use crate::{Error, Result};

const BITS_PER_LONG: u8 = 8 * std::mem::size_of::<i32>() as u8;
const MIN_GET_BITS: u8 = BITS_PER_LONG - 7; // max value for long get_buffer

// BMASK[n] is mask for n rightmost bits
const BMASK: [u16; 17] = [
    0x0000, 0x0001, 0x0003, 0x0007, 0x000F, 0x001F, 0x003F, 0x007F, 0x00FF, 0x01FF, 0x03FF, 0x07FF,
    0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF,
];

/// BitReader trait to define a common interface.
pub(crate) trait BitReader {
    fn peek(&mut self, nbits: u8) -> Result<u16>;
    fn consume(&mut self, nbits: u8);

    #[inline]
    fn get_bits(&mut self, nbits: u8) -> Result<u16> {
        if nbits >= 17 {
            return Err(Error::Decompression(format!(
                "BitReader: Tried to request {nbits} bits (max 16)."
            )));
        }
        let value = self.peek(nbits);
        self.consume(nbits);

        value
    }

    #[inline]
    /// Consume all leading zeros.
    fn consume_zerobits(&mut self) -> u32 {
        const NUM_BITS: u32 = u16::BITS - 1;
        let mut count = 0;
        loop {
            // u16::leading_zeros require at least one bit set.
            let bits = (self.peek(NUM_BITS as u8).unwrap() << 1) | 0x1;
            let n = bits.leading_zeros();
            self.consume(n as u8);
            count += n;
            // less than a whole batch of contiguous zero, it's the end
            if n != NUM_BITS {
                break;
            }
        }
        count
    }
}

/// Load bits from Big Endian 32bits.
// XXX make this a generic on ByteOrder
pub(crate) struct BitReaderBe32<R> {
    reader: std::io::BufReader<R>,
    bits_left: u8,
    bits: u64,
}

impl<R: std::io::Read> BitReaderBe32<R> {
    pub fn new(reader: R) -> BitReaderBe32<R> {
        BitReaderBe32 {
            reader: std::io::BufReader::new(reader),
            bits_left: 0,
            bits: 0,
        }
    }
}

impl<R: std::io::Read> BitReader for BitReaderBe32<R> {
    fn peek(&mut self, nbits: u8) -> Result<u16> {
        if self.bits_left < nbits {
            let b = self.reader.read_u32::<BigEndian>()? as u64;
            self.bits = (self.bits << 32) | b;
            self.bits_left += 32;
        }

        Ok(((self.bits >> (self.bits_left - nbits)) & ((1_u32 << nbits) - 1) as u64) as u16)
    }

    fn consume(&mut self, nbits: u8) {
        self.bits_left -= nbits;
        self.bits &= (1 << self.bits_left) - 1;
    }
}

/// Load bits from Little Endian 32bits.
pub(crate) struct BitReaderLe32<R> {
    reader: std::io::BufReader<R>,
    bits_left: u8,
    bits: u64,
}

impl<R: std::io::Read> BitReaderLe32<R> {
    pub fn new(reader: R) -> BitReaderLe32<R> {
        BitReaderLe32 {
            reader: std::io::BufReader::new(reader),
            bits_left: 0,
            bits: 0,
        }
    }
}

impl<R: std::io::Read + std::io::Seek> BitReader for BitReaderLe32<R> {
    fn peek(&mut self, nbits: u8) -> Result<u16> {
        if self.bits_left < nbits {
            let b = self.reader.read_u32::<LittleEndian>()? as u64;
            self.bits = (self.bits << 32) | b;
            self.bits_left += 32;
        }

        Ok(((self.bits >> (self.bits_left - nbits)) & ((1_u32 << nbits) - 1) as u64) as u16)
    }

    fn consume(&mut self, nbits: u8) {
        // XXX should we use saturating_sub() instead?
        self.bits_left -= std::cmp::min(nbits, self.bits_left);
    }
}

/// JPEG bit reader. It allows also reading u8 and u16
/// and handle markers in he stream.
pub(crate) struct LJpegBitReader<'a> {
    buffer: &'a [u8],
    pos: usize,
    bits_left: u8,
    bits: u32,
}

impl BitReader for LJpegBitReader<'_> {
    fn peek(&mut self, nbits: u8) -> Result<u16> {
        if self.bits_left < nbits {
            self.fill_bit_buffer(nbits)?;
        }

        Ok(((self.bits >> (self.bits_left - nbits)) & BMASK[nbits as usize] as u32) as u16)
    }

    fn consume(&mut self, nbits: u8) {
        self.bits_left -= nbits;
    }
}

impl<'a> LJpegBitReader<'a> {
    pub fn new(buffer: &'a [u8]) -> LJpegBitReader<'a> {
        LJpegBitReader {
            buffer,
            pos: 0,
            bits_left: 0,
            bits: 0,
        }
    }

    /// Read a byte out of the buffer.
    pub(crate) fn read_u8(&mut self) -> u8 {
        let b = self.buffer[self.pos];
        self.pos += 1;
        b
    }

    /// Read an u16 from the bit reader. JPEG is always BigEndian.
    pub(crate) fn read_u16(&mut self) -> u16 {
        let b = BigEndian::read_u16(&self.buffer[self.pos..]);
        self.pos += 2;
        b
    }

    /// Skip `seek` bytes.
    pub(crate) fn skip(&mut self, seek: usize) {
        self.pos += seek;
    }

    /// Discard current bits.
    #[inline]
    pub(crate) fn discard(&mut self) {
        self.bits_left = 0;
    }

    // Load up the bit buffer with at least nbits
    // Process any stuffed bytes at this time.
    fn fill_bit_buffer(&mut self, nbits: u8) -> Result<()> {
        while self.bits_left < MIN_GET_BITS {
            let mut c = self.read_u8();
            // If it's 0xFF, check and discard stuffed zero byte
            if c == 0xff {
                let c2 = self.read_u8();
                if c2 != 0 {
                    // Oops, it's actually a marker indicating end of
                    // compressed data.  Better put it back for use later.
                    self.pos -= 2;
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
            self.bits = (self.bits << 8) | c as u32;
            self.bits_left += 8;
        }

        Ok(())
    }
}

#[cfg(test)]
mod test {
    use super::{BitReader, BitReaderBe32, LJpegBitReader, BITS_PER_LONG, MIN_GET_BITS};

    #[test]
    fn test_ljpeg_bit_reader() {
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

        let mut br = LJpegBitReader::new(&bits);

        assert_eq!(BITS_PER_LONG, 32);
        assert_eq!(MIN_GET_BITS, 25);

        assert_eq!(br.bits, 0);
        assert_eq!(br.bits_left, 0);
        br.discard();
        assert_eq!(br.bits, 0);
        assert_eq!(br.bits_left, 0);

        assert!(matches!(br.peek(8), Ok(0b1010_1010)));
        assert_eq!(br.bits_left, 32);
        assert_eq!(br.bits, 0b1010_1010_0101_0101_1101_1011_0011_0011);
        assert!(matches!(br.peek(8), Ok(0b1010_1010)));

        br.discard();
        // this doesn't clear the buffer
        assert_eq!(br.bits_left, 0);

        assert!(matches!(br.fill_bit_buffer(8), Ok(())));
        assert_eq!(br.bits_left, 32);
        assert_eq!(br.bits, 0b1010_1010_0101_0101_1101_1011_0011_0011);
        assert!(matches!(br.peek(8), Ok(0b1010_1010)));

        assert!(matches!(br.get_bits(8), Ok(0b1010_1010)));
        assert_eq!(br.bits_left, 24);
        assert_eq!(br.bits, 0b1010_1010_0101_0101_1101_1011_0011_0011);

        assert!(matches!(br.get_bits(1), Ok(0)));
        assert_eq!(br.bits_left, 23);
        assert!(matches!(br.get_bits(1), Ok(1)));
        assert_eq!(br.bits_left, 22);

        // XXX test fill_bit_buffer encountering 0xff
    }

    #[test]
    fn test_zerobits() {
        let src = vec![0_u8, 0, 0, 1, 0, 0, 0, 0];

        let cursor = std::io::Cursor::new(&src);
        let mut reader = BitReaderBe32::new(cursor);

        let zeros = reader.consume_zerobits();
        assert_eq!(zeros, 31);
    }
}
