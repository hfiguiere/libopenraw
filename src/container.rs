/*
 * libopenraw - container.rs
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

//! Container traits. A RAW file is a bunch of containers.

use std::cell::RefMut;
use std::io::{Read, Seek, SeekFrom};

use byteorder::{BigEndian, ByteOrder, LittleEndian};

use crate::io::View;
use crate::thumbnail::{Data, ThumbDesc, Thumbnail};
use crate::utils;
use crate::Result;

/// Endian of the container
#[derive(Clone, Copy, Debug)]
pub enum Endian {
    Unset,
    Big,
    Little,
}

/// Allow converting a `byteorder::ByteOrder` type to a
/// `Endian` value
///
/// ```no_compile
/// use byteorder::{BigEndian, LittleEndian};
///
/// let endian = LittleEndian::endian();
/// let endian = BigEndian::endian();
/// ```
pub(crate) trait EndianType: ByteOrder {
    fn endian() -> Endian;
}

impl EndianType for LittleEndian {
    fn endian() -> Endian {
        Endian::Little
    }
}

impl EndianType for BigEndian {
    fn endian() -> Endian {
        Endian::Big
    }
}

/// Container abstract trait
pub(crate) trait GenericContainer {
    /// Return the endian of the container
    fn endian(&self) -> Endian {
        Endian::Unset
    }

    /// Make a thumbnail from the thumbdesc
    fn make_thumbnail(&self, desc: &ThumbDesc) -> Result<Thumbnail> {
        let data = match desc.data {
            Data::Bytes(ref b) => b.clone(),
            Data::Offset(ref offset) => {
                let mut view = self.borrow_view_mut();
                let mut data = Vec::new();
                data.resize(offset.len as usize, 0);
                view.seek(SeekFrom::Start(offset.offset))?;
                view.read_exact(data.as_mut_slice())?;
                data
            }
        };
        Ok(Thumbnail::new(
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
        let mut data = Vec::new();

        let mut view = self.borrow_view_mut();
        data.resize(len as usize, 0);
        if view.seek(SeekFrom::Start(offset)).is_err() {
            log::error!("load_buffer8: Seek failed");
        }
        if view.read_exact(data.as_mut_slice()).is_err() {
            log::error!("load_buffer8: read failed");
        }

        data
    }

    /// Load an 16 bit buffer at `offset` and of `len` bytes.
    fn load_buffer16(&self, offset: u64, len: u64) -> Vec<u16> {
        let mut data = Vec::new();

        let mut view = self.borrow_view_mut();
        data.resize((len / 2) as usize, 0);
        if view.seek(SeekFrom::Start(offset)).is_err() {
            log::error!("load_buffer16: Seek failed");
        }
        // XXX do we need to deal with the endian????
        let slice = utils::to_u8_slice_mut(&mut data);
        if view.read_exact(slice).is_err() {
            log::error!("load_buffer18: read failed");
        }

        data
    }
}
