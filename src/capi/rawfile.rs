// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - capi/rawfile.rs
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

//! This contain all the `or_rawfile_*` APIs.

use std::ffi::{CStr, OsStr};
use std::os::raw::c_char;
// This is not portable to Windows
use std::os::unix::ffi::OsStrExt;

use crate::render::RenderingOptions;
use crate::tiff::exif;
use crate::{or_unwrap, rawfile_from_file, rawfile_from_io, RawFileHandle, Type};

use super::iterator::ORMetadataIterator;
use super::metavalue::ORMetaValue;
use super::{
    or_colour_matrix_origin, or_error, or_ifd_dir_type, or_options, ORBitmapDataRef, ORIfdDirRef,
    ORMetaValueRef, ORMetadataIteratorRef, ORRawDataRef, ORThumbnailRef,
};

#[allow(non_camel_case_types)]
/// The type id of the raw file: the 16 MSb are a vendor id
/// and the 16 LSb are the camera id.
pub type or_rawfile_typeid = u32;

#[allow(non_camel_case_types)]
/// Equivalent to [`Type`]: the type of the raw file.
pub type or_rawfile_type = u32;

/// Wrapper for the [RawFile] trait. This is because we can't expose
/// traits, and also we need to refcount it.
#[derive(Clone)]
pub struct ORRawFile(RawFileHandle);
/// Pointer to a [`ORRawFile`] object wrapper exported to the C API.
pub type ORRawFileRef = *mut ORRawFile;

#[no_mangle]
/// Open a new raw file located at `filename` (a path to the file system).
///
/// `type_` is a hint type, or pass [`Unknown`][Type::Unknown] to let the
/// library detect the file.
/// It will return a [`ORRawFileRef`], that must be freed in with
/// [`or_rawfile_release`].
extern "C" fn or_rawfile_new(filename: *mut c_char, type_: Type) -> ORRawFileRef {
    let filename = unsafe { CStr::from_ptr(filename) };
    let type_ = if type_ == Type::Unknown {
        None
    } else {
        Some(type_)
    };
    if let Ok(rawfile) = rawfile_from_file(OsStr::from_bytes(filename.to_bytes()), type_) {
        Box::into_raw(Box::new(ORRawFile(rawfile)))
    } else {
        std::ptr::null_mut()
    }
}

#[no_mangle]
/// Open a raw file in `buffer`, a memory buffer of `len` bytes.
///
/// `type_` is a type hint, like for [`or_rawfile_new`].
/// It will return a [`ORRawFileRef`], that must be freed in with
/// [`or_rawfile_release`].
extern "C" fn or_rawfile_new_from_memory(buffer: *const u8, len: u32, type_: Type) -> ORRawFileRef {
    let bytes = unsafe { std::slice::from_raw_parts(buffer, len as usize) };
    let type_ = if type_ == Type::Unknown {
        None
    } else {
        Some(type_)
    };
    let buffer = Vec::from(bytes);
    let io = Box::new(std::io::Cursor::new(buffer));
    match rawfile_from_io(io, type_) {
        Ok(rawfile) => Box::into_raw(Box::new(ORRawFile(rawfile))),
        Err(_) => std::ptr::null_mut(),
    }
}

#[no_mangle]
/// Release `rawfile` of type [`ORRawFileRef`], and return an error code.
///
/// If rawfile is `null`, then [`NOT_AREF`][or_error::NOT_AREF] is returned,
/// otherwise [`NONE`][or_error::NONE].
/// Passing an invalid ref (not null) or of the wrong type is an error
/// and will cause undefined behaviour.
extern "C" fn or_rawfile_release(rawfile: ORRawFileRef) -> or_error {
    if !rawfile.is_null() {
        unsafe { drop(Box::from_raw(rawfile)) };
        return or_error::NONE;
    }
    or_error::NOT_AREF
}

#[no_mangle]
/// Get the type of `rawfile`.
///
/// A returned value of `0` denote either an error or a an unknown file.
/// A non null invalid `rawfile` is undefined behaviour.
extern "C" fn or_rawfile_get_type(rawfile: ORRawFileRef) -> or_rawfile_type {
    or_unwrap!(rawfile, 0, rawfile.0.type_() as u32)
}

