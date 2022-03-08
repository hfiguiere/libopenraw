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

use crate::DataType;

/// Trait for bitmap objects.
pub trait Bitmap {
    fn data_type(&self) -> DataType;
    fn width(&self) -> u32;
    fn height(&self) -> u32;
    /// Bits per component
    fn bpc(&self) -> u16;
    /// Image data in 8 bits
    fn data8(&self) -> Option<&[u8]>;
    /// Image data in 16 bits
    fn data16(&self) -> Option<&[u16]>;
}

/// Rectangle struct.
#[derive(Debug)]
pub struct Rect {
    pub x: u32,
    pub y: u32,
    pub width: u32,
    pub height: u32,
}
