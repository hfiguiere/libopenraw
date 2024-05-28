// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - capi/mod.rs
 *
 * Copyright (C) 2022-2023 Hubert Figuière
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

//! C API for libopenraw
//!
//! This module will implement all the C API for libopenraw
//! It will only be built if the feature `capi` is enabled.
//!
//! By default the autotools build system will generate libopenraw.so
//! and the associated pkg-config files that can be used like in the
//! C++ version.
//!
//! To preserve the consistency of the API, some of the symbols in this
//! module don't follow Rust conventions.
//!
//! The goal is to expose all the functionality of the library.

#![warn(missing_docs)]

#[cfg(feature = "capi")]
mod bitmap;
#[cfg(feature = "capi")]
mod ifd;
#[cfg(feature = "capi")]
mod iterator;
#[cfg(feature = "capi")]
mod metavalue;
#[cfg(feature = "capi")]
mod mime;
#[cfg(feature = "capi")]
mod mosaic;
#[cfg(feature = "capi")]
mod rawdata;
#[cfg(feature = "capi")]
mod rawfile;
#[cfg(feature = "capi")]
mod thumbnail;

#[cfg(feature = "capi")]
use std::ffi::{CStr, CString, OsStr};
// This is not portable to Windows
#[cfg(feature = "capi")]
use std::os::unix::ffi::OsStrExt;

#[cfg(feature = "capi")]
use libc::c_char;
#[cfg(feature = "capi")]
use once_cell::sync::Lazy;

use crate::mosaic::Pattern;
#[cfg(feature = "capi")]
use crate::rawfile_from_file;
use crate::{DataType, Error};
#[cfg(feature = "capi")]
pub use bitmap::ORBitmapDataRef;
#[cfg(feature = "capi")]
pub use ifd::{or_ifd_dir_type, ORIfdDirRef};
#[cfg(feature = "capi")]
pub use iterator::ORMetadataIteratorRef;
#[cfg(feature = "capi")]
pub use metavalue::{ORConstMetaValueRef, ORMetaValueRef};
#[cfg(feature = "capi")]
pub use mosaic::ORMosaicInfoRef;
#[cfg(feature = "capi")]
pub use rawdata::ORRawDataRef;
#[cfg(feature = "capi")]
pub use thumbnail::ORThumbnailRef;

#[macro_export]
/// unwrap a C type.
macro_rules! or_unwrap {
    ($wrap:ident, $ret:expr, $code:expr) => {
        if !$wrap.is_null() {
            let $wrap = unsafe { &(*$wrap) };
            $code
        } else {
            $ret
        }
    };
}

#[macro_export]
/// unwrap a C type.
macro_rules! or_unwrap_mut {
    ($wrap:ident, $ret:expr, $code:expr) => {
        if !$wrap.is_null() {
            let $wrap = unsafe { &mut (*$wrap) };
            $code
        } else {
            $ret
        }
    };
}

#[repr(C)]
#[allow(non_camel_case_types)]
#[allow(clippy::upper_case_acronyms)]
#[derive(Copy, Clone, Debug, Eq, PartialEq)]
/// Error types.
pub enum or_error {
    /// No error.
    NONE = 0,
    /// The buffer is too small
    BUF_TOO_SMALL = 1,
    /// The expected pointer / ref is invalid.
    NOT_AREF = 2,
    /// Not found.
    NOT_FOUND = 5,
    /// Invalid parameter
    INVALID_PARAM = 6,
    /// Invalid format
    INVALID_FORMAT = 7,
    /// An unknown error.
    UNKNOWN = 42,
}

impl From<Error> for or_error {
    fn from(err: Error) -> or_error {
        match err {
            Error::NotFound => or_error::NOT_FOUND,
            _ => or_error::UNKNOWN,
        }
    }
}

impl From<or_error> for Error {
    fn from(err: or_error) -> Error {
        match err {
            or_error::NONE => Error::None,
            or_error::BUF_TOO_SMALL => Error::BufferTooSmall,
            or_error::NOT_AREF => Error::InvalidAddress,
            or_error::NOT_FOUND => Error::NotFound,
            or_error::INVALID_PARAM => Error::InvalidParam,
            or_error::INVALID_FORMAT => Error::InvalidFormat,
            or_error::UNKNOWN => Error::Unknown,
        }
    }
}

