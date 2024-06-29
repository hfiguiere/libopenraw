// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - container.rs
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

//! Container traits. A RAW file is a bunch of containers.

use std::cell::RefMut;
use std::io::{Read, Seek, SeekFrom};

use byteorder::{BigEndian, ByteOrder, LittleEndian, NativeEndian, ReadBytesExt};

use crate::io::View;
use crate::metadata;
use crate::thumbnail::{Data, ThumbDesc, Thumbnail};
use crate::Type as RawType;
use crate::{Error, Result};

/// Endian of the container
#[derive(Clone, Copy, Debug, PartialEq)]
pub enum Endian {
    Unset,
    Big,
    Little,
}

impl Endian {
    /// Read an u16 from a reader based on the endian.
    pub(crate) fn read_u16_from<R>(&self, rdr: &mut R) -> std::io::Result<u16>
    where
        R: Read,
    {
        match *self {
            Endian::Big => rdr.read_u16::<BigEndian>(),
            Endian::Little => rdr.read_u16::<LittleEndian>(),
            _ => unreachable!("Endian undefined"),
        }
    }

    pub(crate) fn read_u16(&self, data: &[u8]) -> u16 {
        match *self {
            Endian::Big => BigEndian::read_u16(data),
            Endian::Little => LittleEndian::read_u16(data),
            _ => unreachable!("Endian undefined"),
        }
    }

    pub(crate) fn read_u32(&self, data: &[u8]) -> u32 {
        match *self {
            Endian::Big => BigEndian::read_u32(data),
            Endian::Little => LittleEndian::read_u32(data),
            _ => unreachable!("Endian undefined"),
        }
    }
}

/// Allow converting a `byteorder::ByteOrder` type to a
/// `Endian` value
///
/// ```no_compile
/// use byteorder::{BigEndian, LittleEndian};
///
/// let endian = LittleEndian::ENDIAN;
/// let endian = BigEndian::ENDIAN;
/// ```
pub(crate) trait EndianType: ByteOrder {
    const ENDIAN: Endian;
}

impl EndianType for LittleEndian {
    const ENDIAN: Endian = Endian::Little;
}

impl EndianType for BigEndian {
    const ENDIAN: Endian = Endian::Big;
}

/// Container abstract trait
pub trait RawContainer {
    /// Return the endian of the container
    fn endian(&self) -> Endian {
        Endian::Unset
    }

    /// Return an dir metadata iterator.
    fn dir_iterator(&self) -> metadata::Iterator {
        metadata::Iterator::default()
    }

    /// Return the rawtype for which this was created
    fn raw_type(&self) -> RawType;

    /// Make a thumbnail from the thumbdesc
    fn make_thumbnail(&self, desc: &ThumbDesc) -> Result<Thumbnail> {
        let data = match desc.data {
            Data::Bytes(ref b) => b.clone(),
            Data::Offset(ref offset) => {
                let mut view = self.borrow_view_mut();
                if offset.offset + offset.len > view.len() {
                    log::error!("Thumbmail too big");
                    return Err(Error::FormatError);
                }
                let mut data = uninit_vec!(offset.len as usize);
                view.seek(SeekFrom::Start(offset.offset))?;
                view.read_exact(data.as_mut_slice())?;
                data
            }
        };
        Ok(Thumbnail::with_data(
            desc.width,
            desc.height,
            desc.data_type,
            data,
        ))
    }

    /// Get the io::View for the container.
    fn borrow_view_mut(&self) -> RefMut<'_, View>;

    /// Load an 8bit buffer at `offset` and of `len` bytes.
    fn load_buffer8(&self, offset: u64, len: u64) -> Vec<u8> {
        let mut data = uninit_vec!(len as usize);

        let mut view = self.borrow_view_mut();
        if view.seek(SeekFrom::Start(offset)).is_err() {
            log::error!("load_buffer8: Seek failed");
        }
        if let Ok(n) = view.read(data.as_mut_slice()) {
            if n < len as usize {
                log::debug!("Short read {} < {}", n, len);
                data.resize(n, 0);
            }
        } else {
            log::error!("load_buffer8: read failed");
        }

        data
    }

    /// Load an 16 bit buffer at `offset` and of `len` bytes in the native endian.
    fn load_buffer16(&self, offset: u64, len: u64) -> Vec<u16> {
        let mut view = self.borrow_view_mut();
        load_buffer16_endian::<NativeEndian>(&mut view, offset, len)
    }

    /// Load an 16 bit buffer at `offset` and of `len` bytes, from Little Endian
    fn load_buffer16_le(&self, offset: u64, len: u64) -> Vec<u16> {
        let mut view = self.borrow_view_mut();
        load_buffer16_endian::<LittleEndian>(&mut view, offset, len)
    }

    /// Load an 16 bit buffer at `offset` and of `len` bytes, from Big Endian
    fn load_buffer16_be(&self, offset: u64, len: u64) -> Vec<u16> {
        let mut view = self.borrow_view_mut();
        load_buffer16_endian::<BigEndian>(&mut view, offset, len)
    }
}

/// Load an 16 bit buffer at `offset` and of `len` bytes following endian `E`.
fn load_buffer16_endian<E>(view: &mut View, offset: u64, len: u64) -> Vec<u16>
where
    E: ByteOrder,
{
    let mut data = uninit_vec!((len / 2) as usize);

    if let Err(err) = view.seek(SeekFrom::Start(offset)) {
        log::error!("load_buffer16: Seek failed: {err}");
    }

    if let Err(err) = view.read_u16_into::<E>(&mut data) {
        log::error!("load_buffer16: {err}");
    }

    data
}
