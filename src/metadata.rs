// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - metadata.rs
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

//! Metatadata

use crate::tiff::{self, exif, Dir};
use crate::utils;

/// The key type. Currently an alias to `String`.
pub type Key = String;

#[derive(Debug, PartialEq)]
pub enum Value {
    Int(Vec<u32>),
    SInt(Vec<i32>),
    String(Vec<u8>),
    Float(Vec<f32>),
    Double(Vec<f64>),
    Rational(Vec<exif::Rational>),
    SRational(Vec<exif::SRational>),
    Bytes(Vec<u8>),
    SBytes(Vec<i8>),
    Invalid(Vec<u8>),
}

/// Metadata type.
pub type Metadata = (Key, Value, i16);

impl Value {
    pub fn count(&self) -> usize {
        match *self {
            Self::Int(ref v) => v.len(),
            Self::SInt(ref v) => v.len(),
            // String has one value
            Self::String(_) => 1,
            Self::Float(ref v) => v.len(),
            Self::Double(ref v) => v.len(),
            Self::Rational(ref v) => v.len(),
            Self::SRational(ref v) => v.len(),
            Self::Bytes(ref v) => v.len(),
            Self::SBytes(ref v) => v.len(),
            Self::Invalid(ref v) => v.len(),
        }
    }

    pub fn string(&self) -> Option<String> {
        match self {
            Self::String(s) => Some(utils::from_maybe_nul_terminated(s)),
            _ => None,
        }
    }

    fn value_into_string(&self, idx: usize) -> String {
        match self {
            Self::Int(ref v) => v[idx].to_string(),
            Self::SInt(ref v) => v[idx].to_string(),
            Self::String(s) => utils::from_maybe_nul_terminated(s),
            Self::Float(ref v) => v[idx].to_string(),
            Self::Double(ref v) => v[idx].to_string(),
            Self::Rational(ref v) => v[idx].to_string(),
            Self::SRational(ref v) => v[idx].to_string(),
            Self::Bytes(ref v) => v[idx].to_string(),
            Self::SBytes(ref v) => v[idx].to_string(),
            Self::Invalid(ref v) => v[idx].to_string(),
        }
    }

    pub fn into_string(&self, full: bool) -> String {
        let mut count = 0;
        let multiple = self.count() > 1;
        let mut output = String::new();
        if self.is_string() {
            output.push_str(&self.string().unwrap());
        } else {
            if multiple {
                output.push_str("[ ");
            }
            for i in 0..self.count() {
                output.push_str(&self.value_into_string(i));
                if multiple {
                    output.push_str(", ");
                }
                count += 1;
                if !full && count > 20 {
                    output.push_str("...");
                    break;
                }
            }
            if multiple {
                output.push(']');
            }
        }

        output
    }

    pub(crate) fn is_string(&self) -> bool {
        matches!(self, Self::String(_))
    }

    pub fn integer(&self) -> Option<u32> {
        match self {
            Self::Int(i) => Some(i[0]),
            _ => None,
        }
    }

    pub fn rational(&self) -> Option<exif::Rational> {
        match self {
            Self::Rational(r) => Some(r[0]),
            _ => None,
        }
    }
}

/// Inner iterator
/// This is use to go through the containers.
#[derive(Default)]
enum InnerIter<'a> {
    Container(tiff::DirIterator<'a>),
    Ifd(tiff::Iterator<'a>),
    #[default]
    Empty,
}

/// Metadata iterator that yield key value tuples.
#[derive(Default)]
pub struct Iterator<'a> {
    inner: InnerIter<'a>,
    /// Stack of parent inner iterators.
    stack: Vec<InnerIter<'a>>,
}

impl Iterator<'_> {
    /// Return the current IFD if there is one.
    pub(crate) fn dir(&self) -> Option<&Dir> {
        match self.inner {
            InnerIter::Ifd(ref iter) => Some(iter.dir()),
            _ => None,
        }
    }
}

impl std::iter::Iterator for Iterator<'_> {
    type Item = Metadata;

    fn next(&mut self) -> Option<Self::Item> {
        let next = match self.inner {
            InnerIter::Container(ref mut iter) => iter.next().and_then(|container| {
                let mut iter = InnerIter::Ifd(container.iter());
                std::mem::swap(&mut self.inner, &mut iter);
                self.stack.push(iter);
                self.next()
            }),
            InnerIter::Ifd(ref mut iter) => iter.next(),
            InnerIter::Empty => return None,
        };
        if next.is_none() {
            if let Some(iter) = self.stack.pop() {
                self.inner = iter;
                return self.next();
            }
        }
        next
    }
}

impl<'a> std::convert::From<tiff::Iterator<'a>> for Iterator<'a> {
    fn from(tiff_iter: tiff::Iterator<'a>) -> Self {
        Iterator {
            inner: InnerIter::Ifd(tiff_iter),
            stack: vec![],
        }
    }
}

impl<'a> std::convert::From<tiff::DirIterator<'a>> for Iterator<'a> {
    fn from(dir_iter: tiff::DirIterator<'a>) -> Self {
        Iterator {
            inner: InnerIter::Container(dir_iter),
            stack: vec![],
        }
    }
}

#[cfg(test)]
mod test {
    use super::Value;

    #[test]
    fn test_metavalue_to_string() {
        let i = Value::Int(vec![1, 2, 3]);

        assert!(!i.is_string());
        assert_eq!(i.string(), None);
        assert_eq!(i.integer(), Some(1));
        assert_eq!(i.count(), 3);
        let s = i.into_string(true);
        assert_eq!(s, "[ 1, 2, 3, ]");
        let s = i.into_string(false);
        assert_eq!(s, "[ 1, 2, 3, ]");

        let i = Value::Int(vec![0; 25]);
        assert_eq!(i.count(), 25);
        let s = i.into_string(true);
        assert_eq!(
            s,
            "[ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ]"
        );
        let s = i.into_string(false);
        assert_eq!(
            s,
            "[ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ...]"
        );

        let v = Value::String(b"Abracadabra".to_vec());
        assert!(v.is_string());
        assert_eq!(v.count(), 1);
        assert_eq!(v.string(), Some("Abracadabra".to_string()));
        assert_eq!(v.integer(), None);
        let s = v.into_string(true);
        assert_eq!(s, "Abracadabra");
        let s = v.into_string(false);
        assert_eq!(s, "Abracadabra");
    }
}
