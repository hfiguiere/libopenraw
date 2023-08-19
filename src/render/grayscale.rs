// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - grayscale.rs
 *
 * Copyright (C) 2023 Hubert Figuière
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

use crate::bitmap::ImageBuffer;
use crate::{Error, Result};

/// Convert a grayscale buffer to RGB
///
/// It's done naively.
pub(crate) fn to_rgb(buffer: &ImageBuffer<f64>) -> Result<ImageBuffer<f64>> {
    if buffer.cc != 1 {
        return Err(Error::InvalidFormat);
    }
    let width = buffer.width;
    let height = buffer.height;

    let out = buffer
        .data
        .iter()
        .flat_map(|v| [*v, *v, *v])
        .collect::<Vec<f64>>();

    Ok(ImageBuffer::with_data(out, width, height, buffer.bpc, 3))
}
