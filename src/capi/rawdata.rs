// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - capi/rawdata.rs
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

use super::{or_cfa_pattern, or_data_type, or_error, ORMosaicInfoRef};
use crate::{or_unwrap, Bitmap, RawData};

/// Pointer to a [`RawData`] object exported to the C API.
pub type ORRawDataRef = *mut RawData;

#[no_mangle]
/// Create a new `RawDataRef`
///
/// Will return an `ORRawDataref` that will need to be freed with
/// `or_rawdata_release`.
extern "C" fn or_rawdata_new() -> ORRawDataRef {
    Box::into_raw(Box::new(RawData::new()))
}

#[no_mangle]
/// Release `rawdata` of type [`ORRawDataRef`], and return an error code.
///
/// If `rawdata` is `null`, then [`NOT_AREF`][or_error::NOT_AREF] is returned,
/// otherwise [`NONE`][or_error::NONE].
/// Passing an invalid ref (not null) or of the wrong type is an error
/// and will cause undefined behaviour.
extern "C" fn or_rawdata_release(rawdata: ORRawDataRef) -> or_error {
    if !rawdata.is_null() {
        unsafe { Box::from_raw(rawdata) };
        return or_error::NONE;
    }
    or_error::NOT_AREF
}

#[no_mangle]
/// Return the size in bytes of the raw data.
extern "C" fn or_rawdata_data_size(rawdata: ORRawDataRef) -> libc::size_t {
    or_unwrap!(rawdata, 0, rawdata.data_size())
}

#[no_mangle]
extern "C" fn or_rawdata_data(rawdata: ORRawDataRef) -> *const libc::c_void {
    or_unwrap!(
        rawdata,
        std::ptr::null(),
        rawdata
            .data8()
            .map(|data| data.as_ptr() as *const libc::c_void)
            .or_else(|| {
                rawdata
                    .data16()
                    .map(|data| data.as_ptr() as *const libc::c_void)
            })
            .unwrap_or_else(std::ptr::null)
    )
}

#[no_mangle]
/// Return the format of the raw data.
extern "C" fn or_rawdata_format(rawdata: ORRawDataRef) -> or_data_type {
    or_unwrap!(rawdata, or_data_type::UNKNOWN, rawdata.data_type().into())
}

#[no_mangle]
extern "C" fn or_rawdata_get_levels(
    rawdata: ORRawDataRef,
    black: *mut u16,
    white: *mut u16,
) -> or_error {
    or_unwrap!(rawdata, or_error::NOT_AREF, {
        if !black.is_null() {
            unsafe { *black = rawdata.black() };
        }
        if !white.is_null() {
            unsafe { *white = rawdata.white() };
        }
        or_error::NONE
    })
}

#[no_mangle]
extern "C" fn or_rawdata_dimensions(rawdata: ORRawDataRef, x: *mut u32, y: *mut u32) {
    or_unwrap!(rawdata, (), {
        if !x.is_null() {
            unsafe { *x = rawdata.width() };
        }
        if !y.is_null() {
            unsafe { *y = rawdata.height() };
        }
    })
}

#[no_mangle]
extern "C" fn or_rawdata_get_cfa_pattern_type(rawdata: ORRawDataRef) -> or_cfa_pattern {
    or_unwrap!(
        rawdata,
        or_cfa_pattern::NONE,
        rawdata.mosaic_pattern().into()
    )
}

#[no_mangle]
extern "C" fn or_rawdata_get_compression(rawdata: ORRawDataRef) -> u32 {
    or_unwrap!(rawdata, 0, rawdata.compression() as u32)
}

#[no_mangle]
extern "C" fn or_rawdata_bpc(rawdata: ORRawDataRef) -> u32 {
    or_unwrap!(rawdata, 0, rawdata.bpc() as u32)
}

#[no_mangle]
extern "C" fn or_rawdata_get_colour_matrix(
    rawdata: ORRawDataRef,
    index: u32,
    size: *mut u32,
) -> *const std::ffi::c_double {
    or_unwrap!(
        rawdata,
        std::ptr::null(),
        if let Some(matrix) = rawdata.colour_matrix(index as usize) {
            if !size.is_null() {
                unsafe { *size = matrix.len() as u32 };
            }
            matrix.as_ptr()
        } else {
            std::ptr::null()
        }
    )
}

#[no_mangle]
extern "C" fn or_rawdata_get_active_area(
    rawdata: ORRawDataRef,
    x: *mut u32,
    y: *mut u32,
    width: *mut u32,
    height: *mut u32,
) -> or_error {
    if !rawdata.is_null() {
        let rawdata = unsafe { &*rawdata };
        let active_area = rawdata.active_area();
        if active_area.is_none() {
            // XXX Not sure what we expect here
            return or_error::NOT_FOUND;
        }
        let active_area = active_area.unwrap();
        if !x.is_null() {
            unsafe { *x = active_area.x };
        }
        if !y.is_null() {
            unsafe { *y = active_area.y };
        }
        if !width.is_null() {
            unsafe { *width = active_area.width };
        }
        if !height.is_null() {
            unsafe { *height = active_area.height };
        }

        or_error::NONE
    } else {
        or_error::NOT_AREF
    }
}

#[no_mangle]
extern "C" fn or_rawdata_get_mosaicinfo(rawdata: ORRawDataRef) -> ORMosaicInfoRef {
    if !rawdata.is_null() {
        let rawdata = unsafe { &*rawdata };

        rawdata.mosaic_pattern() as ORMosaicInfoRef
    } else {
        std::ptr::null_mut()
    }
}
