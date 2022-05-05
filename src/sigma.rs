// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - sigma.rs
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

//! Sigma camera support.

use std::collections::HashMap;

lazy_static::lazy_static! {
    pub static ref MNOTE_TAG_NAMES: HashMap<u16, &'static str> = HashMap::from([
        (0x2, "SerialNumber"),
        (0x3, "DriveMode"),
        (0x4, "ResolutionMode"),
        (0x5, "AFMode"),
        (0x6, "FocusSetting"),
        (0x7, "WhiteBalance"),
        (0x8, "ExposureMode"),
        (0x9, "MeteringMode"),
        (0xa, "LensFocalRange"),
        (0xb, "ColorSpace"),
        (0xc, "ExposureCompensation"),
        (0xd, "Contrast"),
        (0xe, "Shadow"),
        (0xf, "Highlight"),
        (0x10, "Saturation"),
        (0x11, "Sharpness"),
        (0x12, "X3FillLight"),
        (0x14, "ColorAdjustment"),
        (0x15, "AdjustmentMode"),
        (0x16, "Quality"),
        (0x17, "Firmware"),
        (0x18, "Software"),
        (0x19, "AutoBracket"),
        (0x1a, "PreviewImageStart"),
        (0x1b, "PreviewImageLength"),
        (0x1c, "PreviewImageSize"),
        (0x1d, "MakerNoteVersion"),
        (0x1e, "PreviewImageSize"),
        (0x1f, "AFPoint"),
        (0x22, "FileFormat"),
        (0x24, "Calibration"),
        (0x26, "FileFormat"),
        (0x27, "LensType"),
        (0x2a, "LensFocalRange"),
        (0x2b, "LensMaxApertureRange"),
        (0x2c, "ColorMode"),
        (0x30, "LensApertureRange"),
        (0x31, "FNumber"),
        (0x32, "ExposureTime"),
        (0x33, "ExposureTime2"),
        (0x34, "BurstShot"),
        (0x35, "ExposureCompensation"),
        (0x39, "SensorTemperature"),
        (0x3a, "FlashExposureComp"),
        (0x3b, "Firmware"),
        (0x3c, "WhiteBalance"),
        (0x3d, "PictureMode"),
        (0x48, "LensApertureRange"),
        (0x49, "FNumber"),
        (0x4a, "ExposureTime"),
        (0x4b, "ExposureTime2"),
        (0x4d, "ExposureCompensation"),
        (0x55, "SensorTemperature"),
        (0x56, "FlashExposureComp"),
        (0x57, "Firmware2"),
        (0x58, "WhiteBalance"),
        (0x59, "DigitalFilter"),
        (0x84, "Model"),
        (0x86, "ISO"),
        (0x87, "ResolutionMode"),
        (0x88, "WhiteBalance"),
        (0x8c, "Firmware"),
        (0x11f, "CameraCalibration"),
        (0x120, "WBSettings"),
        (0x121, "WBSettings2"),
    ]);
}
