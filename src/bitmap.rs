// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - bitmap.rs
 *
 * Copyright (C) 2022-2025 Hubert Figuière
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

use crate::{DataType, FloatPixel};

/// An image buffer carries the data and the dimension. It is used to
/// carry pipeline input and ouput as dimensions can change.
#[derive(Clone)]
pub struct ImageBuffer<T> {
    pub(crate) data: Vec<T>,
    pub(crate) width: usize,
    pub(crate) height: usize,
    /// bits per channel
    pub(crate) bpc: u16,
    /// number of channel
    pub(crate) cc: usize,
}

impl<T> ImageBuffer<T> {
    pub(crate) fn new(width: usize, height: usize, bpc: u16, cc: usize) -> Self
    where
        T: Default + Clone,
    {
        Self {
            data: vec![T::default(); width * height],
            width,
            height,
            bpc,
            cc,
        }
    }

    /// Create an image buffer.
    pub(crate) fn with_data(
        data: Vec<T>,
        width: usize,
        height: usize,
        bpc: u16,
        cc: usize,
    ) -> Self {
        Self {
            data,
            width,
            height,
            bpc,
            cc,
        }
    }

    #[inline]
    /// Calculate the index on the buffer for the pixel at `row`, `col`.
    /// It return the index on component 0.
    pub(crate) fn pixel_index(&self, row: usize, col: usize) -> usize {
        row * self.width * self.cc + col * self.cc
    }

    /// Return the pixel RGB value at `row` and `col`.
    pub(crate) fn pixel_at(&self, row: usize, col: usize) -> Option<&[T]>
    where
        T: Copy,
    {
        if col > self.width || row > self.height {
            return None;
        }
        let pos = (row * self.width * self.cc) + col * self.cc;
        let pixel = &self.data[pos..pos + self.cc];

        Some(pixel)
    }

    #[inline]
    /// Return a mut pixel value at `row` and `col` for component `c`.
    // XXX make sure to handle overflows.
    pub(crate) fn mut_pixel_at(&mut self, row: usize, col: usize, c: usize) -> &mut T
    where
        T: Copy,
    {
        let index = self.pixel_index(row, col);
        &mut self.data[index + c]
    }
}

impl ImageBuffer<FloatPixel> {
    pub(crate) fn into_u16(self) -> ImageBuffer<u16> {
        ImageBuffer::<u16>::with_data(
            self.data
                .iter()
                .map(|v| (*v * u16::MAX as FloatPixel).round() as u16)
                .collect(),
            self.width,
            self.height,
            16,
            self.cc,
        )
    }
}

impl ImageBuffer<u16> {
    /// Convert an u16 buffer to f64 downsampling (binning) components.
    /// This is currently only used for X-Trans due to the specifics of
    /// the algortithm and its expectation of input.
    ///
    /// Set `ncolor` to `self.cc` to not perform a downsampling.
    /// # Assert
    /// Assert if `ncolor` is > `self.cc`
    pub(crate) fn into_float_downsample(self, ncolor: usize) -> ImageBuffer<FloatPixel> {
        let old_ncolor = self.cc;
        assert!(old_ncolor >= ncolor);
        ImageBuffer::with_data(
            self.data
                .iter()
                .enumerate()
                .filter_map(|(idx, v)| {
                    if idx % old_ncolor < ncolor {
                        Some(*v as FloatPixel / u16::MAX as FloatPixel)
                    } else {
                        None
                    }
                })
                .collect(),
            self.width,
            self.height,
            16,
            ncolor,
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
    /// Image data in 16 bits. `None` by default
    fn data16(&self) -> Option<&[u16]> {
        None
    }
}

/// Encapsulate data 8 or 16 bits
#[derive(Clone)]
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

#[cfg(test)]
mod test {
    use crate::FloatPixel;

    use super::ImageBuffer;

    #[test]
    fn test_into_float_downsample() {
        let mut data = [0_u16; 64];
        data.iter_mut().enumerate().for_each(|(idx, value)| {
            *value = idx as u16 + 1;
        });

        let buffer = ImageBuffer::with_data(data.into(), 4, 4, 16, 4);
        assert_eq!(buffer.data[0], 1);
        assert_eq!(buffer.data[3], 4);
        assert_eq!(buffer.data[4], 5);

        // No downsampling
        let buffer2 = buffer.into_float_downsample(4);
        assert_eq!(buffer2.data[0], 1.0 / u16::MAX as FloatPixel);
        assert_eq!(buffer2.data[3], 4.0 / u16::MAX as FloatPixel);
        assert_eq!(buffer2.data[4], 5.0 / u16::MAX as FloatPixel);

        // Downsampling with binning the 4th component.
        let buffer = ImageBuffer::with_data(data.into(), 4, 4, 16, 4);
        let buffer3 = buffer.into_float_downsample(3);
        assert_eq!(buffer3.data[0], 1.0 / u16::MAX as FloatPixel);
        assert_eq!(buffer3.data[3], 5.0 / u16::MAX as FloatPixel);
        assert_eq!(buffer3.data[4], 6.0 / u16::MAX as FloatPixel);
    }

    #[test]
    #[should_panic(expected = "assertion failed")]
    fn test_into_float_downsample_assert() {
        let mut data = [0_u16; 64];
        data.iter_mut().enumerate().for_each(|(idx, value)| {
            *value = idx as u16 + 1;
        });

        let buffer = ImageBuffer::with_data(data.into(), 4, 4, 16, 4);
        _ = buffer.into_float_downsample(5);
    }
}
