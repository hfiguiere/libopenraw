/*
 * libopenraw - ifd.rs
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

//! TIFF format (Image File Directories)

mod container;
mod dir;
mod entry;
pub mod exif;

use byteorder::{BigEndian, LittleEndian};

use crate::container::Endian;
pub(crate) use container::Container;
pub(crate) use dir::Dir;
pub(crate) use entry::Entry;

/// Type of IFD
#[derive(Clone, Copy)]
pub enum Type {
    /// Main IFD (see TIFF)
    Main,
    /// CFA specific IFD
    Cfa,
    /// Exif IFD
    Exif,
    /// MakerNote IFD
    MakerNote,
    /// Any other IFD
    Other,
}

/// Trait for Ifd
pub trait Ifd {
    /// Return the type if IFD
    fn ifd_type(&self) -> Type;

    fn endian(&self) -> Endian;

    /// The number of entries
    fn num_entries(&self) -> usize;

    /// Return the entry for the `tag`.
    fn entry(&self, tag: u16) -> Option<&Entry>;

    /// Get value for tag.
    fn value<T>(&self, tag: u16) -> Option<T>
    where
        T: exif::ExifValue,
    {
        self.entry(tag).and_then(|e| match self.endian() {
            Endian::Big => e.value::<T, BigEndian>(),
            Endian::Little => e.value::<T, LittleEndian>(),
            _ => unreachable!("Endian unset"),
        })
    }
}
