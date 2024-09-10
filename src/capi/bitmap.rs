// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - capi/bitmap.rs
 *
 * Copyright (C) 2023-2024 Hubert Figuière
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

//! This contain all the `or_bitmapdata_*` APIs.

use crate::{or_unwrap, Bitmap, DataType, RawImage};

use super::{or_data_type, or_error};

/// Pointer to a [`Thumbnail`] object exported to the C API.
pub type ORBitmapDataRef = *mut RawImage;

#[no_mangle]
/// Release `bitmap` of type [`ORBitmapref`], and return an error code.
///
/// If bitmap is `null`, then [`NOT_AREF`][or_error::NOT_AREF] is returned,
/// otherwise [`NONE`][or_error::NONE].
/// Passing an invalid ref (not null) or of the wrong type is an error
/// and will cause undefined behaviour.
extern "C" fn or_bitmapdata_release(bitmap: ORBitmapDataRef) -> or_error {
    if !bitmap.is_null() {
        unsafe { drop(Box::from_raw(bitmap)) };
        return or_error::NONE;
    }
    or_error::NOT_AREF
}

#[no_mangle]
extern "C" fn or_bitmapdata_data_size(bitmap: ORBitmapDataRef) -> libc::size_t {
    or_unwrap!(bitmap, 0, bitmap.data_size())
}

#[no_mangle]
extern "C" fn or_bitmapdata_data(bitmap: ORBitmapDataRef) -> *const libc::c_void {
    or_unwrap!(
        bitmap,
        std::ptr::null(),
        if bitmap.data_type() == DataType::PixmapRgb16 {
            bitmap
                .data16()
                .map(|data| data.as_ptr())
                .unwrap_or_else(std::ptr::null) as *const libc::c_void
        } else {
            bitmap
                .data8()
                .map(|data| data.as_ptr())
                .unwrap_or_else(std::ptr::null) as *const libc::c_void
        }
    )
}

#[no_mangle]
extern "C" fn or_bitmapdata_format(bitmap: ORBitmapDataRef) -> or_data_type {
    or_unwrap!(bitmap, or_data_type::UNKNOWN, bitmap.data_type().into())
}

#[no_mangle]
extern "C" fn or_bitmapdata_dimensions(bitmap: ORBitmapDataRef, x: *mut u32, y: *mut u32) {
    or_unwrap!(bitmap, (), {
        if !x.is_null() {
            unsafe { *x = bitmap.width() };
        }
        if !y.is_null() {
            unsafe { *y = bitmap.height() };
        }
    })
}
