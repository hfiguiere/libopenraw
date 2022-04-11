/*
 * libopenraw - pentax.rs
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

//! Pentax camera support.

use std::collections::HashMap;

lazy_static::lazy_static! {
    pub static ref MNOTE_TAG_NAMES: HashMap<u16, &'static str> = HashMap::from([
        (0x0, "PentaxVersion"),
        (0x1, "PentaxModelType"),
        (0x2, "PreviewImageSize"),
        (0x3, "PreviewImageLength"),
        (0x4, "PreviewImageStart"),
        (0x5, "PentaxModelID"),
        (0x6, "Date"),
        (0x7, "Time"),
        (0x8, "Quality"),
        (0x9, "PentaxImageSize"),
        (0xb, "PictureMode"),
        (0xc, "FlashMode"),
        (0xd, "FocusMode"),
        (0xe, "AFPointSelected"),
        (0xf, "AFPointsInFocus"),
        (0x10, "FocusPosition"),
        (0x12, "ExposureTime"),
        (0x13, "FNumber"),
        (0x14, "ISO"),
        (0x15, "LightReading"),
        (0x16, "ExposureCompensation"),
        (0x17, "MeteringMode"),
        (0x18, "AutoBracketing"),
        (0x19, "WhiteBalance"),
        (0x1a, "WhiteBalanceMode"),
        (0x1b, "BlueBalance"),
        (0x1c, "RedBalance"),
        (0x1d, "FocalLength"),
        (0x1e, "DigitalZoom"),
        (0x1f, "Saturation"),
        (0x20, "Contrast"),
        (0x21, "Sharpness"),
        (0x22, "WorldTimeLocation"),
        (0x23, "HometownCity"),
        (0x24, "DestinationCity"),
        (0x25, "HometownDST"),
        (0x26, "DestinationDST"),
        (0x27, "DSPFirmwareVersion"),
        (0x28, "CPUFirmwareVersion"),
        (0x29, "FrameNumber"),
        (0x2d, "EffectiveLV"),
        (0x32, "ImageEditing"),
        (0x33, "PictureMode"),
        (0x34, "DriveMode"),
        (0x35, "SensorSize"),
        (0x37, "ColorSpace"),
        (0x38, "ImageAreaOffset"),
        (0x39, "RawImageSize"),
        (0x3c, "AFPointsInFocus"),
        (0x3d, "DataScaling"),
        (0x3e, "PreviewImageBorders"),
        (0x3f, "LensRec"),
        (0x40, "SensitivityAdjust"),
        (0x41, "ImageEditCount"),
        (0x47, "CameraTemperature"),
        (0x48, "AELock"),
        (0x49, "NoiseReduction"),
        (0x4d, "FlashExposureComp"),
        (0x4f, "ImageTone"),
        (0x50, "ColorTemperature"),
        (0x53, "ColorTempDaylight"),
        (0x54, "ColorTempShade"),
        (0x55, "ColorTempCloudy"),
        (0x56, "ColorTempTungsten"),
        (0x57, "ColorTempFluorescentD"),
        (0x58, "ColorTempFluorescentN"),
        (0x59, "ColorTempFluorescentW"),
        (0x5a, "ColorTempFlash"),
        (0x5c, "ShakeReductionInfo"),
        (0x5d, "ShutterCount"),
        (0x60, "FaceInfo"),
        (0x62, "RawDevelopmentProcess"),
        (0x67, "Hue"),
        (0x68, "AWBInfo"),
        (0x69, "DynamicRangeExpansion"),
        (0x6b, "TimeInfo"),
        (0x6c, "HighLowKeyAdj"),
        (0x6d, "ContrastHighlight"),
        (0x6e, "ContrastShadow"),
        (0x6f, "ContrastHighlightShadowAdj"),
        (0x70, "FineSharpness"),
        (0x71, "HighISONoiseReduction"),
        (0x72, "AFAdjustment"),
        (0x73, "MonochromeFilterEffect"),
        (0x74, "MonochromeToning"),
        (0x76, "FaceDetect"),
        (0x77, "FaceDetectFrameSize"),
        (0x79, "ShadowCorrection"),
        (0x7a, "ISOAutoParameters"),
        (0x7b, "CrossProcess"),
        (0x7d, "LensCorr"),
        (0x7e, "WhiteLevel"),
        (0x7f, "BleachBypassToning"),
        (0x80, "AspectRatio"),
        (0x82, "BlurControl"),
        (0x85, "HDR"),
        (0x87, "ShutterType"),
        (0x88, "NeutralDensityFilter"),
        (0x8b, "ISO"),
        (0x92, "IntervalShooting"),
        (0x95, "SkinToneCorrection"),
        (0x96, "ClarityControl"),
        (0x200, "BlackPoint"),
        (0x201, "WhitePoint"),
        (0x203, "ColorMatrixA"),
        (0x204, "ColorMatrixB"),
        (0x205, "CameraSettings"),
        (0x206, "AEInfo"),
        (0x207, "LensInfo"),
        (0x208, "FlashInfo"),
        (0x209, "AEMeteringSegments"),
        (0x20a, "FlashMeteringSegments"),
        (0x20b, "SlaveFlashMeteringSegments"),
        (0x20d, "WB_RGGBLevelsDaylight"),
        (0x20e, "WB_RGGBLevelsShade"),
        (0x20f, "WB_RGGBLevelsCloudy"),
        (0x210, "WB_RGGBLevelsTungsten"),
        (0x211, "WB_RGGBLevelsFluorescentD"),
        (0x212, "WB_RGGBLevelsFluorescentN"),
        (0x213, "WB_RGGBLevelsFluorescentW"),
        (0x214, "WB_RGGBLevelsFlash"),
        (0x215, "CameraInfo"),
        (0x216, "BatteryInfo"),
        (0x21b, "SaturationInfo"),
        (0x21c, "ColorMatrixA2"),
        (0x21d, "ColorMatrixB2"),
        (0x21f, "AFInfo"),
        (0x220, "HuffmanTable"),
        (0x221, "KelvinWB"),
        (0x222, "ColorInfo"),
        (0x224, "EVStepInfo"),
        (0x226, "ShotInfo"),
        (0x227, "FacePos"),
        (0x228, "FaceSize"),
        (0x229, "SerialNumber"),
        (0x22a, "FilterInfo"),
        (0x22b, "LevelInfo"),
        (0x22d, "WBLevels"),
        (0x22e, "Artist"),
        (0x22f, "Copyright"),
        (0x230, "FirmwareVersion"),
        (0x231, "ContrastDetectAFArea"),
        (0x235, "CrossProcessParams"),
        (0x239, "LensInfoQ"),
        (0x23f, "Model"),
        (0x243, "PixelShiftInfo"),
        (0x245, "AFPointInfo"),
        (0x3fe, "DataDump"),
        (0x3ff, "TempInfo"),
        (0x402, "ToneCurve"),
        (0x403, "ToneCurves"),
        (0x405, "UnknownBlock"),
        (0xe00, "PrintIM"),
    ]);
}
