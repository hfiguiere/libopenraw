// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - capi/ifd.rs
 *
 * Copyright (C) 2023 Hubert Figuière
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

use crate::tiff::{Ifd, IfdType};

use crate::or_unwrap;

/// And IFD directory.
pub type ORIfdDirRef = *const crate::tiff::Dir;

#[repr(C)]
#[allow(non_camel_case_types)]
#[allow(clippy::upper_case_acronyms)]
/// Type of IFD
pub enum or_ifd_dir_type {
    /// Generic
    OTHER = 0,
    /// Main (like in TIFF)
    MAIN = 1,
    /// Exif metadata
    EXIF = 2,
    /// MakerNote
    MNOTE = 3,
    /// RAW data
    RAW = 4,
    /// SubIFD
    SUBIFD = 5,
    /// GPSInfo
    GPSINFO = 6,
    /// INVALID value
    INVALID = 10000,
}

impl From<IfdType> for or_ifd_dir_type {
    fn from(t: IfdType) -> Self {
        use IfdType::*;
        match t {
            Main => Self::MAIN,
            Exif => Self::EXIF,
            MakerNote => Self::MNOTE,
            Raw => Self::RAW,
            SubIfd => Self::SUBIFD,
            GpsInfo => Self::GPSINFO,
            Other => Self::OTHER,
        }
    }
}

impl From<or_ifd_dir_type> for IfdType {
    fn from(t: or_ifd_dir_type) -> Self {
        use or_ifd_dir_type::*;
        match t {
            MAIN => Self::Main,
            EXIF => Self::Exif,
            MNOTE => Self::MakerNote,
            RAW => Self::Raw,
            SUBIFD => Self::SubIfd,
            GPSINFO => Self::GpsInfo,
            OTHER | INVALID => Self::Other,
        }
    }
}

#[no_mangle]
/// Count the tags in the IFD. Returns -1 if the dir in `nullptr`.
extern "C" fn or_ifd_count_tags(dir: ORIfdDirRef) -> i32 {
    or_unwrap!(dir, -1, dir.num_entries() as i32)
}

#[no_mangle]
/// Return the makenote id. A NUL terminated string.
extern "C" fn or_ifd_get_makernote_id(dir: ORIfdDirRef) -> *const std::ffi::c_char {
    or_unwrap!(dir, std::ptr::null(), {
        dir.id().as_ptr() as *const std::ffi::c_char
    })
}

#[no_mangle]
extern "C" fn or_ifd_get_type(dir: ORIfdDirRef) -> or_ifd_dir_type {
    or_unwrap!(dir, or_ifd_dir_type::INVALID, dir.type_.into())
}

#[no_mangle]
extern "C" fn or_ifd_get_tag_name(dir: ORIfdDirRef, tag: u32) -> *const std::ffi::c_char {
    or_unwrap!(dir, std::ptr::null(), {
        dir.tag_name(tag as u16)
            // XXX this is very very unsafe, it is not NUL terminated
            .as_ptr() as *const std::ffi::c_char
    })
}
