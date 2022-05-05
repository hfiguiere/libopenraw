// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - ricoh.rs
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

//! Ricoh camera support.

use std::collections::HashMap;

lazy_static::lazy_static! {
    pub static ref MNOTE_TAG_NAMES: HashMap<u16, &'static str> = HashMap::from([
        (0x1, "MakerNoteType"),
        (0x2, "FirmwareVersion"),
        (0x5, "SerialNumber"),
        (0xe00, "PrintIM"),
        (0x1000, "RecordingFormat"),
        (0x1001, "ImageInfo"),
        (0x1002, "DriveMode"),
        (0x1003, "Sharpness"),
        (0x1004, "WhiteBalanceFineTune"),
        (0x1006, "FocusMode"),
        (0x1007, "AutoBracketing"),
        (0x1009, "MacroMode"),
        (0x100a, "FlashMode"),
        (0x100b, "FlashExposureComp"),
        (0x100c, "ManualFlashOutput"),
        (0x100d, "FullPressSnap"),
        (0x100e, "DynamicRangeExpansion"),
        (0x100f, "NoiseReduction"),
        (0x1010, "ImageEffects"),
        (0x1011, "Vignetting"),
        (0x1012, "Contrast"),
        (0x1013, "Saturation"),
        (0x1014, "Sharpness"),
        (0x1015, "ToningEffect"),
        (0x1016, "HueAdjust"),
        (0x1017, "WideAdapter"),
        (0x1018, "CropMode"),
        (0x1019, "NDFilter"),
        (0x101a, "WBBracketShotNumber"),
        (0x1200, "AFStatus"),
        (0x1201, "AFAreaXPosition1"),
        (0x1202, "AFAreaYPosition1"),
        (0x1203, "AFAreaXPosition"),
        (0x1204, "AFAreaYPosition"),
        (0x1205, "AFAreaMode"),
        (0x1307, "ColorTempKelvin"),
        (0x1308, "ColorTemperature"),
        (0x1500, "FocalLength"),
        (0x1601, "SensorWidth"),
        (0x1602, "SensorHeight"),
        (0x1603, "CroppedImageWidth"),
        (0x1604, "CroppedImageHeight"),
        (0x2001, "RicohSubdir"),
        (0x4001, "ThetaSubdir"),
    ]);
}
