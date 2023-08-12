// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - rawimage.rs
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

//! RAW data

use super::{Bitmap, DataType, Error, Image, Rect, Result};
use crate::bitmap::{Data, ImageBuffer};
use crate::capi::or_error;
use crate::mosaic::Pattern;
use crate::render::RenderingOptions;
use crate::render::{self, RenderingStage};
use crate::tiff::exif;
use crate::utils;
use crate::{tiff, ColourSpace};

/// RAW Data extracted from the file.
#[derive(Debug, Default)]
pub struct RawImage {
    /// Thumbnail width
    width: u32,
    /// Thumbnail height
    height: u32,
    /// Type if the data
    data_type: DataType,
    /// Wrapped data
    data: Data,
    /// Bits per component
    bpc: u16,
    /// White point
    whites: [u16; 4],
    /// Black point
    blacks: [u16; 4],
    ///
    photom_int: exif::PhotometricInterpretation,
    ///
    compression: tiff::Compression,
    /// Sensor active area
    active_area: Option<Rect>,
    /// The mosaic pattern
    mosaic_pattern: Pattern,
    /// The neutral camera white balance
    as_shot_neutral: [f64; 4],
    /// Colour matrices
    matrices: [Vec<f64>; 2],
    /// Linearization table. len = 2^bpc
    linearization_table: Option<Vec<u16>>,
}

impl RawImage {
    pub fn new() -> Self {
        Self::default()
    }

    /// New `RawImage` with 8 bit data.
    pub fn with_data8(
        width: u32,
        height: u32,
        bpc: u16,
        data_type: DataType,
        data: Vec<u8>,
        mosaic_pattern: Pattern,
    ) -> Self {
        RawImage {
            width,
            height,
            bpc,
            data_type,
            data: Data::Data8(data),
            active_area: None,
            whites: [0, 0, 0, 0],
            blacks: [0, 0, 0, 0],
            compression: tiff::Compression::Unknown,
            photom_int: exif::PhotometricInterpretation::CFA,
            mosaic_pattern,
            as_shot_neutral: [0_f64; 4],
            matrices: [vec![], vec![]],
            linearization_table: None,
        }
    }

    pub fn new_tiled(
        width: u32,
        height: u32,
        bpc: u16,
        data_type: DataType,
        data: Vec<Vec<u8>>,
        tile_size: (u32, u32),
        mosaic_pattern: Pattern,
    ) -> Self {
        RawImage {
            width,
            height,
            bpc,
            data_type,
            data: Data::Tiled((data, tile_size)),
            active_area: None,
            whites: [0, 0, 0, 0],
            blacks: [0, 0, 0, 0],
            compression: tiff::Compression::Unknown,
            photom_int: exif::PhotometricInterpretation::CFA,
            mosaic_pattern,
            as_shot_neutral: [0_f64; 4],
            matrices: [vec![], vec![]],
            linearization_table: None,
        }
    }

    /// New `RawImage` with 16 bit data.
    pub fn with_data16(
        width: u32,
        height: u32,
        bpc: u16,
        data_type: DataType,
        data: Vec<u16>,
        mosaic_pattern: Pattern,
    ) -> Self {
        RawImage {
            width,
            height,
            bpc,
            data_type,
            data: Data::Data16(data),
            active_area: None,
            whites: [0, 0, 0, 0],
            blacks: [0, 0, 0, 0],
            compression: tiff::Compression::Unknown,
            photom_int: exif::PhotometricInterpretation::CFA,
            mosaic_pattern,
            as_shot_neutral: [0_f64; 4],
            matrices: [vec![], vec![]],
            linearization_table: None,
        }
    }

    pub(crate) fn with_image_buffer(
        buffer: ImageBuffer<u16>,
        data_type: DataType,
        mosaic_pattern: Pattern,
    ) -> Self {
        RawImage {
            width: buffer.width,
            height: buffer.height,
            bpc: buffer.bpc,
            data_type,
            data: Data::Data16(buffer.data),
            active_area: None,
            whites: [0, 0, 0, 0],
            blacks: [0, 0, 0, 0],
            compression: tiff::Compression::Unknown,
            photom_int: exif::PhotometricInterpretation::CFA,
            mosaic_pattern,
            as_shot_neutral: [0_f64; 4],
            matrices: [vec![], vec![]],
            linearization_table: None,
        }
    }

    /// Reset the buffer from an `ImageBuffer<u16>`.
    /// This is usefull when decompressing.
    pub(crate) fn set_with_buffer(&mut self, buffer: ImageBuffer<u16>) {
        self.width = buffer.width;
        self.height = buffer.height;
        self.bpc = buffer.bpc;
        self.data = Data::Data16(buffer.data);
    }

