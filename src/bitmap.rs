// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - bitmap.rs
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

//! Trait for Bitmap data, and other geometry.

use crate::DataType;

/// Trait for bitmap objects.
pub trait Bitmap {
    fn data_type(&self) -> DataType;
    /// The data size is bytes
    fn data_size(&self) -> usize;
    /// Pixel width
    fn width(&self) -> u32;
    /// Pixel height
    fn height(&self) -> u32;
    /// Bits per component
    fn bpc(&self) -> u16;
    /// Image data in 8 bits
    fn data8(&self) -> Option<&[u8]>;
    /// Image data in 16 bits
    fn data16(&self) -> Option<&[u16]>;
}

/// Rectangle struct.
#[derive(Clone, Debug)]
pub struct Rect {
    pub x: u32,
    pub y: u32,
    pub width: u32,
    pub height: u32,
}

impl Rect {
    pub fn new(origin: Point, size: Size) -> Rect {
        Rect {
            x: origin.x,
            y: origin.y,
            width: size.width,
            height: size.height,
        }
    }
}

/// Point struct
#[derive(Debug, PartialEq)]
pub struct Point {
    pub x: u32,
    pub y: u32,
}

/// Size struct
#[derive(Debug, PartialEq)]
pub struct Size {
    pub width: u32,
    pub height: u32,
}

/// Encapsulate data 8 or 16 bits
pub(crate) enum Data {
    Data8(Vec<u8>),
    Data16(Vec<u16>),
    Tiled((Vec<Vec<u8>>, (u32, u32))),
}

impl Default for Data {
    fn default() -> Data {
        Data::Data16(Vec::default())
    }
}

impl std::fmt::Debug for Data {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::result::Result<(), std::fmt::Error> {
        f.write_str(&match *self {
            Self::Data8(ref v) => format!("Data(Data8([{}]))", v.len()),
            Self::Data16(ref v) => format!("Data(Data16([{}]))", v.len()),
            Self::Tiled((ref v, sz)) => format!("Data(Tiled([{}], {:?}))", v.len(), sz),
        })
    }
}
