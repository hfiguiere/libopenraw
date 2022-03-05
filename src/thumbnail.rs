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

pub struct Thumbnail {
    /// Thumbnail width
    pub width: u32,
    /// Thumbnail height
    pub height: u32,
    /// Type if the data
    pub data_type: DataType,
    pub data: Vec<u8>,
}

impl Thumbnail {}
