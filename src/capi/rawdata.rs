// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - capi/rawdata.rs
 *
 * Copyright (C) 2022-2024 Hubert Figuière
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

use super::{or_cfa_pattern, or_data_type, or_error, ORMosaicInfoRef};
use crate::{
    colour::ColourSpace,
    or_unwrap,
    render::{RenderingOptions, RenderingStage},
    AspectRatio, Bitmap, Image, RawImage,
};

/// Pointer to a [`RawImage`] object exported to the C API.
pub type ORRawDataRef = *mut RawImage;

#[allow(dead_code)]
/// The rendering options const for the C API.
pub(crate) mod or_rendering_options {
    use crate::{ColourSpace, RenderingStage};

    /// The mask for the target coulour space. 16 possible values.
    pub const OR_RENDERING_TARGET_CS_MASK: u32 = 0x0000000f;
    /// Keep the camera colour space (unadjusted demosaic) (default).
    pub const OR_RENDERING_TARGET_CAMERA_CS: u32 = ColourSpace::Camera as u32;
    /// Render to XYZ colour space.
    pub const OR_RENDERING_TARGET_XYZ_CS: u32 = ColourSpace::XYZ as u32;
    /// Render to sRGB colour space.
    pub const OR_RENDERING_TARGET_SRGB_CS: u32 = ColourSpace::SRgb as u32;

    /// The mask for the stage
    pub const OR_RENDERING_STAGE_MASK: u32 = 0x00000030;
    /// The number of bits to shift in or out.
    pub const OR_RENDERING_STAGE_BIT_SHIFT: u32 = 4;
    /// Raw stage
    pub const OR_RENDERING_STAGE_RAW: u32 =
        (RenderingStage::Raw as u32) << OR_RENDERING_STAGE_BIT_SHIFT;
    /// Linearization stage
    pub const OR_RENDERING_STAGE_LINEAR: u32 =
        (RenderingStage::Linearization as u32) << OR_RENDERING_STAGE_BIT_SHIFT;
    /// Interpolation stage
    pub const OR_RENDERING_STAGE_INTERP: u32 =
        (RenderingStage::Interpolation as u32) << OR_RENDERING_STAGE_BIT_SHIFT;
    /// Colour stage
    pub const OR_RENDERING_STAGE_COLOUR: u32 =
        (RenderingStage::Colour as u32) << OR_RENDERING_STAGE_BIT_SHIFT;

    /// Default is SRgb, Interpolation stage.
    pub const OR_RENDERING_OPTIONS_DEFAULT: u32 =
        OR_RENDERING_TARGET_SRGB_CS + OR_RENDERING_STAGE_INTERP;
}

impl From<u32> for RenderingOptions {
    fn from(value: u32) -> RenderingOptions {
        use or_rendering_options::*;

        let mut options = RenderingOptions::default();
        if let Ok(cs) = ColourSpace::try_from_primitive(value & OR_RENDERING_TARGET_CS_MASK) {
            options = options.with_target(cs);
        }
        if let Ok(stage) = RenderingStage::try_from_primitive(
            (value & OR_RENDERING_STAGE_MASK) >> OR_RENDERING_STAGE_BIT_SHIFT,
        ) {
            options = options.with_stage(stage);
        }

        options
    }
}

#[no_mangle]
/// Create a new `RawDataRef`
///
/// Will return an `ORRawDataref` that will need to be freed with
/// `or_rawdata_release`.
extern "C" fn or_rawdata_new() -> ORRawDataRef {
    Box::into_raw(Box::new(RawImage::new()))
}

#[no_mangle]
/// Release `rawdata` of type [`ORRawDataRef`], and return an error code.
///
/// If `rawdata` is `null`, then [`NOT_AREF`][or_error::NOT_AREF] is returned,
/// otherwise [`NONE`][or_error::NONE].
/// Passing an invalid ref (not null) or of the wrong type is an error
/// and will cause undefined behaviour.
extern "C" fn or_rawdata_release(rawdata: ORRawDataRef) -> or_error {
    if !rawdata.is_null() {
        unsafe { drop(Box::from_raw(rawdata)) };
        return or_error::NONE;
    }
    or_error::NOT_AREF
}

#[no_mangle]
/// Return the size in bytes of the raw data.
extern "C" fn or_rawdata_data_size(rawdata: ORRawDataRef) -> libc::size_t {
    or_unwrap!(rawdata, 0, rawdata.data_size())
}

#[no_mangle]
extern "C" fn or_rawdata_data(rawdata: ORRawDataRef) -> *const libc::c_void {
    or_unwrap!(
        rawdata,
        std::ptr::null(),
        rawdata
            .data8()
            .map(|data| data.as_ptr() as *const libc::c_void)
            .or_else(|| {
                rawdata
                    .data16()
                    .map(|data| data.as_ptr() as *const libc::c_void)
            })
            .unwrap_or_else(std::ptr::null)
    )
}

#[no_mangle]
/// Return the format of the raw data.
extern "C" fn or_rawdata_format(rawdata: ORRawDataRef) -> or_data_type {
    or_unwrap!(rawdata, or_data_type::UNKNOWN, rawdata.data_type().into())
}

