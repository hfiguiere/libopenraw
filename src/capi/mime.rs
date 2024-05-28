// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - capi/mime.rs
 *
 * Copyright (C) 2024 Hubert Figuière
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

use std::ffi::{c_char, CStr, CString};

use once_cell::sync::Lazy;

use super::rawfile::or_rawfile_type;
use super::WrappedVec;
use crate::Type;

/// The storage for the static strings. The CString array
/// is just used to keep the pointers valid.
static STATIC_MIME: Lazy<Vec<CString>> = Lazy::new(|| {
    crate::mime_types()
        .iter()
        .map(|e| CString::new(e.as_bytes()).expect("static C string failed"))
        .collect()
});

/// The static value for the pointer array to be returned
/// by `or_get_mime_types`
static MIMETYPES: Lazy<WrappedVec> = Lazy::new(|| {
    let mut mime_types: Vec<*const c_char> = STATIC_MIME
        .iter()
        .map(|e| e.as_c_str().as_ptr() as *const c_char)
        .collect();
    mime_types.push(std::ptr::null());

    WrappedVec(mime_types)
});

#[no_mangle]
/// Return the list of mime types the library can handle.
///
/// The returning value is a static `const char**` which a `null`
/// terminated array of `const char*` (C strings). It is owned by
/// the library.
extern "C" fn or_get_mime_types() -> *const *const c_char {
    MIMETYPES.0.as_slice().as_ptr()
}

#[no_mangle]
/// Get the type for the mime type..
///
/// A returned value of `0` denote either an error or a an unknown file.
/// A non null invalid `rawfile` is undefined behaviour.
extern "C" fn or_get_type_for_mime_type(mime_type: *const c_char) -> or_rawfile_type {
    let mime_type = unsafe { CStr::from_ptr(mime_type) };
    crate::type_for_mime_type(&mime_type.to_string_lossy()).unwrap_or(Type::Unknown) as u32
}