#[no_mangle]
/// Get the type id of `rawfile`.
///
/// A returned value of `0` denote either an error or a an unknown file.
/// A non null invalid `rawfile` is undefined behaviour.
extern "C" fn or_rawfile_get_typeid(rawfile: ORRawFileRef) -> or_rawfile_typeid {
    or_unwrap!(rawfile, 0, rawfile.0.identify_id().into())
}

#[no_mangle]
/// Get the vendor id, extracted from the type id of `rawfile`.
///
/// A returned value of `0` denote either an error or a an unknown file.
/// This the the 16 MSb of the type id, shift to the right as to be the
/// 16 LSb of the returned value.
/// A non null invalid `rawfile` is undefined behaviour.
extern "C" fn or_rawfile_get_vendorid(rawfile: ORRawFileRef) -> or_rawfile_typeid {
    or_unwrap!(rawfile, 0, rawfile.0.identify_id().0.into())
}

#[no_mangle]
/// Get the rawdata for the rawfile.
///
/// The returned `ORRawDataRef` must be freed with `or_rawdata_release`.
extern "C" fn or_rawfile_get_rawdata(
    rawfile: ORRawFileRef,
    options: u32,
    error: *mut or_error,
) -> ORRawDataRef {
    or_unwrap!(rawfile, std::ptr::null_mut(), {
        let skip_decompression = options == or_options::DONT_DECOMPRESS as u32;
        rawfile
            .0
            .raw_data(skip_decompression)
            .map(|rawdata| {
                if !error.is_null() {
                    unsafe { *error = or_error::NONE }
                }
                Box::into_raw(Box::new(rawdata))
            })
            .map_err(|err| {
                if !error.is_null() {
                    unsafe { *error = err.into() }
                }
            })
            .unwrap_or(std::ptr::null_mut())
    })
}

#[no_mangle]
extern "C" fn or_rawfile_get_orientation(rawfile: ORRawFileRef) -> i32 {
    or_unwrap!(rawfile, 0, rawfile.0.orientation() as i32)
}

#[no_mangle]
extern "C" fn or_rawfile_get_thumbnail(
    rawfile: ORRawFileRef,
    preferred_size: u32,
    error: *mut or_error,
) -> ORThumbnailRef {
    or_unwrap!(rawfile, std::ptr::null_mut(), {
        rawfile
            .0
            .thumbnail(preferred_size)
            .map(|rawfile| {
                if !error.is_null() {
                    unsafe { *error = or_error::NONE }
                }
                Box::into_raw(Box::new(rawfile))
            })
            .map_err(|err| {
                if !error.is_null() {
                    unsafe { *error = err.into() }
                }
            })
            .unwrap_or(std::ptr::null_mut())
    })
}

#[no_mangle]
/// Return the thubmnail sizes. The pointer belong to the `rawfile`.
extern "C" fn or_rawfile_get_thumbnail_sizes(
    rawfile: ORRawFileRef,
    size: *mut libc::size_t,
) -> *const u32 {
    or_unwrap!(rawfile, std::ptr::null(), {
        let sizes = rawfile.0.thumbnail_sizes();
        if !size.is_null() {
            unsafe { *size = sizes.len() };
        }
        // sizes belong to the rawfile.
        sizes.as_ptr()
    })
}

#[no_mangle]
/// Get metavalue for `key`.
/// The metavalue should be released by the caller.
extern "C" fn or_rawfile_get_metavalue(
    rawfile: ORRawFileRef,
    key: *const std::ffi::c_char,
) -> ORMetaValueRef {
    or_unwrap!(rawfile, std::ptr::null_mut(), {
        let key = unsafe { std::ffi::CStr::from_ptr(key) };
        rawfile
            .0
            .metadata_value(&key.to_string_lossy().to_string())
            .map(ORMetaValue::from)
            .map(Box::new)
            .map(Box::into_raw)
            .unwrap_or(std::ptr::null_mut())
    })
}

