// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - capi/thumbnail.rs
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

//! This contain all the `or_thumbnail_*` APIs.

use crate::{or_unwrap, Bitmap, Thumbnail};

use super::{or_data_type, or_error};

/// Pointer to a [`Thumbnail`] object exported to the C API.
pub type ORThumbnailRef = *mut Thumbnail;

#[no_mangle]
/// Create a new `ThumbnailRef`
///
/// Will return an `ORThumbnailref` that will need to be freed with
/// `or_thumbnail_release`.
extern "C" fn or_thumbnail_new() -> ORThumbnailRef {
    Box::into_raw(Box::new(Thumbnail::new()))
}

#[no_mangle]
/// Release `thumbnail` of type [`ORThumbnailref`], and return an error code.
///
/// If thumbnail is `null`, then [`NOT_AREF`][or_error::NOT_AREF] is returned,
/// otherwise [`NONE`][or_error::NONE].
/// Passing an invalid ref (not null) or of the wrong type is an error
/// and will cause undefined behaviour.
extern "C" fn or_thumbnail_release(thumbnail: ORThumbnailRef) -> or_error {
    if !thumbnail.is_null() {
        unsafe { Box::from_raw(thumbnail) };
        return or_error::NONE;
    }
    or_error::NOT_AREF
}

#[no_mangle]
extern "C" fn or_thumbnail_data_size(thumbnail: ORThumbnailRef) -> libc::size_t {
    or_unwrap!(thumbnail, 0, thumbnail.data_size())
}

#[no_mangle]
extern "C" fn or_thumbnail_data(thumbnail: ORThumbnailRef) -> *const u8 {
    or_unwrap!(
        thumbnail,
        std::ptr::null(),
        thumbnail
            .data8()
            .map(|data| data.as_ptr())
            .unwrap_or_else(std::ptr::null)
    )
}

#[no_mangle]
extern "C" fn or_thumbnail_format(thumbnail: ORThumbnailRef) -> or_data_type {
    or_unwrap!(
        thumbnail,
        or_data_type::UNKNOWN,
        thumbnail.data_type().into()
    )
}

#[no_mangle]
extern "C" fn or_thumbnail_dimensions(thumbnail: ORThumbnailRef, x: *mut u32, y: *mut u32) {
    or_unwrap!(thumbnail, (), {
        if !x.is_null() {
            unsafe { *x = thumbnail.width() };
        }
        if !y.is_null() {
            unsafe { *y = thumbnail.height() };
        }
    })
}
