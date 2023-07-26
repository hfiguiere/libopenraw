// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - thumbnail.rs
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

//! Representation of thumbnails

use crate::bitmap;
use crate::bitmap::Bitmap;
use crate::DataType;

/// Offset/len representation for `Data`
pub struct DataOffset {
    /// Offset in the container
    pub offset: u64,
    /// Data size
    pub len: u64,
}

/// Represent data either as offset/len or a buffer.
pub enum Data {
    Offset(DataOffset),
    Bytes(Vec<u8>),
}

impl Data {
    pub fn len(&self) -> usize {
        match *self {
            Self::Offset(ref offset) => offset.len as usize,
            Self::Bytes(ref v) => v.len(),
        }
    }
}

/// Describe a thumbnail to fetch it from the container later
/// as a blob
pub struct ThumbDesc {
    /// Thumbnail width
    pub width: u32,
    /// Thumbnail height
    pub height: u32,
    /// Type if the data
    pub data_type: DataType,
    /// The data
    pub data: Data,
}

impl ThumbDesc {
    pub fn data_size(&self) -> u64 {
        match self.data {
            Data::Offset(ref offset) => offset.len,
            Data::Bytes(ref v) => v.len() as u64,
        }
    }
}

/// A thumbnail
#[derive(Default, Debug)]
pub struct Thumbnail {
    /// Thumbnail width
    width: u32,
    /// Thumbnail height
    height: u32,
    /// Type of the data
    data_type: DataType,
    data: bitmap::Data,
}

impl Thumbnail {
    pub fn new() -> Thumbnail {
        Thumbnail::default()
    }

    /// New thumbnail with data.
    pub fn with_data(width: u32, height: u32, data_type: DataType, data: Vec<u8>) -> Thumbnail {
        Thumbnail {
            width,
            height,
            data_type,
            data: bitmap::Data::Data8(data),
        }
    }

    /// New thumbnail with data16.
    pub fn with_data16(width: u32, height: u32, data_type: DataType, data: Vec<u16>) -> Thumbnail {
        Thumbnail {
            width,
            height,
            data_type,
            data: bitmap::Data::Data16(data),
        }
    }
}

impl Bitmap for Thumbnail {
    fn data_type(&self) -> DataType {
        self.data_type
    }

    fn data_size(&self) -> usize {
        use bitmap::Data;
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
        8
    }

    fn data8(&self) -> Option<&[u8]> {
        use bitmap::Data;
        match self.data {
            Data::Data8(ref d) => Some(d),
            _ => None,
        }
    }

    fn data16(&self) -> Option<&[u16]> {
        use bitmap::Data;
        match self.data {
            Data::Data16(ref d) => Some(d),
            _ => None,
        }
    }
}
