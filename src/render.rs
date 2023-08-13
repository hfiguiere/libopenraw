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

use crate::bitmap::ImageBuffer;
use crate::capi::or_error;
use crate::colour::ColourSpace;
use crate::mosaic::Pattern;

#[repr(u32)]
#[derive(Copy, Clone, Debug, Default, Eq, PartialEq, PartialOrd, TryFromPrimitive)]
/// Rendering stage. The values are also the order in the pipeline.
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

pub(crate) fn bimedian_demosaic(
    input: &ImageBuffer<u16>,
    pattern: &Pattern,
) -> crate::Result<ImageBuffer<u16>> {
    let mut out_x = 0_u32;
    let mut out_y = 0_u32;
    let mut data = vec![0_u16; (3 * input.width * input.height) as usize];
    let err = unsafe {
        ffi::bimedian_demosaic(
            input.data.as_ptr(),
            input.width,
            input.height,
            pattern.into(),
            data.as_mut_ptr(),
            &mut out_x as *mut u32,
            &mut out_y as *mut u32,
        )
    };
    if err != or_error::NONE {
        Err(err.into())
    } else {
        // This is necessary to have a consistent size with the output.
        // Notably, the `image` crate doesn't like it.
        // The assumption is that the resize should shrink the buffer.
        data.resize((3 * out_x * out_y) as usize, 0);
        Ok(ImageBuffer::with_data(data, out_x, out_y, 16))
    }
}

pub(crate) mod ffi {
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
}
