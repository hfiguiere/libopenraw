/*
 * libopenraw - colour/matrix.rs
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

//! Deal with colour matrixes

use crate::TypeId;

/// Builtin Colour Matrix. This is the static data for
/// hardcoded colour matrices.
pub struct BuiltinMatrix {
    /// The camera for which this is the matrix
    pub camera: TypeId,
    /// Black value
    pub black: u16,
    /// White value
    pub white: u16,
    // 3x3 matrix coefficients
    pub matrix: [f64; 9],
}

impl BuiltinMatrix {
    /// Create a builtin matrix. Matrix is integer in 1/10_000th
    pub fn new(camera: TypeId, black: u16, white: u16, matrix: [i16; 9]) -> Self {
        BuiltinMatrix {
            camera,
            black,
            white,
            matrix: matrix.map(|v| v as f64 / 10_000_f64),
        }
    }
}
