/*
 * libopenraw - sony.rs
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

//! Sony specific code.

use std::collections::HashMap;

lazy_static::lazy_static! {
    /// Sony MakerNote tag names
    pub static ref MNOTE_TAG_NAMES: HashMap<u16, &'static str> = HashMap::from([
        (0x10, "CameraInfo"),
        (0x20, "FocusInfo"),
        (0x102, "Quality"),
        (0x104, "FlashExposureComp"),
        (0x105, "Teleconverter"),
        (0x112, "WhiteBalanceFineTune"),
        (0x114, "CameraSettings"),
        (0x115, "WhiteBalance"),
        (0x116, "ExtraInfo"),
        (0xe00, "PrintIM"),
        (0x1000, "MultiBurstMode"),
        (0x1001, "MultiBurstImageWidth"),
        (0x1002, "MultiBurstImageHeight"),
        (0x1003, "Panorama"),
        (0x2001, "PreviewImage"),
        (0x2002, "Rating"),
        (0x2004, "Contrast"),
        (0x2005, "Saturation"),
        (0x2006, "Sharpness"),
        (0x2007, "Brightness"),
        (0x2008, "LongExposureNoiseReduction"),
        (0x2009, "HighISONoiseReduction"),
        (0x200a, "HDR"),
        (0x200b, "MultiFrameNoiseReduction"),
        (0x200e, "PictureEffect"),
        (0x200f, "SoftSkinEffect"),
        (0x2010, "Tag2010a"),
        (0x2011, "VignettingCorrection"),
        (0x2012, "LateralChromaticAberration"),
        (0x2013, "DistortionCorrectionSetting"),
        (0x2014, "WBShiftAB_GM"),
        (0x2016, "AutoPortraitFramed"),
        (0x2017, "FlashAction"),
        (0x201a, "ElectronicFrontCurtainShutter"),
        (0x201b, "FocusMode"),
        (0x201c, "AFAreaModeSetting"),
        (0x201d, "FlexibleSpotPosition"),
        (0x201e, "AFPointSelected"),
        (0x2020, "AFPointsUsed"),
        (0x2021, "AFTracking"),
        (0x2022, "FocalPlaneAFPointsUsed"),
        (0x2023, "MultiFrameNREffect"),
        (0x2026, "WBShiftAB_GM_Precise"),
        (0x2027, "FocusLocation"),
        (0x2028, "VariableLowPassFilter"),
        (0x2029, "RAWFileType"),
        (0x202a, "Tag202a"),
        (0x202b, "PrioritySetInAWB"),
        (0x202c, "MeteringMode2"),
        (0x202d, "ExposureStandardAdjustment"),
        (0x202e, "Quality"),
        (0x202f, "PixelShiftInfo"),
        (0x2031, "SerialNumber"),
        (0x3000, "ShotInfo"),
        (0x900b, "Tag900b"),
        (0x9050, "Tag9050a"),
        (0x9400, "Tag9400a"),
        (0x9401, "Tag9401"),
        (0x9402, "Tag9402"),
        (0x9403, "Tag9403"),
        (0x9404, "Tag9404a"),
        (0x9405, "Tag9405a"),
        (0x9406, "Tag9406"),
        (0x9407, "Sony_0x9407"),
        (0x9408, "Sony_0x9408"),
        (0x9409, "Sony_0x9409"),
        (0x940a, "Tag940a"),
        (0x940b, "Sony_0x940b"),
        (0x940c, "Tag940c"),
        (0x940d, "Sony_0x940d"),
        (0x940e, "AFInfo"),
        (0x940f, "Sony_0x940f"),
        (0x9411, "Sony_0x9411"),
        (0xb000, "FileFormat"),
        (0xb001, "SonyModelID"),
        (0xb020, "CreativeStyle"),
        (0xb021, "ColorTemperature"),
        (0xb022, "ColorCompensationFilter"),
        (0xb023, "SceneMode"),
        (0xb024, "ZoneMatching"),
        (0xb025, "DynamicRangeOptimizer"),
        (0xb026, "ImageStabilization"),
        (0xb027, "LensType"),
        (0xb028, "MinoltaMakerNote"),
        (0xb029, "ColorMode"),
        (0xb02a, "LensSpec"),
        (0xb02b, "FullImageSize"),
        (0xb02c, "PreviewImageSize"),
        (0xb040, "Macro"),
        (0xb041, "ExposureMode"),
        (0xb042, "FocusMode"),
        (0xb043, "AFAreaMode"),
        (0xb044, "AFIlluminator"),
        (0xb047, "JPEGQuality"),
        (0xb048, "FlashLevel"),
        (0xb049, "ReleaseMode"),
        (0xb04a, "SequenceNumber"),
        (0xb04b, "Anti-Blur"),
        (0xb04e, "FocusMode"),
        (0xb04f, "DynamicRangeOptimizer"),
        (0xb050, "HighISONoiseReduction2"),
        (0xb052, "IntelligentAuto"),
        (0xb054, "WhiteBalance"),
    ]);
}