#[repr(C)]
#[allow(non_camel_case_types)]
#[allow(dead_code)]
#[allow(clippy::upper_case_acronyms)]
/// Data type.
pub enum or_data_type {
    /// No data type.
    NONE = 0,
    /// 8bit per channel RGB pixmap
    PIXMAP_8RGB = 1,
    /// 16bit per channel RGB pixmap
    PIXMAP_16RGB = 2,
    /// JPEG data
    JPEG = 3,
    /// TIFF container
    TIFF = 4,
    /// PNG container
    PNG = 5,
    /// RAW container
    RAW = 6,
    /// Compressed RAW container
    COMPRESSED_RAW = 7,

    /// Unknown data type. Unlikely.
    UNKNOWN = 100,
}

impl From<DataType> for or_data_type {
    fn from(dt: DataType) -> Self {
        match dt {
            DataType::Jpeg => Self::JPEG,
            DataType::PixmapRgb8 => Self::PIXMAP_8RGB,
            DataType::PixmapRgb16 => Self::PIXMAP_16RGB,
            DataType::CompressedRaw => Self::COMPRESSED_RAW,
            DataType::Raw => Self::RAW,
            DataType::Unknown => Self::UNKNOWN,
        }
    }
}

#[repr(C)]
#[allow(non_camel_case_types)]
#[allow(clippy::upper_case_acronyms)]
/// The CFA pattern type.
pub enum or_cfa_pattern {
    /// Invalid value
    NONE = 0,
    /// Non RGB 2x2 CFA
    NON_RGB22 = 1,
    /// RGGB 2x2 pattern
    RGGB = 2,
    /// GBRG 2x2 pattern
    GBRG = 3,
    /// BGGR 2x2 pattern
    BGGR = 4,
    /// GRBG 2x2 pattern
    GRBG = 5,
}

impl From<&Pattern> for or_cfa_pattern {
    fn from(pattern: &Pattern) -> Self {
        match *pattern {
            Pattern::Empty => Self::NONE,
            Pattern::NonRgb22(_) => Self::NON_RGB22,
            Pattern::Rggb => Self::RGGB,
            Pattern::Gbrg => Self::GBRG,
            Pattern::Bggr => Self::BGGR,
            Pattern::Grbg => Self::GRBG,
        }
    }
}

#[repr(C)]
#[allow(non_camel_case_types)]
#[allow(dead_code)]
#[allow(clippy::upper_case_acronyms)]
/// Options.
pub(crate) enum or_options {
    /// No option.
    NONE = 0,
    /// Don't decompress
    DONT_DECOMPRESS = 1,
}

#[cfg(feature = "capi")]
#[allow(non_camel_case_types)]
type or_colour_matrix_origin = crate::colour::MatrixOrigin;

#[repr(C)]
#[allow(non_camel_case_types)]
#[allow(dead_code)]
#[allow(clippy::upper_case_acronyms)]
/// Debug level
pub(crate) enum debug_level {
    /// Error.
    ERROR = 0,
    /// Warning.
    WARNING = 1,
    /// Notice.
    NOTICE = 2,
    /// Debug.
    DEBUG = 3,
    /// Debug 2.
    DEBUG2 = 4,
}

impl From<debug_level> for log::LevelFilter {
    fn from(lvl: debug_level) -> log::LevelFilter {
        use debug_level::*;
        use log::LevelFilter;
        match lvl {
            ERROR => LevelFilter::Error,
            WARNING => LevelFilter::Warn,
            NOTICE => LevelFilter::Info,
            DEBUG => LevelFilter::Debug,
            DEBUG2 => LevelFilter::Trace,
        }
    }
}

#[cfg(feature = "capi")]
#[no_mangle]
/// Set the debug level.
///
/// This will enable the verbosity of the library. Will display
/// message for level less then or equal to `lvl`.
extern "C" fn or_debug_set_level(lvl: debug_level) {
    let level: log::LevelFilter = lvl.into();

    log::set_max_level(level);
}

