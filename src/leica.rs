// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - leica.rs
 *
 * Copyright (C) 2022-2023 Hubert Figuière
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

//! Leica camera support

use crate::tiff;

#[macro_export]
macro_rules! leica {
    ($id:expr, $model:ident) => {
        (
            $id,
            TypeId(
                $crate::camera_ids::vendor::LEICA,
                $crate::camera_ids::leica::$model,
            ),
        )
    };
    ($model:ident) => {
        TypeId(
            $crate::camera_ids::vendor::LEICA,
            $crate::camera_ids::leica::$model,
        )
    };
}

pub use tiff::exif::generated::MNOTE_LEICA2_TAG_NAMES as MNOTE_TAG_NAMES_2;
pub use tiff::exif::generated::MNOTE_LEICA4_TAG_NAMES as MNOTE_TAG_NAMES_4;
pub use tiff::exif::generated::MNOTE_LEICA5_TAG_NAMES as MNOTE_TAG_NAMES_5;
pub use tiff::exif::generated::MNOTE_LEICA6_TAG_NAMES as MNOTE_TAG_NAMES_6;
pub use tiff::exif::generated::MNOTE_LEICA9_TAG_NAMES as MNOTE_TAG_NAMES_9;
