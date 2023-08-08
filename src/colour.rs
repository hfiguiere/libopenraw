// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - colour.rs
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

//! Everything about colour

mod matrix;

pub use matrix::BuiltinMatrix;
use num_enum::TryFromPrimitive;

#[repr(C)]
#[derive(Debug, Default)]
/// Where the colour matrix comes from.
/// Typically DNG is provided. The others are built-in.
pub enum MatrixOrigin {
    #[default]
    /// Unknown. This usually signify an error.
    Unknown = 0,
    /// Colour matrix in library.
    Builtin = 1,
    /// Colour matrix provided by file.
    Provided = 2,
}

#[repr(u32)]
#[derive(Copy, Clone, Debug, Eq, TryFromPrimitive, PartialEq)]
/// The colour space to use.
pub enum ColourSpace {
    #[num_enum(default)]
    /// Unknown, likely an error
    Unknown = 0,
    /// Camera colour space.
    Camera = 1,
    /// XYZ colour space.
    XYZ = 2,
    /// Standard RGB.
    SRgb = 3,
}