#[no_mangle]
/// Get the rendered image. The returned ORBitmapDataRef must be freed.
extern "C" fn or_rawfile_get_rendered_image(
    rawfile: ORRawFileRef,
    options: u32,
    error: *mut or_error,
) -> ORBitmapDataRef {
    let options = RenderingOptions::from(options);
    or_unwrap!(rawfile, std::ptr::null_mut(), {
        rawfile
            .0
            .rendered_image(options)
            .map(|r| Box::into_raw(Box::new(r)))
            .unwrap_or_else(|e| {
                if !error.is_null() {
                    unsafe { *error = e.into() };
                }
                std::ptr::null_mut()
            })
    })
}

#[no_mangle]
/// Get the IFD with type `ifd_type`. May return `null` if not found.
extern "C" fn or_rawfile_get_ifd(rawfile: ORRawFileRef, ifd_type: or_ifd_dir_type) -> ORIfdDirRef {
    or_unwrap!(
        rawfile,
        std::ptr::null(),
        rawfile
            .0
            .ifd(ifd_type.into())
            .map(|dir| dir as ORIfdDirRef)
            .unwrap_or(std::ptr::null())
    )
}

fn rawfile_get_calibration_illuminant(rawfile: ORRawFileRef, index: u32) -> exif::LightsourceValue {
    or_unwrap!(rawfile, exif::LightsourceValue::Unknown, {
        rawfile.0.calibration_illuminant(index)
    })
}

#[no_mangle]
extern "C" fn or_rawfile_get_calibration_illuminant1(
    rawfile: ORRawFileRef,
) -> exif::LightsourceValue {
    rawfile_get_calibration_illuminant(rawfile, 1)
}

#[no_mangle]
extern "C" fn or_rawfile_get_calibration_illuminant2(
    rawfile: ORRawFileRef,
) -> exif::LightsourceValue {
    rawfile_get_calibration_illuminant(rawfile, 2)
}

fn rawfile_get_colour_matrix(
    rawfile: ORRawFileRef,
    index: u32,
    matrix: *mut std::ffi::c_double,
    size: *mut u32,
) -> or_error {
    or_unwrap!(rawfile, or_error::NOT_AREF, {
        if size.is_null() {
            return or_error::INVALID_PARAM;
        }
        rawfile.0.colour_matrix(index as usize).map_or_else(
            |err| err.into(),
            |cm| {
                let out_size = unsafe { *size } as usize;
                if out_size < cm.1.len() {
                    return or_error::BUF_TOO_SMALL;
                }
                let slice = unsafe { std::slice::from_raw_parts_mut(matrix, out_size) };
                slice.copy_from_slice(&cm.1[..out_size]);
                unsafe { *size = cm.1.len() as u32 };
                or_error::NONE
            },
        )
    })
}

#[no_mangle]
extern "C" fn or_rawfile_get_colourmatrix1(
    rawfile: ORRawFileRef,
    matrix: *mut std::ffi::c_double,
    size: *mut u32,
) -> or_error {
    rawfile_get_colour_matrix(rawfile, 1, matrix, size)
}

#[no_mangle]
extern "C" fn or_rawfile_get_colourmatrix2(
    rawfile: ORRawFileRef,
    matrix: *mut std::ffi::c_double,
    size: *mut u32,
) -> or_error {
    rawfile_get_colour_matrix(rawfile, 2, matrix, size)
}

#[no_mangle]
extern "C" fn or_rawfile_get_colour_matrix_origin(
    rawfile: ORRawFileRef,
) -> or_colour_matrix_origin {
    or_unwrap!(rawfile, or_colour_matrix_origin::Unknown, {
        rawfile
            .0
            .colour_matrix(1)
            .map(|m| m.0)
            .unwrap_or(or_colour_matrix_origin::Unknown)
    })
}

#[no_mangle]
extern "C" fn or_rawfile_get_metadata_iterator(rawfile: ORRawFileRef) -> ORMetadataIteratorRef {
    or_unwrap!(
        rawfile,
        std::ptr::null_mut(),
        Box::into_raw(Box::new(ORMetadataIterator(
            rawfile.0.metadata(),
            rawfile.clone(),
            None
        )))
    )
}
