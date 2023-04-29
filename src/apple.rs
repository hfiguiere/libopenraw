// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - apple.rs
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

//! Apple camera support.

use std::collections::HashMap;

lazy_static::lazy_static! {
    pub static ref MNOTE_TAG_NAMES: HashMap<u16, &'static str> = HashMap::from([
        (0x3, "RunTime"),
        (0x8, "AccelerationVector"),
        (0xa, "HDRImageType"),
        (0xb, "BurstUUID"),
        (0xc, "FocusDistanceRange"),
        (0x11, "ContentIdentifier"),
        (0x15, "ImageUniqueID"),
        (0x17, "LivePhotoVideoIndex"),
    ]);
}
