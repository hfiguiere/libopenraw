/*
 * libopenraw - decompress.rs
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

//! Decompression

mod ljpeg;
mod sliced_buffer;
mod tiled;

pub(crate) use ljpeg::LJpeg;
pub(crate) use tiled::TiledLJpeg;

use std::io::{Read, Seek, SeekFrom};

use crate::container::GenericContainer;
use crate::tiff;
use crate::{Error, Result};

/// Unpack n-bits into 16-bits values
/// out_len is the number of expected 16-bits pixel.
/// For performance `out_data` should have reserved size
/// Return the number of elements written.
fn unpack_bento16(input: &[u8], n: u16, out_len: usize, out_data: &mut Vec<u16>) -> Result<usize> {
    if n > 16 {
        return Err(Error::InvalidParam);
    }
    let mut reader = bitreader::BitReader::new(input);
    let mut written = 0_usize;
    for _ in 0..out_len {
        let t = reader.read_u16(n as u8)?;
        out_data.push(t);
        written += 1;
    }
    Ok(written)
}

/// Unpack 12-bits into 16-bits values
/// For performance `out_data` should have reserved size
/// Return the number of elements written.
pub(crate) fn unpack_be12to16(
    input: &[u8],
    out_data: &mut Vec<u16>,
    compression: tiff::Compression,
) -> Result<usize> {
    let pad = if compression == tiff::Compression::NikonPack {
        1_usize
    } else {
        0_usize
    };
    let n = input.len() / (15 + pad);
    let rest = input.len() % (15 + pad);
    let mut src = 0_usize; // index in source

    if pad != 0 && (input.len() % 16) != 0 {
        log::error!("be12to16 incorrect padding for {:?}.", compression);
        return Err(Error::Decompression);
    }
    if (rest % 3) != 0 {
        log::error!("be12to16 incorrect rest for {:?}.", compression);
        return Err(Error::Decompression);
    }

    let mut written = 0_usize;
    for i in 0..=n {
        let m = if i == n { rest / 3 } else { 5 };
        // XXX check overflow
        for _ in 0..m {
            let mut t: u32 = input[src] as u32;
            src += 1;
            t <<= 8;

            t |= input[src] as u32;
            src += 1;
            t <<= 8;

            t |= input[src] as u32;
            src += 1;

            out_data.push(((t & (0xfff << 12)) >> 12) as u16);
            out_data.push((t & 0xfff) as u16);
            written += 2;
        }
        src += pad;
    }
    Ok(written)
}

/// Unpack data at `offset` into a 16-bits buffer.
pub(crate) fn unpack(
    container: &dyn GenericContainer,
    width: u32,
    height: u32,
    bpc: u16,
    compression: tiff::Compression,
    offset: u64,
    byte_len: usize,
) -> Result<Vec<u16>> {
    log::debug!(
        "Unpack {} bytes {} x {} - compression {:?}",
        byte_len,
        width,
        height,
        compression
    );
    let block_size: usize = match bpc {
        10 => (width / 4 * 5) as usize,
        12 => {
            if compression == tiff::Compression::NikonPack {
                ((width / 2 * 3) + width / 10) as usize
            } else {
                (width / 2 * 3) as usize
            }
        }
        14 => (width / 4 * 7) as usize,
        _ => {
            log::warn!("Invalid BPC {}", bpc);
            return Err(Error::InvalidFormat);
        }
    };
    log::debug!("Block size = {}", block_size);
    let mut block = Vec::new();
    block.resize(block_size, 0);
    let out_size = width as usize * height as usize;
    let mut out_data = Vec::with_capacity(out_size);
    let mut fetched = 0_usize;
    let mut written = 0_usize;

    let mut view = container.borrow_view_mut();
    view.seek(SeekFrom::Start(offset))?;

    let byte_len = std::cmp::min(byte_len, block_size * height as usize);
    while fetched < byte_len {
        view.read_exact(block.as_mut_slice())?;
        fetched += block.len();
        match bpc {
            n @ 10 | n @ 14 => written += unpack_bento16(&block, n, width as usize, &mut out_data)?,
            12 => written += unpack_be12to16(&block, &mut out_data, compression)?,
            _ => unreachable!(),
        }
    }
    log::debug!("Unpacked {} pixels", written);

    Ok(out_data)
}

#[cfg(test)]
mod test {

    use super::unpack_be12to16;
    use super::unpack_bento16;
    use crate::tiff;

    #[test]
    fn test_unpack() {
        let packed: [u8; 32] = [
            0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x78, 0x90, 0xAB,
            0xCD, 0x00, 0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x78,
            0x90, 0xAB, 0xCD, 0x00,
        ];

        let mut unpacked: Vec<u16> = Vec::with_capacity(20);

        let result = unpack_be12to16(
            packed.as_slice(),
            &mut unpacked,
            tiff::Compression::NikonPack,
        );

        assert_eq!(result, Ok(20));
        for i in 0..2 {
            assert_eq!(unpacked[10 * i + 0], 0x0123);
            assert_eq!(unpacked[10 * i + 1], 0x0456);
            assert_eq!(unpacked[10 * i + 2], 0x0789);
            assert_eq!(unpacked[10 * i + 3], 0x00ab);
            assert_eq!(unpacked[10 * i + 4], 0x0cde);
            assert_eq!(unpacked[10 * i + 5], 0x0f12);
            assert_eq!(unpacked[10 * i + 6], 0x0345);
            assert_eq!(unpacked[10 * i + 7], 0x0678);
            assert_eq!(unpacked[10 * i + 8], 0x090a);
            assert_eq!(unpacked[10 * i + 9], 0x0bcd);
        }
    }

    #[test]
    fn test_unpack2() {
        let packed: [u8; 3] = [0x12, 0x34, 0x56];

        let mut unpacked: Vec<u16> = Vec::with_capacity(2);

        let result = unpack_be12to16(packed.as_slice(), &mut unpacked, tiff::Compression::None);

        assert_eq!(result, Ok(2));
        assert_eq!(unpacked[0], 0x0123);
        assert_eq!(unpacked[1], 0x0456);
    }

    #[test]
    fn test_unpack14() {
        let buf: [u8; 7] = [
            0b1111_1111,
            0b1111_1100,
            0b0000_0000,
            0b0000_1111,
            0b1111_1111,
            0b1100_0000,
            0b0000_0000,
        ];

        let mut unpacked: Vec<u16> = Vec::with_capacity(4);

        let result = unpack_bento16(buf.as_slice(), 14, 4, &mut unpacked);
        assert_eq!(result, Ok(4));
        assert_eq!(unpacked[0], 0b0011_1111_1111_1111);
        assert_eq!(unpacked[1], 0b0000_0000_0000_0000);
        assert_eq!(unpacked[2], 0b0011_1111_1111_1111);
        assert_eq!(unpacked[3], 0b0000_0000_0000_0000);
    }
}
