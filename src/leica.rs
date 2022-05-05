// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - panasonic.rs
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

//! Leica camera support

use std::collections::HashMap;

lazy_static::lazy_static! {
    pub static ref MNOTE_TAG_NAMES_2: HashMap<u16, &'static str> = HashMap::from([
        (0x300, "Quality"),
        (0x302, "UserProfile"),
        (0x303, "SerialNumber"),
        (0x304, "WhiteBalance"),
        (0x310, "LensType"),
        (0x311, "ExternalSensorBrightnessValue"),
        (0x312, "MeasuredLV"),
        (0x313, "ApproximateFNumber"),
        (0x320, "CameraTemperature"),
        (0x321, "ColorTemperature"),
        (0x322, "WBRedLevel"),
        (0x323, "WBGreenLevel"),
        (0x324, "WBBlueLevel"),
        (0x325, "UV-IRFilterCorrection"),
        (0x330, "CCDVersion"),
        (0x331, "CCDBoardVersion"),
        (0x332, "ControllerBoardVersion"),
        (0x333, "M16CVersion"),
        (0x340, "ImageIDNumber"),
    ]);

    pub static ref MNOTE_TAG_NAMES_4: HashMap<u16, &'static str> = HashMap::from([
        (0x3000, "Subdir3000"),
        (0x3100, "Subdir3100"),
        (0x3400, "Subdir3400"),
        (0x3900, "Subdir3900"),
    ]);

    pub static ref MNOTE_TAG_NAMES_5: HashMap<u16, &'static str> = HashMap::from([
        (0x303, "LensType"),
        (0x305, "SerialNumber"),
        (0x407, "OriginalFileName"),
        (0x408, "OriginalDirectory"),
        (0x40a, "FocusInfo"),
        (0x40d, "ExposureMode"),
        (0x410, "ShotInfo"),
        (0x412, "FilmMode"),
        (0x413, "WB_RGBLevels"),
        (0x500, "InternalSerialNumber"),
    ]);

    pub static ref MNOTE_TAG_NAMES_6: HashMap<u16, &'static str> = HashMap::from([
        (0x300, "PreviewImage"),
        (0x301, "UnknownBlock"),
        (0x303, "LensType"),
        (0x304, "FocusDistance"),
        (0x311, "ExternalSensorBrightnessValue"),
        (0x312, "MeasuredLV"),
        (0x320, "FirmwareVersion"),
        (0x321, "LensSerialNumber"),
    ]);

    pub static ref MNOTE_TAG_NAMES_9: HashMap<u16, &'static str> = HashMap::from([
        (0x304, "FocusDistance"),
        (0x311, "ExternalSensorBrightnessValue"),
        (0x312, "MeasuredLV"),
        (0x34c, "UserProfile"),
        (0x359, "ISOSelected"),
        (0x35a, "FNumber"),
        (0x35b, "CorrelatedColorTemp"),
        (0x35c, "ColorTint"),
        (0x35d, "WhitePoint"),
    ]);
}
