// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - capi/mosaic.rs
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

use crate::mosaic::Pattern;
use crate::or_unwrap;

use super::or_cfa_pattern;

/// A mosaic pattern.
pub type ORMosaicInfoRef = *const Pattern;

#[no_mangle]
/// Get the pattern type.
extern "C" fn or_mosaicinfo_get_type(mosaic_info: ORMosaicInfoRef) -> or_cfa_pattern {
    or_unwrap!(mosaic_info, or_cfa_pattern::NONE, mosaic_info.into())
}

#[no_mangle]
/// Get the pattern. Currently unimplemented.
extern "C" fn or_mosaicinfo_get_pattern(_mosaic_info: ORMosaicInfoRef) -> *const u8 {
    // XXX todo
    unreachable!();
}
