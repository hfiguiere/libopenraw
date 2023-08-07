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

use crate::bitmap::Bitmap;
use crate::DataType;

#[derive(Debug)]
/// Offset/len representation for `Data`
pub struct DataOffset {
    /// Offset in the container
    pub offset: u64,
    /// Data size
    pub len: u64,
}

#[derive(Debug)]
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

#[derive(Debug)]
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
#[derive(Debug)]
pub struct Thumbnail {
    /// Thumbnail width
    width: u32,
    /// Thumbnail height
    height: u32,
    /// Bits per component
    bpc: u16,
    /// Type of the data
    data_type: DataType,
    data: Vec<u8>,
}

impl Default for Thumbnail {
    fn default() -> Thumbnail {
        Thumbnail {
            width: 0,
            height: 0,
            bpc: 8,
            data_type: DataType::default(),
            data: vec![],
        }
    }
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
            bpc: 8,
            data_type,
            data,
        }
    }
}

impl Bitmap for Thumbnail {
    fn data_type(&self) -> DataType {
        self.data_type
    }

    fn data_size(&self) -> usize {
        self.data.len()
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
        Some(&self.data)
    }
}
