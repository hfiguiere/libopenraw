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

use super::{Bitmap, DataType, Rect};

/// Encapsulate data 8 or 16 bits
enum Data {
    Data8(Vec<u8>),
    Data16(Vec<u16>),
}

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

    /// Active area
    active_area: Option<Rect>,
}

impl RawData {
    pub fn new8(width: u32, height: u32, bpc: u16, data_type: DataType, data: Vec<u8>) -> Self {
        RawData {
            width,
            height,
            bpc,
            data_type,
            data: Data::Data8(data),
            active_area: None,
        }
    }

    pub fn new16(width: u32, height: u32, bpc: u16, data_type: DataType, data: Vec<u16>) -> Self {
        RawData {
            width,
            height,
            bpc,
            data_type,
            data: Data::Data16(data),
            active_area: None,
        }
    }

    pub fn active_area(&self) -> Option<&Rect> {
        self.active_area.as_ref()
    }

    pub fn set_active_area(&mut self, rect: Option<Rect>) {
        self.active_area = rect;
    }
}

impl Bitmap for RawData {
    fn data_type(&self) -> DataType {
        self.data_type
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
