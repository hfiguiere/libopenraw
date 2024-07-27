// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - bitmap.rs
 *
 * Copyright (C) 2022-2023 Hubert Figuière
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

//! Trait and types for various bitmap data, iamges, etc. and other
//! geometry.

use crate::DataType;

/// An image buffer carries the data and the dimension. It is used to
/// carry pipeline input and ouput as dimensions can change.
pub(crate) struct ImageBuffer<T> {
    pub(crate) data: Vec<T>,
    pub(crate) width: u32,
    pub(crate) height: u32,
    /// bits per channel
    pub(crate) bpc: u16,
    /// number of channel
    pub(crate) cc: u32,
}

impl<T> ImageBuffer<T> {
    pub(crate) fn new(width: u32, height: u32, bpc: u16, cc: u32) -> Self
    where
        T: Default + Clone,
    {
        Self {
            data: vec![T::default(); width as usize * height as usize],
            width,
            height,
            bpc,
            cc,
        }
    }

    /// Create an image buffer.
    pub(crate) fn with_data(data: Vec<T>, width: u32, height: u32, bpc: u16, cc: u32) -> Self {
        Self {
            data,
            width,
            height,
            bpc,
            cc,
        }
    }

    /// Return the pixel RGB value at `x` and `y`.
    pub(crate) fn pixel_at(&self, x: u32, y: u32) -> Option<Vec<T>>
    where
        T: Copy,
    {
        if x > self.width || y > self.height {
            return None;
        }
        let pos = ((y * self.width * self.cc) + x * self.cc) as usize;
        let pixel = &self.data[pos..pos + self.cc as usize];

        Some(pixel.to_vec())
    }

    #[inline]
    /// Return a mut pixel value.
    // XXX make sure to handle overflows.
    pub(crate) fn mut_pixel_at(&mut self, row: usize, col: usize, c: usize) -> &mut T
    where
        T: Copy,
    {
        &mut self.data[row * self.width as usize * self.cc as usize + col * self.cc as usize + c]
    }
}

impl ImageBuffer<f64> {
    pub(crate) fn into_u16(self) -> ImageBuffer<u16> {
        ImageBuffer::<u16>::with_data(
            self.data
                .iter()
                .map(|v| (*v * u16::MAX as f64).round() as u16)
                .collect(),
            self.width,
            self.height,
            16,
            self.cc,
        )
    }
}

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
}

/// An `Image` is a more comprehensive `Bitmap` with 16-bits
/// support.
pub trait Image: Bitmap {
    /// Image data in 16 bits
    fn data16(&self) -> Option<&[u16]>;
}

/// Rectangle struct.
#[derive(Clone, Debug, Default, PartialEq)]
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

    /// Generate a `Vec<u32>` with the values in x, y, w, h order.
    pub fn to_vec(&self) -> Vec<u32> {
        [self.x, self.y, self.width, self.height].to_vec()
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