#[cfg(feature = "capi")]
#[no_mangle]
/// Get the thumbnail of size `preferred_size` from the raw file at
/// `filename` (a path), and return the [`ORThumbnailRef`] at the location
/// pointed by `thumb`.
///
/// If `thumb` is `null`, [`NOT_AREF`][or_error::NOT_AREF] is returned,
/// [`NONE`][or_error::NONE] in case of success. Any other error will
/// be returned too.
/// Only if `NONE` is returned then `*thumb` will contain a valid
/// `ORThumbnailRef`.
/// The returned `ORThumbnailRef` must be freed by the caller with
/// `or_thumbnail_release`.
///
/// # Safety
/// Dereference pointers.
unsafe extern "C" fn or_get_extract_thumbnail(
    filename: *const c_char,
    preferred_size: u32,
    thumb: *mut ORThumbnailRef,
) -> or_error {
    if thumb.is_null() {
        return or_error::NOT_AREF;
    }
    let filename = unsafe { CStr::from_ptr(filename) };
    let thumbnail = rawfile_from_file(OsStr::from_bytes(filename.to_bytes()), None)
        .and_then(|rawfile| rawfile.thumbnail_for_size(preferred_size))
        .map(Box::new)
        .map_err(or_error::from);
    if let Err(err) = thumbnail {
        err
    } else {
        unsafe { *thumb = Box::into_raw(thumbnail.unwrap()) };

        or_error::NONE
    }
}

#[cfg(feature = "capi")]
#[no_mangle]
/// Get the raw data from the raw file at `filename` (a path), and
/// return the [`ORRawDataRef`] at the location pointed by `rawdata`.
///
/// If `rawdata` is `null`, [`NOT_AREF`][or_error::NOT_AREF] is returned,
/// [`NONE`][or_error::NONE] in case of success. Any other error will
/// be returned too.
/// Only if `NONE` is returned then `*rawdata` will contain a valid
/// `ORRawDataRef`.
/// The returned `ORRawDataRef` must be freed by the caller with
/// `or_rawdata_release`.
///
/// # Safety
/// Dereference pointers.
unsafe extern "C" fn or_get_extract_rawdata(
    filename: *const c_char,
    options: u32,
    rawdata: *mut ORRawDataRef,
) -> or_error {
    if rawdata.is_null() {
        return or_error::NOT_AREF;
    }

    let filename = unsafe { CStr::from_ptr(filename) };
    let skip_decompression = options == or_options::DONT_DECOMPRESS as u32;
    let rawdata_ = rawfile_from_file(OsStr::from_bytes(filename.to_bytes()), None)
        .and_then(|rawfile| rawfile.load_rawdata(skip_decompression))
        .map(Box::new)
        .map_err(or_error::from);
    if let Err(err) = rawdata_ {
        err
    } else {
        unsafe { *rawdata = Box::into_raw(rawdata_.unwrap()) };

        or_error::NONE
    }
}

#[cfg(feature = "capi")]
/// The storage for the static strings. The CString array
/// is just used to keep the pointers valid.
static STATIC_EXTS: Lazy<Vec<CString>> = Lazy::new(|| {
    crate::extensions()
        .iter()
        .map(|e| CString::new(e.as_bytes()).expect("static C string failed"))
        .collect()
});

#[cfg(feature = "capi")]
/// Wrap the Vector for Sync / Send
/// This is exclusively for the lazy statics below.
struct WrappedVec(Vec<*const c_char>);

#[cfg(feature = "capi")]
unsafe impl Send for WrappedVec {}
#[cfg(feature = "capi")]
unsafe impl Sync for WrappedVec {}

#[cfg(feature = "capi")]
/// The static value for the pointer array to be returned
/// by `or_get_file_extensions`
static EXTS: Lazy<WrappedVec> = Lazy::new(|| {
    let mut extensions: Vec<*const c_char> = STATIC_EXTS
        .iter()
        .map(|e| e.as_c_str().as_ptr() as *const c_char)
        .collect();
    extensions.push(std::ptr::null());

    WrappedVec(extensions)
});

#[cfg(feature = "capi")]
#[no_mangle]
/// Return the list a file extensions the library can handle.
///
/// The returning value is a static `const char**` which a `null`
/// terminated array of `const char*` (C strings). It is owned by
/// the library.
extern "C" fn or_get_file_extensions() -> *const *const c_char {
    EXTS.0.as_slice().as_ptr()
}
