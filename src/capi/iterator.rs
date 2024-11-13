// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - capi/iterator.rs
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

use crate::{metadata, or_unwrap, or_unwrap_mut};

use super::metavalue::ORMetaValue;
use super::rawfile::ORRawFile;
use super::{ORConstMetaValueRef, ORIfdDirRef};

pub struct ORMetadata {
    key: std::ffi::CString,
    value: ORMetaValue,
    type_: i16,
}

impl From<metadata::Metadata> for ORMetadata {
    fn from(value: metadata::Metadata) -> Self {
        Self {
            key: std::ffi::CString::new(value.0).unwrap(),
            value: ORMetaValue::from(value.1),
            type_: value.2,
        }
    }
}

/// Pointer type to a metadata
pub type ORMetadataRef = *const ORMetadata;

/// Metadata iterator. It keeps the ORRawFile.
pub struct ORMetadataIterator<'a>(
    pub metadata::Iterator<'a>,
    // We need to keep the raw file alive, but we never read it.
    #[allow(dead_code)]
    pub ORRawFile,
    pub Option<ORMetadata>,
);

/// Pointer type to a metadata iterator with a 'static lifetime.
pub type ORMetadataIteratorRef = *mut ORMetadataIterator<'static>;

#[no_mangle]
extern "C" fn or_metadata_iterator_free(iterator: ORMetadataIteratorRef) {
    if !iterator.is_null() {
        unsafe {
            let _ = Box::from_raw(iterator);
        }
    }
}

#[no_mangle]
extern "C" fn or_metadata_iterator_next(iterator: ORMetadataIteratorRef) -> libc::c_int {
    or_unwrap_mut!(iterator, 0, {
        let item = iterator.0.next();
        let r = if item.is_some() { 1 } else { 0 };
        iterator.2 = item.map(ORMetadata::from);
        r
    })
}

#[no_mangle]
extern "C" fn or_metadata_iterator_get_entry(iterator: ORMetadataIteratorRef) -> ORMetadataRef {
    or_unwrap!(
        iterator,
        std::ptr::null(),
        iterator
            .2
            .as_ref()
            .map(|data| data as ORMetadataRef)
            .unwrap_or_else(std::ptr::null)
    )
}

#[no_mangle]
/// Return the current IFD for the iterator, or null.
extern "C" fn or_metadata_iterator_get_dir(iterator: ORMetadataIteratorRef) -> ORIfdDirRef {
    or_unwrap!(
        iterator,
        std::ptr::null(),
        iterator
            .0
            .dir()
            .map(|dir| dir as ORIfdDirRef)
            .unwrap_or_else(std::ptr::null)
    )
}

#[no_mangle]
/// Get the key out of a metadata. May return null.
extern "C" fn or_metadata_get_key(metadata: ORMetadataRef) -> *const libc::c_char {
    or_unwrap!(
        metadata,
        std::ptr::null(),
        metadata.key.as_ptr() as *const libc::c_char
    )
}

#[no_mangle]
/// Get the value out of a metadata. May return null.
extern "C" fn or_metadata_get_value(metadata: ORMetadataRef) -> ORConstMetaValueRef {
    or_unwrap!(
        metadata,
        std::ptr::null(),
        &metadata.value as ORConstMetaValueRef
    )
}
#[no_mangle]
extern "C" fn or_metadata_get_type(metadata: ORMetadataRef) -> i16 {
    or_unwrap!(metadata, 0, metadata.type_)
}
