// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - rawdata.rs
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

//! RAW data

use super::{Bitmap, DataType, Rect};
use crate::tiff;
use crate::tiff::exif;
use crate::utils;

/// Encapsulate data 8 or 16 bits
enum Data {
    Data8(Vec<u8>),
    Data16(Vec<u16>),
    Tiled((Vec<Vec<u8>>, (u32, u32))),
}

/// RAW Data extracted from the file.
pub struct RawData {
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
    white: u16,
    /// Black point
    black: u16,
    ///
    photom_int: exif::PhotometricInterpretation,
    ///
    compression: tiff::Compression,
    /// Sensor active area
    active_area: Option<Rect>,
}

impl RawData {
    /// New `RawData` with 8 bit data.
    pub fn new8(width: u32, height: u32, bpc: u16, data_type: DataType, data: Vec<u8>) -> Self {
        RawData {
            width,
            height,
            bpc,
            data_type,
            data: Data::Data8(data),
            active_area: None,
            white: 0,
            black: 0,
            compression: tiff::Compression::Unknown,
            photom_int: exif::PhotometricInterpretation::CFA,
        }
    }

    pub fn new_tiled(
        width: u32,
        height: u32,
        bpc: u16,
        data_type: DataType,
        data: Vec<Vec<u8>>,
        tile_size: (u32, u32),
    ) -> Self {
        RawData {
            width,
            height,
            bpc,
            data_type,
            data: Data::Tiled((data, tile_size)),
            active_area: None,
            white: 0,
            black: 0,
            compression: tiff::Compression::Unknown,
            photom_int: exif::PhotometricInterpretation::CFA,
        }
    }

    /// New `RawData` with 16 bit data.
    pub fn new16(width: u32, height: u32, bpc: u16, data_type: DataType, data: Vec<u16>) -> Self {
        RawData {
            width,
            height,
            bpc,
            data_type,
            data: Data::Data16(data),
            active_area: None,
            white: 0,
            black: 0,
            compression: tiff::Compression::Unknown,
            photom_int: exif::PhotometricInterpretation::CFA,
        }
    }

    /// The sensor active area.
    pub fn active_area(&self) -> Option<&Rect> {
        self.active_area.as_ref()
    }

    /// Set the sensor active area.
    pub fn set_active_area(&mut self, rect: Option<Rect>) {
        self.active_area = rect;
    }

    /// Set the width of the Rawdata. Use with caution.
    pub fn set_width(&mut self, width: u32) {
        self.width = width;
    }

    /// Black value
    pub fn black(&self) -> u16 {
        self.black
    }

    pub fn set_black(&mut self, b: u16) {
        self.black = b;
    }

    pub fn white(&self) -> u16 {
        self.white
    }

    pub fn set_white(&mut self, w: u16) {
        self.white = w;
    }

    pub fn set_data_type(&mut self, data_type: DataType) {
        self.data_type = data_type
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

    pub fn replace_data(mut self, data: Vec<u16>) -> RawData {
        self.data = Data::Data16(data);

        self
    }
}

impl Bitmap for RawData {
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

    fn data16(&self) -> Option<&[u16]> {
        match self.data {
            Data::Data16(ref d) => Some(d),
            _ => None,
        }
    }
}