    /// Get the linearization table if there is one.
    pub fn linearization_table(&self) -> Option<&Vec<u16>> {
        self.linearization_table.as_ref()
    }

    pub(crate) fn set_linearization_table(&mut self, table: Option<Vec<u16>>) {
        self.linearization_table = table;
    }

    /// The sensor active area.
    pub fn active_area(&self) -> Option<&Rect> {
        self.active_area.as_ref()
    }

    /// Set the sensor active area.
    pub fn set_active_area(&mut self, rect: Option<Rect>) {
        self.active_area = rect;
    }

    /// Retrieve the White balance as RGBx multiplier values.
    ///
    /// Usually on RGB raw data `x` will be NAN. These multipliers are
    /// usually normalized around a 1.0 multiplier value for Green.
    /// For a white balanced RGB image, returns `[1.0, 1.0, 1.0, NAN]`
    pub fn as_shot_neutral(&self) -> &[f64] {
        &self.as_shot_neutral
    }

    /// Set the white balance.
    ///
    /// Currently only 3 RGB component is supported.
    pub fn set_as_shot_neutral(&mut self, as_shot: &[f64]) {
        self.as_shot_neutral[0] = as_shot[0];
        self.as_shot_neutral[1] = as_shot[1];
        self.as_shot_neutral[2] = as_shot[2];
        self.as_shot_neutral[3] = f64::NAN;
    }

    /// Set the width of the Rawdata. Use with caution.
    pub fn set_width(&mut self, width: u32) {
        self.width = width;
    }

    /// Black values
    pub fn blacks(&self) -> &[u16; 4] {
        &self.blacks
    }

    pub fn set_blacks(&mut self, b: [u16; 4]) {
        self.blacks = b;
    }

    pub fn whites(&self) -> &[u16; 4] {
        &self.whites
    }

    pub fn set_whites(&mut self, w: [u16; 4]) {
        self.whites = w;
    }

    pub fn set_data_type(&mut self, data_type: DataType) {
        self.data_type = data_type
    }

    pub fn photometric_interpretation(&self) -> exif::PhotometricInterpretation {
        self.photom_int
    }

    pub fn set_photometric_interpretation(&mut self, photom_int: exif::PhotometricInterpretation) {
        self.photom_int = photom_int;
    }

    pub fn compression(&self) -> tiff::Compression {
        self.compression
    }

    pub fn set_compression(&mut self, compression: tiff::Compression) {
        self.compression = compression;
    }

    pub fn set_bpc(&mut self, bpc: u16) {
        self.bpc = bpc;
    }

    pub fn colour_matrix(&self, index: usize) -> Option<&[f64]> {
        if index == 1 || index == 2 {
            return Some(&self.matrices[index - 1]);
        }
        None
    }

    pub fn set_colour_matrix(&mut self, index: usize, m: &[f64]) {
        if index == 1 || index == 2 {
            self.matrices[index - 1] = m.to_vec();
        }
    }

    pub fn set_data16(&mut self, data: Vec<u16>) {
        self.data = Data::Data16(data)
    }

    /// Provide the 16bits data as a u8 slice.
    /// Use with caution
    pub fn data16_as_u8(&self) -> Option<&[u8]> {
        match self.data {
            Data::Data16(ref d) => Some(utils::to_u8_slice(d)),
            _ => None,
        }
    }

    pub fn tile_data(&self) -> Option<&[Vec<u8>]> {
        match self.data {
            Data::Tiled(ref d) => Some(&d.0),
            _ => None,
        }
    }

    pub fn tile_size(&self) -> Option<(u32, u32)> {
        match self.data {
            Data::Tiled(ref d) => Some(d.1),
            _ => None,
        }
    }

    pub fn replace_data(mut self, data: Vec<u16>) -> RawImage {
        self.data = Data::Data16(data);

        self
    }

    /// Set the mosaic pattern.
    pub fn set_mosaic_pattern(&mut self, pattern: Pattern) {
        self.mosaic_pattern = pattern;
    }

    /// Return the mosaic pattern for the RAW data.
    pub fn mosaic_pattern(&self) -> &Pattern {
        &self.mosaic_pattern
    }