#[no_mangle]
/// Get the black and white.
extern "C" fn or_rawdata_levels(
    rawdata: ORRawDataRef,
    black: *mut u16,
    white: *mut u16,
) -> or_error {
    or_unwrap!(rawdata, or_error::NOT_AREF, {
        if !black.is_null() {
            unsafe {
                black.copy_from_nonoverlapping(rawdata.blacks().as_ptr(), 4);
            }
        }
        if !white.is_null() {
            unsafe {
                white.copy_from_nonoverlapping(rawdata.whites().as_ptr(), 4);
            }
        }
        or_error::NONE
    })
}

#[no_mangle]
// As shot neutral white balance.
extern "C" fn or_rawdata_as_shot_neutral(rawdata: ORRawDataRef, wb: *mut f64) -> or_error {
    or_unwrap!(rawdata, or_error::NOT_AREF, {
        let rwb = rawdata.as_shot_neutral();
        unsafe {
            wb.copy_from_nonoverlapping(rwb.as_ptr(), 4);
        }
        or_error::NONE
    })
}

#[no_mangle]
extern "C" fn or_rawdata_dimensions(rawdata: ORRawDataRef, x: *mut u32, y: *mut u32) {
    or_unwrap!(rawdata, (), {
        if !x.is_null() {
            unsafe { *x = rawdata.width() };
        }
        if !y.is_null() {
            unsafe { *y = rawdata.height() };
        }
    })
}

#[no_mangle]
extern "C" fn or_rawdata_get_cfa_pattern_type(rawdata: ORRawDataRef) -> or_cfa_pattern {
    or_unwrap!(
        rawdata,
        or_cfa_pattern::NONE,
        rawdata.mosaic_pattern().into()
    )
}

#[no_mangle]
extern "C" fn or_rawdata_get_compression(rawdata: ORRawDataRef) -> u32 {
    or_unwrap!(rawdata, 0, rawdata.compression() as u32)
}

#[no_mangle]
extern "C" fn or_rawdata_bpc(rawdata: ORRawDataRef) -> u32 {
    or_unwrap!(rawdata, 0, rawdata.bpc() as u32)
}

#[no_mangle]
extern "C" fn or_rawdata_get_colour_matrix(
    rawdata: ORRawDataRef,
    index: u32,
    size: *mut u32,
) -> *const std::ffi::c_double {
    or_unwrap!(
        rawdata,
        std::ptr::null(),
        if let Some(matrix) = rawdata.colour_matrix(index as usize) {
            if !size.is_null() {
                unsafe { *size = matrix.matrix.len() as u32 };
            }
            matrix.matrix.as_ptr()
        } else {
            std::ptr::null()
        }
    )
}

#[no_mangle]
extern "C" fn or_rawdata_get_active_area(
    rawdata: ORRawDataRef,
    x: *mut u32,
    y: *mut u32,
    width: *mut u32,
    height: *mut u32,
) -> or_error {
    if !rawdata.is_null() {
        let rawdata = unsafe { &*rawdata };
        let active_area = rawdata.active_area();
        if active_area.is_none() {
            // XXX Not sure what we expect here
            return or_error::NOT_FOUND;
        }
        let active_area = active_area.unwrap();
        if !x.is_null() {
            unsafe { *x = active_area.x };
        }
        if !y.is_null() {
            unsafe { *y = active_area.y };
        }
        if !width.is_null() {
            unsafe { *width = active_area.width };
        }
        if !height.is_null() {
            unsafe { *height = active_area.height };
        }

        or_error::NONE
    } else {
        or_error::NOT_AREF
    }
}

#[no_mangle]
extern "C" fn or_rawdata_get_user_crop(
    rawdata: ORRawDataRef,
    x: *mut u32,
    y: *mut u32,
    width: *mut u32,
    height: *mut u32,
) -> or_error {
    if !rawdata.is_null() {
        let rawdata = unsafe { &*rawdata };
        let user_crop = rawdata.user_crop();
        if user_crop.is_none() {
            // XXX Not sure what we expect here
            return or_error::NOT_FOUND;
        }
        let user_crop = user_crop.unwrap();
        if !x.is_null() {
            unsafe { *x = user_crop.x };
        }
        if !y.is_null() {
            unsafe { *y = user_crop.y };
        }
        if !width.is_null() {
            unsafe { *width = user_crop.width };
        }
        if !height.is_null() {
            unsafe { *height = user_crop.height };
        }

        or_error::NONE
    } else {
        or_error::NOT_AREF
    }
}

#[no_mangle]
extern "C" fn or_rawdata_get_user_aspect_ratio(
    rawdata: ORRawDataRef,
    width: *mut u32,
    height: *mut u32,
) -> or_error {
    if !rawdata.is_null() {
        let rawdata = unsafe { &*rawdata };
        if let Some(AspectRatio(w, h)) = rawdata.user_aspect_ratio() {
            if !width.is_null() {
                unsafe { *width = w };
            }
            if !height.is_null() {
                unsafe { *height = h };
            }
            or_error::NONE
        } else {
            or_error::NOT_FOUND
        }
    } else {
        or_error::NOT_AREF
    }
}

#[no_mangle]
extern "C" fn or_rawdata_get_mosaicinfo(rawdata: ORRawDataRef) -> ORMosaicInfoRef {
    if !rawdata.is_null() {
        let rawdata = unsafe { &*rawdata };

        rawdata.mosaic_pattern() as ORMosaicInfoRef
    } else {
        std::ptr::null_mut()
    }
}
