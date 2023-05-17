// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - metadata/mod.rs
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

use crate::tiff::{self, exif};

/// The key type. Currently an alias to `String`.
pub type Key = String;

#[derive(Debug)]
pub enum Value {
    Int(Vec<u32>),
    SInt(Vec<i32>),
    String(String),
    Float(Vec<f32>),
    Double(Vec<f64>),
    Rational(Vec<exif::Rational>),
    SRational(Vec<exif::SRational>),
    Bytes(Vec<u8>),
    SBytes(Vec<i8>),
    Invalid(Vec<u8>),
}

impl Value {
    pub fn string(&self) -> Option<String> {
        match self {
            Self::String(s) => Some(s.clone()),
            _ => None,
        }
    }

    pub fn is_string(&self) -> bool {
        matches!(self, Self::String(_))
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

impl<'a> std::iter::Iterator for Iterator<'a> {
    type Item = (Key, Value);

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