    /// Simple linearize based on black and white
    ///
    /// Linearization will:
    /// - lookup the linearization table to directly map indexed values:
    ///   this is notable on Leica M8 files.
    /// - scale by range / white
    ///
    fn linearize(&self, mut buffer: ImageBuffer<u16>) -> ImageBuffer<u16> {
        log::debug!("linearize");
        // XXX fix this to use the 4 component
        let white = self.whites()[0];
        let black = self.blacks()[0];
        let range = white - black;
        let scale = range as f64 / white as f64;
        let table = if self
            .linearization_table
            .as_ref()
            .map(|t| t.len() == (1 << self.bpc()))
            .unwrap_or(false)
        {
            self.linearization_table.as_ref()
        } else {
            None
        };
        buffer.data.iter_mut().for_each(|v| {
            *v = (table.map(|t| t[*v as usize] as f64).unwrap_or(*v as f64) * scale).round() as u16
        });

        buffer
    }

    /// Interplate the image buffer. Return a new buffer if successful.
    fn interpolate(&self, buffer: &ImageBuffer<u16>) -> Result<ImageBuffer<u16>> {
        let pattern = self.mosaic_pattern();
        let x = self.width();
        let y = self.height();
        match self.photom_int {
            exif::PhotometricInterpretation::CFA => {
                let mut out_x = 0_u32;
                let mut out_y = 0_u32;
                let mut data = vec![0_u16; (3 * x * y) as usize];
                let err = unsafe {
                    render::bimedian_demosaic(
                        buffer.data.as_ptr(),
                        x,
                        y,
                        pattern.into(),
                        data.as_mut_ptr(),
                        &mut out_x as *mut u32,
                        &mut out_y as *mut u32,
                    )
                };
                if err != or_error::NONE {
                    Err(err.into())
                } else {
                    // This is necessary to have a consistent size with the output.
                    // Notably, the `image` crate doesn't like it.
                    // The assumption is that the resize should shrink the buffer.
                    data.resize((3 * out_x * out_y) as usize, 0);
                    Ok(ImageBuffer::with_data(data, out_x, out_y, 16))
                }
            }
            exif::PhotometricInterpretation::LinearRaw => {
                let mut data = vec![0_u16; (3 * x * y) as usize];
                let err = unsafe {
                    render::grayscale_to_rgb(buffer.data.as_ptr(), x, y, data.as_mut_ptr())
                };
                if err != or_error::NONE {
                    Err(err.into())
                } else {
                    Ok(ImageBuffer::with_data(data, x, y, 16))
                }
            }
            _ => Err(Error::InvalidFormat),
        }
    }

    /// Render the image using `options`. See `[render::RenderingOptions]`
    /// May return `Error::Unimplemented`.
    pub fn rendered_image(&self, options: RenderingOptions) -> Result<RawImage> {
        // XXX fix to properly handle the Raw stage.
        if options.stage == RenderingStage::Raw || options.stage == RenderingStage::Colour {
            return Err(Error::Unimplemented);
        }
        // XXX remove when the colour stage is implemented
        if options.target != ColourSpace::Camera {
            return Err(Error::Unimplemented);
        }
        if self.data_type() != DataType::Raw {
            return Err(Error::InvalidFormat);
        }
        let x = self.width();
        let y = self.height();
        let mut pattern = self.mosaic_pattern().clone();
        let data16 = ImageBuffer::with_data(
            self.data16().ok_or(Error::InvalidFormat)?.to_vec(),
            x,
            y,
            16,
        );
        let mut data16 = self.linearize(data16);
        if options.stage >= RenderingStage::Interpolation {
            data16 = self.interpolate(&data16)?;
            pattern = Pattern::Empty;
        }

        // XXX make sure to copy over other data from the rawimage.
        let mut image = RawImage::with_image_buffer(data16, DataType::PixmapRgb16, pattern);
        if options.stage >= RenderingStage::Linearization {
            image.set_blacks([0, 0, 0, 0]);
            image.set_whites(*self.whites());
        }

        Ok(image)
    }
}

impl Bitmap for RawImage {
    fn data_type(&self) -> DataType {
        self.data_type
    }

    fn data_size(&self) -> usize {
        match self.data {
            Data::Data8(ref d) => d.len(),
            Data::Data16(ref d) => d.len() * 2,
            Data::Tiled(ref d) => d.0.iter().map(|t| t.len()).sum(),
        }
    }

    fn width(&self) -> u32 {
        self.width
    }
    fn height(&self) -> u32 {
        self.height
    }

    fn bpc(&self) -> u16 {
        self.bpc
    }

    fn data8(&self) -> Option<&[u8]> {
        match self.data {
            Data::Data8(ref d) => Some(d),
            _ => None,
        }
    }
}

impl Image for RawImage {
    fn data16(&self) -> Option<&[u16]> {
        match self.data {
            Data::Data16(ref d) => Some(d),
            _ => None,
        }
    }
}
