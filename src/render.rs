// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - render.rs
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

use crate::capi::{or_cfa_pattern, or_error};

extern "C" {
    pub fn bimedian_demosaic(
        input: *const u16,
        x: u32,
        y: u32,
        pattern: or_cfa_pattern,
        out: *mut u16,
        out_x: *mut u32,
        out_y: *mut u32,
    ) -> or_error;

    pub fn grayscale_to_rgb(input: *const u16, x: u32, y: u32, out: *mut u16) -> or_error;
}
