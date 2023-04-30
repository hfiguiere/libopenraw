// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - tiff/iterator.rs
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

//! The iterator over the ifd entries as `metadata::Value`

use std::collections::btree_map;

use crate::container::Endian;
use crate::metadata::Value as MetadataValue;

use super::{exif, Dir, Entry, IfdType};

/// Iterate over the entries.
pub(crate) struct Iterator<'a> {
    dir: &'a Dir,
    //    container: &'a tiff::Container,
    iter: btree_map::Iter<'a, u16, Entry>,
}

impl<'a> Iterator<'a> {
    /// Create iterator from `Dir`.
    pub(super) fn new(dir: &'a Dir /* container: &'a tiff::Container */) -> Iterator<'a> {
        let iter = dir.entries.iter();
        Iterator { dir, iter }
    }
}

impl<'a> std::iter::Iterator for Iterator<'a> {
    type Item = (String, MetadataValue);

    fn next(&mut self) -> Option<Self::Item> {
        let ns = match self.dir.type_ {
            IfdType::Main => "Exif.Image",
            IfdType::Exif => "Exif.Photo",
            IfdType::Raw => "Raw",
            IfdType::MakerNote => "MakerNote",
            _ => "other",
        };
        self.iter.next().map(|e| {
            let tag_name = self.dir.tag_names.get(e.0).unwrap_or(&"");

            let value = from_entry(e.1, self.dir.endian);
            (format!("{ns}.{tag_name}"), value)
        })
    }
}

fn from_entry(entry: &Entry, endian: Endian) -> MetadataValue {
    match exif::TagType::from(entry.type_) {
        exif::TagType::Ascii => MetadataValue::String(entry.string_value().unwrap()),
        exif::TagType::Rational => {
            MetadataValue::Rational(entry.value_array::<exif::Rational>(endian).unwrap())
        }
        exif::TagType::SRational => {
            MetadataValue::SRational(entry.value_array::<exif::SRational>(endian).unwrap())
        }
        exif::TagType::Float => MetadataValue::Float(entry.value_array::<f32>(endian).unwrap()),
        exif::TagType::Double => MetadataValue::Double(entry.value_array::<f64>(endian).unwrap()),
        exif::TagType::Byte | exif::TagType::Undefined => MetadataValue::Bytes(Vec::default()), //entry.data().to_vec())
        exif::TagType::SByte => MetadataValue::SBytes(Vec::default()), //entry.data().to_vec())
        exif::TagType::Short | exif::TagType::Long => {
            MetadataValue::Int(entry.uint_value_array(endian).unwrap())
        }
        exif::TagType::SShort | exif::TagType::SLong => {
            MetadataValue::SInt(entry.int_value_array(endian).unwrap())
        }
        exif::TagType::Invalid => MetadataValue::Invalid(Vec::default()), //entry.data().to_vec())
    }
}
