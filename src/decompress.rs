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

pub(crate) use ljpeg::LJpeg;

use std::io::{Read, Seek, SeekFrom};

use crate::container::GenericContainer;
use crate::tiff;
use crate::{Error, Result};

/// Unpack 12-bits into 16-bits values
/// For performance `out_data` should have reserved size
/// Return the number of elements written.
fn unpack_be12to16(
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

    if pad == 1 && (input.len() % 16) != 0 {
        log::error!("be12to16 incorrect padding.");
        return Err(Error::Decompression);
    }
    if (rest % 3) != 0 {
        log::error!("be12to16 incorrect rest.");
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
/// Currently only support 12-bits input.
pub(crate) fn unpack(
    container: &dyn GenericContainer,
    width: u32,
    height: u32,
    bpc: u16,
    compression: tiff::Compression,
    offset: u64,
    byte_len: usize,
) -> Result<Vec<u16>> {
    // XXX handle other BPC like 14.
    if bpc != 12 {
        log::warn!("Invalid BPC {}", bpc);
        return Err(Error::InvalidFormat);
    }
    let block_size: usize = if compression == tiff::Compression::NikonPack {
        ((width / 2 * 3) + width / 10) as usize
    } else {
        (width / 2 * 3) as usize
    };
    log::debug!("Block size = {}", block_size);
    let mut block = Vec::new();
    block.resize(block_size, 0);
    let out_size = (width * height * 2) as usize;
    let mut out_data = Vec::new();
    out_data.reserve(out_size);
    let mut fetched = 0_usize;

    let mut view = container.borrow_view_mut();
    view.seek(SeekFrom::Start(offset))?;
    while fetched < byte_len {
        view.read_exact(block.as_mut_slice())?;
        fetched += block.len();
        unpack_be12to16(&block, &mut out_data, compression)?;
    }

    Ok(out_data)
}

#[cfg(test)]
mod test {

    use super::unpack_be12to16;
    use crate::tiff;

    #[test]
    fn test_unpack() {
        let packed: [u8; 32] = [
            0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x78, 0x90, 0xAB,
            0xCD, 0x00, 0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x78,
            0x90, 0xAB, 0xCD, 0x00,
        ];

        let mut unpacked: Vec<u16> = Vec::new();

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

        let mut unpacked: Vec<u16> = Vec::new();

        let result = unpack_be12to16(packed.as_slice(), &mut unpacked, tiff::Compression::None);

        assert_eq!(result, Ok(2));
        assert_eq!(unpacked[0], 0x0123);
        assert_eq!(unpacked[1], 0x0456);
    }
}