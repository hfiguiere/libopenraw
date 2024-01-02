// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - rawimage.rs
 *
 * Copyright (C) 2022-2024 Hubert Figuière
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

use nalgebra::{matrix, Matrix3, Vector3};

use super::{Bitmap, DataType, Error, Image, Rect, Result};
use crate::bitmap::{Data, ImageBuffer};
use crate::mosaic::Pattern;
use crate::render::{self, gamma_correct_f, gamma_correct_srgb, RenderingOptions, RenderingStage};
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
            let matrix = &self.matrices[index - 1];
            return if matrix.is_empty() {
                None
            } else {
                Some(matrix)
            };
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
    fn linearize(&self, buffer: ImageBuffer<u16>) -> ImageBuffer<f64> {
        log::debug!("linearize");
        // XXX fix this to use the 4 component
        let white = self.whites()[0];
        let black = self.blacks()[0];
        let range = (white - black) as f64;
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
        log::debug!("pre-lin at 1000, 1000: {:?}", buffer.pixel_at(1000, 1000));
        let data = buffer
            .data
            .iter()
            .map(|v| {
                table.map(|t| t[*v as usize] as f64).unwrap_or_else(|| {
                    if *v < black {
                        0.0
                    } else {
                        (*v - black) as f64
                    }
                }) / range
            })
            .collect();

        let buffer =
            ImageBuffer::<f64>::with_data(data, buffer.width, buffer.height, buffer.bpc, 1);
        log::debug!("post-lin at 1000, 1000: {:?}", buffer.pixel_at(1000, 1000));
        buffer
    }

    /// Interplate the image buffer. Return a new buffer if successful.
    fn interpolate(&self, buffer: ImageBuffer<f64>) -> Result<ImageBuffer<f64>> {
        let pattern = self.mosaic_pattern();
        match self.photom_int {
            exif::PhotometricInterpretation::CFA => render::demosaic::bimedian(&buffer, pattern),
            exif::PhotometricInterpretation::LinearRaw => render::grayscale::to_rgb(&buffer),
            _ => {
                log::error!("Invalid photometric interpretation {:?}", self.photom_int);
                Err(Error::InvalidFormat)
            }
        }
    }

    /// Calculate the camera to RGB colour matrix using `cm`
    ///
    /// Currently analog balance and camera calibration are identity matrices
    pub fn calculate_cam_rgb(cm: &Matrix3<f64>) -> Matrix3<f64> {
        // Camera calibration
        let cc = Matrix3::<f64>::identity();
        // Analog  balance
        let ab = Matrix3::<f64>::identity();
        let xyz_camera = ab * cc * cm;
        let cam_xyz = xyz_camera.try_inverse().unwrap();
        // XYZ to RGB <https://en.wikipedia.org/wiki/SRGB#From_CIE_XYZ_to_sRGB>
        let xyz_rgb =
            matrix![ 3.2406, -1.5372, -0.4986; -0.9689, 1.8758, 0.0415; 0.0557, -0.2040, 1.0570];
        cam_xyz * xyz_rgb
    }

    pub(crate) fn colour_correct(
        &self,
        mut buffer: ImageBuffer<f64>,
        target: ColourSpace,
    ) -> Result<ImageBuffer<f64>> {
        if target != ColourSpace::SRgb {
            return Err(Error::Unimplemented);
        }
        let width = buffer.width;
        let height = buffer.height;
        let cm = self.colour_matrix(1).map(Matrix3::from_row_slice);
        if let Some(cm) = cm {
            let cam_rgb = Self::calculate_cam_rgb(&cm);
            let mut out = Vec::with_capacity((height * width) as usize * 3);
            log::debug!("pixel cam at 1000, 1000: {:?}", buffer.pixel_at(1000, 1000));
            for row in 0..height {
                let pos = row * width * 3;
                let mut col = 0;
                while col < width * 3 {
                    let c = (pos + col) as usize;
                    let abc = Vector3::from_row_iterator(buffer.data[c..c + 3].iter().copied());
                    let rgb = cam_rgb * abc;
                    col += 3;
                    out.extend(rgb.iter());
                }
            }
            buffer.data = out;
            log::debug!("pixel rgb at 1000, 1000: {:?}", buffer.pixel_at(1000, 1000));
        } else {
            log::error!("no matrix");
        }

        Ok(buffer)
    }

    /// Render the image using `options`. See `[render::RenderingOptions]`
    /// May return `Error::Unimplemented`.
    pub fn rendered_image(&self, options: RenderingOptions) -> Result<RawImage> {
        // XXX fix to properly handle the Raw stage.
        if options.stage == RenderingStage::Raw {
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
            1,
        );
        let mut data = self.linearize(data16);
        if options.stage >= RenderingStage::Interpolation {
            data = self.interpolate(data)?;
            pattern = Pattern::Empty;
        }

        if options.stage >= RenderingStage::Colour {
            match self.photom_int {
                exif::PhotometricInterpretation::CFA => {
                    data = self.colour_correct(data, options.target)?;
                    data.data
                        .iter_mut()
                        .for_each(|v| *v = gamma_correct_srgb(*v));
                }
                exif::PhotometricInterpretation::LinearRaw => {
                    data.data
                        .iter_mut()
                        .for_each(|v| *v = gamma_correct_f::<22>(*v));
                }
                _ => {}
            }
        }

        log::debug!(
            "pixel rgb(float) at 1000, 1000: {:?}",
            data.pixel_at(1000, 1000)
        );

        // XXX make sure to copy over other data from the rawimage.
        let data16 = data.into_u16();
        log::debug!(
            "pixel rgb(u16) at 1000, 1000: {:?}",
            data16.pixel_at(1000, 1000)
        );
        let mut image = RawImage::with_image_buffer(data16, DataType::PixmapRgb16, pattern);
        if options.stage >= RenderingStage::Linearization {
            image.set_blacks([0, 0, 0, 0]);
            image.set_whites([u16::MAX; 4]);
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
