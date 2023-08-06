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

use num_enum::TryFromPrimitive;

use crate::{
    capi::{or_cfa_pattern, or_error},
    colour::ColourSpace,
};

#[repr(u32)]
#[derive(Copy, Clone, Debug, Default, Eq, PartialEq, TryFromPrimitive)]
/// Rendering stage
pub enum RenderingStage {
    /// The raw data from the file
    Raw = 0,
    /// The linearize raw data
    Linearization = 1,
    #[default]
    /// Interpolated (demosaic)
    Interpolation = 2,
    /// Colour corrected (from camera to target `ColourSpace`)
    Colour = 3,
}

#[derive(Clone)]
/// RenderingOptions
///
/// ```rust
/// use libopenraw::{ColourSpace, RenderingOptions, RenderingStage};
///
/// let options = RenderingOptions::default()
///     .with_stage(RenderingStage::Interpolation)
///     .with_target(ColourSpace::SRgb);
/// ```
pub struct RenderingOptions {
    /// The stage of rendering requested.
    pub stage: RenderingStage,
    /// The colour space target for `RenderingStage::Colour`
    pub target: ColourSpace,
}

impl Default for RenderingOptions {
    fn default() -> Self {
        RenderingOptions {
            stage: RenderingStage::Raw,
            target: ColourSpace::Camera,
        }
    }
}

impl RenderingOptions {
    /// Set the target colour space.
    pub fn with_target(mut self, colour_space: ColourSpace) -> Self {
        self.target = colour_space;
        self
    }

    /// Set the rendering stage.
    pub fn with_stage(mut self, stage: RenderingStage) -> Self {
        self.stage = stage;
        self
    }
}

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
