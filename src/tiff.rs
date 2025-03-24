// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - tiff.rs
 *
 * Copyright (C) 2022-2025 Hubert Figuière
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
mod iterator;

use std::convert::TryFrom;

use byteorder::{BigEndian, LittleEndian};
use num_enum::FromPrimitive;

use crate::container::Endian;
use crate::mosaic::Pattern;
use crate::TypeId;
pub(crate) use container::{Container, DirIterator, LoaderFixup};
pub(crate) use dir::Dir;
pub(crate) use entry::Entry;
pub(crate) use exif::TagType;
pub(crate) use iterator::Iterator;

#[repr(u32)]
#[derive(Clone, Copy, Debug, Default, PartialEq, FromPrimitive)]
/// TIFF (a RAW) compression values
pub enum Compression {
    /// Unknown - this value is never a valid one
    #[default]
    Unknown = 0,
    /// No compression
    None = 1,
    /// LZW
    Lzw = 5,
    /// JPEG compression
    Jpeg = 6,
    /// Losless JPEG compression (like in DNG)
    LJpeg = 7,
    /// Deflate (ZIP) DNG 1.4
    Deflate = 8,
    /// Found .GPR files.
    GoPro = 9,
    /// Sony ARW compression
    Arw = 32767,
    /// Nikon packed, also used by Epson ERF.
    NikonPack = 32769,
    /// Pentax packed (12be) - also seems to be PackBits in the
    /// TIFF specification.
    PentaxPack = 32773,
    /// Panasonic raw 1
    PanasonicRaw1 = 34316,
    /// Nikon quantized
    NikonQuantized = 34713,
    /// Panasonic raw 2
    PanasonicRaw2 = 34826,
    /// Panasonic raw 3
    PanasonicRaw3 = 34828,
    /// Panasonic raw 4
    PanasonicRaw4 = 34830,
    /// DNG 1.4 Lossy JPEG
    DngLossy = 34892,
    /// JPEG XL (DNG 1.7)
    JepgXl = 52546,
    /// What everybody seems to use
    Custom = 65535,
    // XXX figure out Olympus compression value
    // Olympus compression
    Olympus = 65536,
}

/// Type of IFD
#[derive(Clone, Copy, Debug, PartialEq)]
pub enum IfdType {
    /// Main IFD (see TIFF)
    Main,
    /// Raw specific IFD
    Raw,
    /// Exif IFD
    Exif,
    /// GPS IFD
    GpsInfo,
    /// MakerNote IFD
    MakerNote,
    /// Sub IFD
    SubIfd,
    /// Any other IFD
    Other,
}

/// Trait for Ifd
pub trait Ifd {
    /// Return the type if IFD
    fn ifd_type(&self) -> IfdType;

    fn endian(&self) -> Endian;

    /// The number of entries
    fn num_entries(&self) -> usize;

    /// Return the entry for the `tag`.
    fn entry(&self, tag: u16) -> Option<&Entry>;

    /// Get the value for entry, at index.
    fn entry_value<T>(&self, entry: &Entry, index: u32) -> Option<T>
    where
        T: exif::ExifValue,
    {
        match self.endian() {
            Endian::Big => entry.value_at_index::<T, BigEndian>(index),
            Endian::Little => entry.value_at_index::<T, LittleEndian>(index),
            _ => unreachable!("Endian unset"),
        }
    }

    /// Get value for tag.
    fn value<T>(&self, tag: u16) -> Option<T>
    where
        T: exif::ExifValue,
    {
        self.entry(tag).and_then(|e| self.entry_value::<T>(e, 0))
    }
}

pub(crate) type MakeToIdMap = std::collections::HashMap<&'static str, TypeId>;

/// Identify a files using the Exif data
pub(crate) fn identify_with_exif(container: &Container, map: &MakeToIdMap) -> Option<TypeId> {
    container.directory(0).and_then(|dir| {
        dir.entry(exif::EXIF_TAG_MODEL)
            // Files like Black Magic's DNG don't have a Model.
            .or_else(|| dir.entry(exif::DNG_TAG_UNIQUE_CAMERA_MODEL))
            .and_then(|e| e.string_value().and_then(|s| map.get(s.as_str()).copied()))
    })
}

fn convert_cfa_pattern(dir: &Dir, entry: &Entry) -> Option<Pattern> {
    let a = entry.value_array::<u8>(dir.endian())?;
    Pattern::try_from(a.as_slice()).ok()
}

fn convert_new_cfa_pattern(entry: &Entry) -> Option<Pattern> {
    if entry.count < 4 {
        return None;
    }
    let data = entry.data();

    let h = data[0];
    let v = data[1];
    if h != 2 && v != 2 {
        log::error!("Invalid CFA pattern size {}x{}", h, v);
        return None;
    }

    Pattern::try_from(&data[4..=7]).ok()
}

fn get_mosaic_info(dir: &Dir) -> Option<Pattern> {
    dir.entry(exif::EXIF_TAG_CFA_PATTERN)
        .and_then(|e| convert_cfa_pattern(dir, e))
        .or_else(|| {
            dir.entry(exif::EXIF_TAG_NEW_CFA_PATTERN)
                .and_then(convert_new_cfa_pattern)
        })
}
