// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - decompress.rs
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

//! Decompression

pub(crate) mod bit_reader;
mod ljpeg;
mod sliced_buffer;
mod tiled;

pub use ljpeg::LJpeg;
pub(crate) use tiled::TiledLJpeg;

use std::io::{Read, Seek, SeekFrom};

use crate::container::{Endian, RawContainer};
use crate::tiff;
use crate::{Error, Result};
use bit_reader::BitReader;

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

/// Unpack 14 bits using a custom BitReader.
/// Returns the number of values read.
pub(crate) fn unpack_14to16<R>(
    reader: &mut R,
    out_len: usize,
    out_data: &mut Vec<u16>,
) -> Result<usize>
where
    R: BitReader,
{
    let mut written = 0_usize;
    while written < out_len {
        let t = reader.get_bits(14)?;
        out_data.push(t);
        written += 1;
    }
    Ok(written)
}

/// Unpack 12-bits into 16-bits values BigEndian
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
        return Err(Error::Decompression(format!(
            "be12to16 incorrect padding for {compression:?}.",
        )));
    }
    if (rest % 3) != 0 {
        log::error!("be12to16 incorrect rest for {:?}.", compression);
        return Err(Error::Decompression(format!(
            "be12to16 incorrect rest for {compression:?}.",
        )));
    }

    let mut written = 0_usize;
    for i in 0..=n {
        let m = if i == n { rest / 3 } else { 5 };
        // XXX check overflow
        for _ in 0..m {
            let i0 = input[src] as u16;
            src += 1;
            let i1 = input[src] as u16;
            src += 1;
            let i2 = input[src] as u16;
            src += 1;

            let o0 = i0 << 4 | i1 >> 4;
            let o1 = (i1 & 0xf) << 8 | i2;

            out_data.push(o0);
            out_data.push(o1);
            written += 2;
        }
        src += pad;
    }
    Ok(written)
}

/// Unpack 12-bits into 16-bits values LittleEndia
/// For performance `out_data` should have reserved size
/// Return the number of elements written.
pub(crate) fn unpack_le12to16(
    input: &[u8],
    out_data: &mut Vec<u16>,
    compression: tiff::Compression,
) -> Result<usize> {
    let pad = if compression == tiff::Compression::Olympus
        || compression == tiff::Compression::PanasonicRaw1
    {
        1_usize
    } else {
        0_usize
    };
    let n = input.len() / (15 + pad);
    let rest = input.len() % (15 + pad);
    let mut src = 0_usize; // index in source

    if pad != 0 && (input.len() % 16) != 0 {
        log::error!("le12to16 incorrect padding for {:?}.", compression);
        return Err(Error::Decompression(format!(
            "le12to16 incorrect padding for {compression:?}.",
        )));
    }
    if (rest % 3) != 0 {
        log::error!("le12to16 incorrect rest for {:?}.", compression);
        return Err(Error::Decompression(format!(
            "le12to16 incorrect rest for {compression:?}.",
        )));
    }

    let mut written = 0_usize;
    for i in 0..=n {
        let m = if i == n { rest / 3 } else { 5 };
        // XXX check overflow
        for _ in 0..m {
            let b1 = input[src] as u16;
            src += 1;

            let b2 = input[src] as u16;
            src += 1;

            let b3 = input[src] as u16;
            src += 1;

            out_data.push(((b2 & 0xf) << 8) | b1);
            out_data.push((b3 << 4) | (b2 >> 4));
            written += 2;
        }
        src += pad;
    }
    Ok(written)
}

/// Unpack data at `offset` into a 16-bits buffer.
pub(crate) fn unpack(
    container: &dyn RawContainer,
    width: u32,
    height: u32,
    bpc: u16,
    compression: tiff::Compression,
    offset: u64,
    byte_len: usize,
) -> Result<Vec<u16>> {
    let mut view = container.borrow_view_mut();
    view.seek(SeekFrom::Start(offset))?;

    unpack_from_reader(
        &mut *view,
        width,
        height,
        bpc,
        compression,
        byte_len,
        container.endian(),
    )
}

/// Unpack from a reader into a 16-bits buffer. `endian` is used when
/// compression is `None`.
pub(crate) fn unpack_from_reader(
    reader: &mut dyn Read,
    width: u32,
    height: u32,
    bpc: u16,
    compression: tiff::Compression,
    byte_len: usize,
    endian: Endian,
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
            if compression == tiff::Compression::NikonPack
                || compression == tiff::Compression::Olympus
                || compression == tiff::Compression::PanasonicRaw1
            {
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
    let mut block = vec![0; block_size];
    let out_size = width as usize * height as usize;
    let mut out_data = Vec::with_capacity(out_size);
    let mut fetched = 0_usize;
    let mut written = 0_usize;

    let byte_len = std::cmp::min(byte_len, block_size * height as usize);
    while fetched < byte_len {
        reader.read_exact(block.as_mut_slice())?;
        fetched += block.len();
        match bpc {
            n @ 10 | n @ 14 => written += unpack_bento16(&block, n, width as usize, &mut out_data)?,
            12 => {
                written += match compression {
                    tiff::Compression::NikonPack | tiff::Compression::PentaxPack => {
                        unpack_be12to16(&block, &mut out_data, compression)?
                    }
                    tiff::Compression::None => match endian {
                        Endian::Little => unpack_le12to16(&block, &mut out_data, compression)?,
                        Endian::Big => unpack_be12to16(&block, &mut out_data, compression)?,
                        _ => unreachable!(),
                    },
                    tiff::Compression::Olympus | tiff::Compression::PanasonicRaw1 => {
                        unpack_le12to16(&block, &mut out_data, compression)?
                    }
                    _ => unreachable!(),
                }
            }
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

        assert!(matches!(result, Ok(20)));
        for i in 0..2 {
            assert_eq!(unpacked[10 * i], 0x0123);
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

        assert!(matches!(result, Ok(2)));
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
        assert!(matches!(result, Ok(4)));
        assert_eq!(unpacked[0], 0b0011_1111_1111_1111);
        assert_eq!(unpacked[1], 0b0000_0000_0000_0000);
        assert_eq!(unpacked[2], 0b0011_1111_1111_1111);
        assert_eq!(unpacked[3], 0b0000_0000_0000_0000);
    }
}
