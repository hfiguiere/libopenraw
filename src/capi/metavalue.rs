// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - capi/metavalue.rs
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

//! API to operate on ORMetaValueRef.

use std::ffi::CString;

use once_cell::unsync::OnceCell;

use crate::metadata;
use crate::or_unwrap;

/// Wrap a metavalue to carry a once String conversion
pub struct ORMetaValue(metadata::Value, OnceCell<CString>);

impl ORMetaValue {
    /// Convert to string, and store it.
    /// Any subsequent call will always return the same string.
    pub fn to_c_string(&self, full: bool) -> &CString {
        self.1
            .get_or_init(|| CString::new(self.0.into_string(full)).unwrap())
    }
}

impl From<metadata::Value> for ORMetaValue {
    /// ORMetaValue constructor
    fn from(value: metadata::Value) -> Self {
        ORMetaValue(value, OnceCell::new())
    }
}

/// A const metavalue.
pub type ORConstMetaValueRef = *const ORMetaValue;
/// A mutable metavalue, usually need to be released.
pub type ORMetaValueRef = *mut ORMetaValue;

#[no_mangle]
/// Release a metavalue.
extern "C" fn or_metavalue_release(metavalue: ORMetaValueRef) {
    if !metavalue.is_null() {
        unsafe { drop(Box::from_raw(metavalue)) };
    }
}

#[no_mangle]
/// Get the string value out of a metevalue. May return `null` if it
/// can't be converted.
/// The string pointer is owned by the metavalue.
extern "C" fn or_metavalue_get_string(value: ORConstMetaValueRef) -> *const std::ffi::c_char {
    or_unwrap!(value, std::ptr::null(), {
        if let metadata::Value::String(s) = &value.0 {
            s.as_ptr() as *const std::ffi::c_char
        } else {
            std::ptr::null()
        }
    })
}

#[no_mangle]
/// Convert the metavalue to a string. `full` mean that it's not truncated.
///
/// The string belong to the metavalue.
extern "C" fn or_metavalue_get_as_string(
    value: ORConstMetaValueRef,
    full: bool,
) -> *const std::ffi::c_char {
    // the wrapper store the conversion
    or_unwrap!(
        value,
        std::ptr::null(),
        value.to_c_string(full).as_ptr() as *const std::ffi::c_char
    )
}

#[no_mangle]
/// Get the count of items in the metavalue.
///
/// For a string always return 1
extern "C" fn or_metavalue_get_count(value: ORConstMetaValueRef) -> u32 {
    or_unwrap!(value, 0, value.0.count() as u32)
}

#[cfg(test)]
mod test {
    use std::ffi::CString;

    use super::ORMetaValue;
    use crate::metadata::Value;

    #[test]
    fn test_metavalue_to_c_string() {
        // test that the string is stored internally.
        let i = ORMetaValue::from(Value::Int(vec![0; 25]));
        let s = i.to_c_string(true);
        assert_eq!(
            s,
            &CString::new(
                "[ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ]"
            )
            .unwrap()
        );
        let s = i.to_c_string(false);
        assert_eq!(
            s,
            &CString::new(
                "[ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ]"
            )
            .unwrap()
        );

        let i = ORMetaValue::from(Value::Int(vec![0; 25]));
        let s = i.to_c_string(false);
        assert_eq!(
            s,
            &CString::new("[ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ...]")
                .unwrap()
        );
        let s = i.to_c_string(true);
        assert_eq!(
            s,
            &CString::new("[ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ...]")
                .unwrap()
        );
    }
}
