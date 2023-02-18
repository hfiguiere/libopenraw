// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - panasonic.rs
 *
 * Copyright (C) 2022-2023 Hubert Figuière
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

//! Panasonic camera support (and some Leica)

use std::collections::HashMap;
use std::rc::Rc;

use once_cell::unsync::OnceCell;

use crate::bitmap;
use crate::camera_ids::{leica, panasonic, vendor};
use crate::colour::BuiltinMatrix;
use crate::container::{Endian, RawContainer};
use crate::decompress;
use crate::io::Viewer;
use crate::jpeg;
use crate::mosaic::Pattern;
use crate::rawfile::ReadAndSeek;
use crate::thumbnail;
use crate::tiff;
use crate::tiff::exif;
use crate::tiff::{Dir, Ifd};
use crate::{DataType, Dump, Error, RawData, RawFile, RawFileImpl, Result, Type, TypeId};

lazy_static::lazy_static! {
    pub static ref RAW_TAG_NAMES: HashMap<u16, &'static str> = HashMap::from([
        (0x1, "PanasonicRawVersion"),
        (0x2, "SensorWidth"),
        (0x3, "SensorHeight"),
        (0x4, "SensorTopBorder"),
        (0x5, "SensorLeftBorder"),
        (0x6, "SensorBottomBorder"),
        (0x7, "SensorRightBorder"),
        (0x8, "SamplesPerPixel"),
        (0x9, "CFAPattern"),
        (0xa, "BitsPerSample"),
        (0xb, "Compression"),
        (0xe, "LinearityLimitRed"),
        (0xf, "LinearityLimitGreen"),
        (0x10, "LinearityLimitBlue"),
        (0x11, "RedBalance"),
        (0x12, "BlueBalance"),
        (0x13, "WBInfo"),
        (0x17, "ISO"),
        (0x18, "HighISOMultiplierRed"),
        (0x19, "HighISOMultiplierGreen"),
        (0x1a, "HighISOMultiplierBlue"),
        (0x1b, "NoiseReductionParams"),
        (0x1c, "BlackLevelRed"),
        (0x1d, "BlackLevelGreen"),
        (0x1e, "BlackLevelBlue"),
        (0x24, "WBRedLevel"),
        (0x25, "WBGreenLevel"),
        (0x26, "WBBlueLevel"),
        (0x27, "WBInfo2"),
        (0x2d, "RawFormat"),
        (0x2e, "JpgFromRaw"),
        (0x2f, "CropTop"),
        (0x30, "CropLeft"),
        (0x31, "CropBottom"),
        (0x32, "CropRight"),
        (0x10f, "Make"),
        (0x110, "Model"),
        (0x111, "StripOffsets"),
        (0x112, "Orientation"),
        (0x116, "RowsPerStrip"),
        (0x117, "StripByteCounts"),
        (0x118, "RawDataOffset"),
        (0x119, "DistortionInfo"),
        (0x11c, "Gamma"),
        (0x120, "CameraIFD"),
        (0x121, "Multishot"),
        (0x2bc, "ApplicationNotes"),
        (0x83bb, "IPTC-NAA"),
        (0x8769, "ExifOffset"),
        (0x8825, "GPSInfo"),
    ]);

    pub static ref MNOTE_TAG_NAMES: HashMap<u16, &'static str> = HashMap::from([
        (0x1, "ImageQuality"),
        (0x2, "FirmwareVersion"),
        (0x3, "WhiteBalance"),
        (0x7, "FocusMode"),
        (0xf, "AFAreaMode"),
        (0x1a, "ImageStabilization"),
        (0x1c, "MacroMode"),
        (0x1f, "ShootingMode"),
        (0x20, "Audio"),
        (0x21, "DataDump"),
        (0x23, "WhiteBalanceBias"),
        (0x24, "FlashBias"),
        (0x25, "InternalSerialNumber"),
        (0x26, "PanasonicExifVersion"),
        (0x27, "VideoFrameRate"),
        (0x28, "ColorEffect"),
        (0x29, "TimeSincePowerOn"),
        (0x2a, "BurstMode"),
        (0x2b, "SequenceNumber"),
        (0x2c, "ContrastMode"),
        (0x2d, "NoiseReduction"),
        (0x2e, "SelfTimer"),
        (0x30, "Rotation"),
        (0x31, "AFAssistLamp"),
        (0x32, "ColorMode"),
        (0x33, "BabyAge"),
        (0x34, "OpticalZoomMode"),
        (0x35, "ConversionLens"),
        (0x36, "TravelDay"),
        (0x38, "BatteryLevel"),
        (0x39, "Contrast"),
        (0x3a, "WorldTimeLocation"),
        (0x3b, "TextStamp"),
        (0x3c, "ProgramISO"),
        (0x3d, "AdvancedSceneType"),
        (0x3e, "TextStamp"),
        (0x3f, "FacesDetected"),
        (0x40, "Saturation"),
        (0x41, "Sharpness"),
        (0x42, "FilmMode"),
        (0x43, "JPEGQuality"),
        (0x44, "ColorTempKelvin"),
        (0x45, "BracketSettings"),
        (0x46, "WBShiftAB"),
        (0x47, "WBShiftGM"),
        (0x48, "FlashCurtain"),
        (0x49, "LongExposureNoiseReduction"),
        (0x4b, "PanasonicImageWidth"),
        (0x4c, "PanasonicImageHeight"),
        (0x4d, "AFPointPosition"),
        (0x4e, "FaceDetInfo"),
        (0x51, "LensType"),
        (0x52, "LensSerialNumber"),
        (0x53, "AccessoryType"),
        (0x54, "AccessorySerialNumber"),
        (0x59, "Transform"),
        (0x5d, "IntelligentExposure"),
        (0x60, "LensFirmwareVersion"),
        (0x61, "FaceRecInfo"),
        (0x62, "FlashWarning"),
        (0x63, "RecognizedFaceFlags"),
        (0x65, "Title"),
        (0x66, "BabyName"),
        (0x67, "Location"),
        (0x69, "Country"),
        (0x6b, "State"),
        (0x6d, "City"),
        (0x6f, "Landmark"),
        (0x70, "IntelligentResolution"),
        (0x76, "HDRShot"),
        (0x77, "BurstSpeed"),
        (0x79, "IntelligentD-Range"),
        (0x7c, "ClearRetouch"),
        (0x80, "City2"),
        (0x86, "ManometerPressure"),
        (0x89, "PhotoStyle"),
        (0x8a, "ShadingCompensation"),
        (0x8b, "WBShiftIntelligentAuto"),
        (0x8c, "AccelerometerZ"),
        (0x8d, "AccelerometerX"),
        (0x8e, "AccelerometerY"),
        (0x8f, "CameraOrientation"),
        (0x90, "RollAngle"),
        (0x91, "PitchAngle"),
        (0x92, "WBShiftCreativeControl"),
        (0x93, "SweepPanoramaDirection"),
        (0x94, "SweepPanoramaFieldOfView"),
        (0x96, "TimerRecording"),
        (0x9d, "InternalNDFilter"),
        (0x9e, "HDR"),
        (0x9f, "ShutterType"),
        (0xa1, "FilterEffect"),
        (0xa3, "ClearRetouchValue"),
        (0xa7, "OutputLUT"),
        (0xab, "TouchAE"),
        (0xac, "MonochromeFilterEffect"),
        (0xad, "HighlightShadow"),
        (0xaf, "TimeStamp"),
        (0xb3, "VideoBurstResolution"),
        (0xb4, "MultiExposure"),
        (0xb9, "RedEyeRemoval"),
        (0xbb, "VideoBurstMode"),
        (0xbc, "DiffractionCorrection"),
        (0xbd, "FocusBracket"),
        (0xbe, "LongExposureNRUsed"),
        (0xbf, "PostFocusMerging"),
        (0xc1, "VideoPreburst"),
        (0xc4, "LensTypeMake"),
        (0xc5, "LensTypeModel"),
        (0xca, "SensorType"),
        (0xd1, "ISO"),
        (0xd2, "MonochromeGrainEffect"),
        (0xd6, "NoiseReductionStrength"),
        (0xe4, "LensTypeModel"),
        (0xe00, "PrintIM"),
        (0x2003, "TimeInfo"),
        (0x8000, "MakerNoteVersion"),
        (0x8001, "SceneMode"),
        (0x8002, "HighlightWarning"),
        (0x8003, "DarkFocusEnvironment"),
        (0x8004, "WBRedLevel"),
        (0x8005, "WBGreenLevel"),
        (0x8006, "WBBlueLevel"),
        (0x8008, "TextStamp"),
        (0x8009, "TextStamp"),
        (0x8010, "BabyAge"),
        (0x8012, "Transform"),
    ]);

    static ref MAKE_TO_ID_MAP: tiff::MakeToIdMap = HashMap::from([
        ("DMC-CM1", TypeId(vendor::PANASONIC, panasonic::CM1)),
        ("DMC-GF1", TypeId(vendor::PANASONIC, panasonic::GF1)),
        ("DMC-GF2", TypeId(vendor::PANASONIC, panasonic::GF2)),
        ("DMC-GF3", TypeId(vendor::PANASONIC, panasonic::GF3)),
        ("DMC-GF5", TypeId(vendor::PANASONIC, panasonic::GF5)),
        ("DMC-GF6", TypeId(vendor::PANASONIC, panasonic::GF6)),
        ("DMC-GF7", TypeId(vendor::PANASONIC, panasonic::GF7)),
        ("DC-GF10", TypeId(vendor::PANASONIC, panasonic::GF10)),
        ("DMC-GX1", TypeId(vendor::PANASONIC, panasonic::GX1)),
        ("DMC-GX7", TypeId(vendor::PANASONIC, panasonic::GX7)),
        ("DMC-GX7MK2", TypeId(vendor::PANASONIC, panasonic::GX7MK2)),
        ("DMC-GX8", TypeId(vendor::PANASONIC, panasonic::GX8)),
        ("DMC-GX80", TypeId(vendor::PANASONIC, panasonic::GX80)),
        ("DC-GX800", TypeId(vendor::PANASONIC, panasonic::GX800)),
        ("DC-GX850", TypeId(vendor::PANASONIC, panasonic::GX850)),
        ("DC-GX880", TypeId(vendor::PANASONIC, panasonic::GX880)),
        ("DC-GX9", TypeId(vendor::PANASONIC, panasonic::GX9)),
        ("DMC-FZ8", TypeId(vendor::PANASONIC, panasonic::FZ8)),
        ("DMC-FZ1000", TypeId(vendor::PANASONIC, panasonic::DMC_FZ1000)),
        ("DC-FZ10002", TypeId(vendor::PANASONIC, panasonic::DC_FZ1000M2)),
        ("DC-FZ1000M2", TypeId(vendor::PANASONIC, panasonic::DC_FZ1000M2)),
        ("DMC-FZ18", TypeId(vendor::PANASONIC, panasonic::FZ18)),
        ("DMC-FZ150", TypeId(vendor::PANASONIC, panasonic::FZ150)),
        ("DMC-FZ28", TypeId(vendor::PANASONIC, panasonic::FZ28)),
        ("DMC-FZ30", TypeId(vendor::PANASONIC, panasonic::FZ30)),
        ("DMC-FZ35", TypeId(vendor::PANASONIC, panasonic::FZ35)),
        ("DMC-FZ40", TypeId(vendor::PANASONIC, panasonic::DMC_FZ40)),
        ("DMC-FZ45", TypeId(vendor::PANASONIC, panasonic::DMC_FZ45)),
        // Not the same as above
        ("DC-FZ45", TypeId(vendor::PANASONIC, panasonic::DC_FZ45)),
        ("DMC-FZ50", TypeId(vendor::PANASONIC, panasonic::FZ50)),
        ("DMC-FZ100", TypeId(vendor::PANASONIC, panasonic::FZ100)),
        ("DMC-FZ200", TypeId(vendor::PANASONIC, panasonic::FZ200)),
        ("DMC-FZ2500", TypeId(vendor::PANASONIC, panasonic::FZ2500)),
        // Alias to DMC-FZ2500
        ("DMC-FZ2000", TypeId(vendor::PANASONIC, panasonic::FZ2000)),
        ("DMC-FZ330", TypeId(vendor::PANASONIC, panasonic::FZ330)),
        ("DC-FZ80", TypeId(vendor::PANASONIC, panasonic::FZ80)),
        ("DC-FZ82", TypeId(vendor::PANASONIC, panasonic::FZ82)),
        ("DMC-G1", TypeId(vendor::PANASONIC, panasonic::G1)),
        ("DMC-G2", TypeId(vendor::PANASONIC, panasonic::G2)),
        ("DMC-G3", TypeId(vendor::PANASONIC, panasonic::G3)),
        ("DMC-G5", TypeId(vendor::PANASONIC, panasonic::G5)),
        ("DMC-G7", TypeId(vendor::PANASONIC, panasonic::G7)),
        ("DMC-G10", TypeId(vendor::PANASONIC, panasonic::G10)),
        ("DMC-G80", TypeId(vendor::PANASONIC, panasonic::G80)),
        ("DC-G9", TypeId(vendor::PANASONIC, panasonic::G9)),
        ("DC-G91", TypeId(vendor::PANASONIC, panasonic::DC_G91)),
        ("DC-G95", TypeId(vendor::PANASONIC, panasonic::DC_G95)),
        ("DC-G99", TypeId(vendor::PANASONIC, panasonic::DC_G99)),
        ("DC-G100", TypeId(vendor::PANASONIC, panasonic::DC_G100)),
        ("DC-G110", TypeId(vendor::PANASONIC, panasonic::DC_G110)),
        ("DMC-GH1", TypeId(vendor::PANASONIC, panasonic::GH1)),
        ("DMC-GH2", TypeId(vendor::PANASONIC, panasonic::GH2)),
        ("DMC-GH3", TypeId(vendor::PANASONIC, panasonic::GH3)),
        ("DMC-GH4", TypeId(vendor::PANASONIC, panasonic::GH4)),
        ("DC-GH5", TypeId(vendor::PANASONIC, panasonic::GH5)),
        ("DC-GH5S", TypeId(vendor::PANASONIC, panasonic::GH5S)),
        ("DC-GH5M2", TypeId(vendor::PANASONIC, panasonic::GH5M2)),
        ("DC-GH6", TypeId(vendor::PANASONIC, panasonic::GH6)),
        ("DMC-GM1", TypeId(vendor::PANASONIC, panasonic::GM1)),
        ("DMC-GM5", TypeId(vendor::PANASONIC, panasonic::GM5)),
        ("DMC-LX1", TypeId(vendor::PANASONIC, panasonic::LX1)),
        ("DMC-LX2", TypeId(vendor::PANASONIC, panasonic::LX2)),
        ("DMC-LX3", TypeId(vendor::PANASONIC, panasonic::LX3)),
        ("DMC-LX5", TypeId(vendor::PANASONIC, panasonic::LX5)),
        ("DMC-LX7", TypeId(vendor::PANASONIC, panasonic::LX7)),
        ("DMC-LX10", TypeId(vendor::PANASONIC, panasonic::LX10)),
        ("DMC-LX15", TypeId(vendor::PANASONIC, panasonic::LX15)),
        ("DMC-LX100", TypeId(vendor::PANASONIC, panasonic::LX100)),
        ("DC-LX100M2", TypeId(vendor::PANASONIC, panasonic::LX100M2)),
        ("DMC-L1", TypeId(vendor::PANASONIC, panasonic::L1)),
        ("DMC-L10", TypeId(vendor::PANASONIC, panasonic::L10)),
        ("DC-S1", TypeId(vendor::PANASONIC, panasonic::DC_S1)),
        ("DC-S1R", TypeId(vendor::PANASONIC, panasonic::DC_S1R)),
        ("DC-S1H", TypeId(vendor::PANASONIC, panasonic::DC_S1H)),
        ("DC-S5", TypeId(vendor::PANASONIC, panasonic::DC_S5)),
        ("DMC-TZ70", TypeId(vendor::PANASONIC, panasonic::TZ70)),
        ("DMC-ZS60", TypeId(vendor::PANASONIC, panasonic::ZS60)),
        // Aliases to DMC-ZS60
        ("DMC-TZ80", TypeId(vendor::PANASONIC, panasonic::TZ80)),
        ("DMC-ZS100", TypeId(vendor::PANASONIC, panasonic::ZS100)),
        // Aliases to DMC-ZS100
        ("DMC-TX1", TypeId(vendor::PANASONIC, panasonic::TX1)),
        ("DMC-TZ100", TypeId(vendor::PANASONIC, panasonic::TZ100)),
        ("DMC-TZ110", TypeId(vendor::PANASONIC, panasonic::TZ110)),
        ("DC-ZS200", TypeId(vendor::PANASONIC, panasonic::ZS200)),
        // Aliases to DMC-ZS200
        ("DC-TZ202", TypeId(vendor::PANASONIC, panasonic::TZ202)),
        ("DC-ZS80", TypeId(vendor::PANASONIC, panasonic::DC_ZS80)),
        // Aliases to DC-ZS80
        ("DC-TZ95", TypeId(vendor::PANASONIC, panasonic::DC_TZ95)),

        ("DIGILUX 2", TypeId(vendor::LEICA, leica::DIGILUX2)),
        ("D-LUX 3", TypeId(vendor::LEICA, leica::DLUX_3)),
        ("D-LUX 4", TypeId(vendor::LEICA, leica::DLUX_4)),
        ("D-LUX 5", TypeId(vendor::LEICA, leica::DLUX_5)),
        ("D-Lux 7", TypeId(vendor::LEICA, leica::DLUX_7)),
        ("V-LUX 1", TypeId(vendor::LEICA, leica::VLUX_1)),
        ("D-LUX (Typ 109)", TypeId(vendor::LEICA, leica::DLUX_TYP109)),
        ("V-LUX 4", TypeId(vendor::LEICA, leica::VLUX_4)),
        ("V-Lux 5", TypeId(vendor::LEICA, leica::VLUX_5)),
        ("V-LUX (Typ 114)", TypeId(vendor::LEICA, leica::VLUX_TYP114)),
        ("C-Lux", TypeId(vendor::LEICA, leica::CLUX)),
        ("C (Typ 112)", TypeId(vendor::LEICA, leica::C_TYP112)),
    ]);

    static ref MATRICES: [BuiltinMatrix; 83] = [
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::CM1),
            15,
            0,
            [ 8770, -3194, -820, -2871, 11281, 1803, -513, 1552, 4434 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::GF1),
            15,
            0xf92,
            [ 7888, -1902, -1011, -8106, 16085, 2099, -2353, 2866, 7330 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::GF2),
            15,
            0xfff,
            [ 7888, -1902, -1011, -8106, 16085, 2099, -2353, 2866, 7330 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::GF3),
            15,
            0xfff,
            [ 9051, -2468, -1204, -5212, 13276, 2121, -1197, 2510, 6890 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::GF5),
            15,
            0xfff,
            [ 8228, -2945, -660, -3938, 11792, 2430, -1094, 2278, 5793 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::GF6),
            15,
            0xfff,
            [ 8130, -2801, -946, -3520, 11289, 2552, -1314, 2511, 5791 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::GF7),
            15,
            0,
            [ 7610, -2780, -576, -4614, 12195, 2733, -1375, 2393, 6490 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::GF10),
            15,
            0,
            [ 7610, -2780, -576, -4614, 12195, 2733, -1375, 2393, 6490 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::GX1),
            15,
            0,
            [ 6763, -1919, -863, -3868, 11515, 2684, -1216, 2387, 5879 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::GX7),
            15,
            0,
            [ 7610, -2780, -576, -4614, 12195, 2733, -1375, 2393, 6490 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::GX7MK2),
            15,
            0,
            [ 7771, -3020, -629, -4029, 1195, 2345, -821, 1977, 6119 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::GX8),
            15,
            0,
            [ 7564, -2263, -606, -3148, 11239, 2177, -540, 1435, 4853 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::GX80),
            15,
            0,
            [ 7771, -3020, -629, -4029, 1195, 2345, -821, 1977, 6119 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::GX800),
            15,
            0,
            [ 7610, -2780, -576, -4614, 12195, 2733, -1375, 2393, 6490 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::GX850),
            15,
            0,
            [ 7610, -2780, -576, -4614, 12195, 2733, -1375, 2393, 6490 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::GX9),
            15,
            0,
            [ 7564, -2263, -606, -3148, 11239, 2177, -540, 1435, 4853 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::FZ8),
            0,
            0xf7f,
            [ 8986, -2755, -802, -6341, 13575, 3077, -1476, 2144, 6379 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::FZ18),
            0,
            0,
            [ 9932, -3060, -935, -5809, 13331, 2753, -1267, 2155, 5575 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::FZ28),
            15,
            0xf96,
            [ 10109, -3488, -993, -5412, 12812, 2916, -1305, 2140, 5543 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::FZ200),
            143,
            0xfff,
            [ 8112, -2563, -740, -3730, 11784, 2197, -941, 2075, 4933 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::FZ2500),
            143,
            0xfff,
            [ 7386, -2443, -743, -3437, 11864, 1757, -608, 1660, 4766 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::FZ30),
            0,
            0xf94,
            [ 10976,-4029,-1141,-7918,15491,2600,-1670,2071,8246 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::FZ330),
            15,
            0,
            [ 8378, -2798, -769, -3068, 11410, 1877, -538, 1792, 4623 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::FZ35),
            15,
            0,
            [ 9938, -2780, -890, -4604, 12393, 2480, -1117, 2304, 4620 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::DMC_FZ45),
            0,
            0,
            [ 13639, -5535, -1371, -1698, 9633, 2430, 316, 1152, 4108 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::FZ50),
            0,
            0,
            [ 7906, -2709, -594, -6231, 13351, 3220, -1922, 2631, 6537 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::FZ100),
            143,
            0xfff,
            [ 16197, -6146, -1761, -2393, 10765, 1869, 366, 2238, 5248 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::DMC_FZ1000),
            0,
            0,
            [ 7830, -2696, -763, -3325, 11667, 1866, -641, 1712, 4824 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::DC_FZ1000M2),
            0,
            0,
            [ 9803, -4185, -992, -4066, 12578, 1628, -838, 1824, 5288 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::FZ150),
            0,
            0,
            [ 11904, -4541, -1189, -2355, 10899, 1662, -296, 1586, 4289 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::FZ80),
            0,
            0,
            [ 11532, -4324, -1066, -2375, 10847, 1749, -564, 1699, 4351 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::G1),
            15,
            0xf94,
            [ 8199, -2065, -1056, -8124, 16156, 2033, -2458, 3022, 7220 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::G2),
            15,
            0xf3c,
            [ 10113, -3400, -1114, -4765, 12683, 2317, -377, 1437, 6710 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::G3),
            143,
            0xfff,
            [ 6763, -1919, -863, -3868, 11515, 2684, -1216, 2387, 5879 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::G5),
            143,
            0xfff,
            [ 7798, -2562, -740, -3879, 11584, 2613, -1055, 2248, 5434 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::G10),
            0,
            0,
            [ 10113, -3400, -1114, -4765, 12683, 2317, -377, 1437, 6710 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::G7),
            0,
            0,
            [ 7610, -2780, -576, -4614, 12195, 2733, -1375, 2393, 6490 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::G80),
            15,
            0,
            [ 7610, -2780, -576, -4614, 12195, 2733, -1375, 2393, 6490 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::G9),
            0,
            0,
            [ 7685, -2375, -634, -3687, 11700, 2249, -748, 1546, 5111 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::DC_G95),
            0,
            0,
            [ 9657, -3963, -748, -3361, 11378, 2258, -568, 1415, 5158 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::DC_G99),
            0,
            0,
            [ 9657, -3963, -748, -3361, 11378, 2258, -568, 1415, 5158 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::DC_G100),
            0,
            0,
            [ 8370, -2869, -710, -3389, 11372, 2298, -640, 1599, 4887 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::GH1),
            15,
            0xf92,
            [ 6299, -1466, -532, -6535, 13852, 2969, -2331, 3112, 5984 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::GH2),
            15,
            0xf95,
            [ 7780, -2410, -806, -3913, 11724, 2484, -1018, 2390, 5298 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::GH3),
            144,
            0,
            [ 6559, -1752, -491, -3672, 11407, 2586, -962, 1875, 5130 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::GH4),
            15,
            0,
            [ 7122, -2108, -512, -3155, 11201, 2231, -541, 1423, 5045 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::GH5),
            15,
            0,
            [ 7641, -2336, -605, -3218, 11299, 2187, -485, 1338, 5121 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::GH5S),
            15,
            0,
            [ 6929, -2355, -708, -4192, 12534, 1828, -1097, 1989, 5195 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::GH5M2),
            15,
            0,
            [ 9300, -3659, -755, -2981, 10988, 2287, -190, 1077, 5016 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::GH6),
            15,
            0,
            [ 7949, -3491, -710, -3435, 11681, 1977, -503, 1622, 5065 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::GM1),
            15,
            0,
            [ 6770, -1895, -744, -5232, 13145, 2303, -1664, 2691, 5703 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::GM5),
            15,
            0,
            [ 8238, -3244, -679, -3921, 11814, 2384, -836, 2022, 5852 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::LX1),
            0,
            0,
            [ 10704, -4187, -1230, -8314, 15952, 2501, -920, 945, 8927 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::LX2),
            0,
            0,
            [ 8048, -2810, -623, -6450, 13519, 3272, -1700, 2146, 7049 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::LX3),
            15,
            0,
            [ 8128, -2668, -655, -6134, 13307, 3161, -1782, 2568, 6083 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::LX5),
            143,
            0,
            [ 10909, -4295, -948, -1333, 9306, 2399, 22, 1738, 4582 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::LX7),
            143,
            0,
            [ 10148, -3743, -991, -2837, 11366, 1659, -701, 1893, 4899 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::LX10), // and LX15 (alias)
            15,
            0,
            [ 7790, -2736, -755, -3452, 11870, 1769, -628, 1647, 4898 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::LX100),
            143,
            0,
            [ 8844, -3538, -768, -3709, 11762, 2200, -698, 1792, 5220 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::LX100M2),
            0,
            0,
            [ 11577, -4230, -1106, -3967, 12211, 1957, -758, 1762, 5610 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::L1),
            0,
            0xf7f,
            [ 8054, -1885, -1025, -8349, 16367, 2040, -2805, 3542, 7629 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::L10),
            15,
            0xf96,
            [ 8025, -1942, -1050, -7920, 15904, 2100, -2456, 3005, 7039 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::TZ70),
            15,
            0,
            [ 8802, -3135, -789, -3151, 11468, 1904, -550, 1745, 4810 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::ZS60),
            15,
            0,
            [ 8550, -2908, -842, -3195, 11529, 1881, -338, 1603, 4631 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::ZS100),
            0,
            0,
            [  7790, -2736, -755, -3452, 11870, 1769, -628, 1647, 4898 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::ZS200),
            0,
            0,
            [  7790, -2736, -755, -3452, 11870, 1769, -628, 1647, 4898 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::DC_S1),
            0,
            0,
            [ 9744, -3905, -779, -4899, 12807, 2324, -798, 1630, 5827 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::DC_S1R),
            0,
            0,
            [ 11822, -5321, -1249, -5958, 15114, 766, -614, 1264, 7043 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::DC_S1H),
            0,
            0,
            [ 9397, -3719, -805, -5425, 13326, 2309, -972, 1715, 6034 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::DC_S5),
            0,
            0,
            [ 9744, -3905, -779, -4899, 12807, 2324, -798, 1630, 5827 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::PANASONIC, panasonic::DC_ZS80),
            0,
            0,
            [ 12194, -5340, -1329, -3035, 11394, 1858, -50, 1418, 5219 ] ),

        BuiltinMatrix::new(
            TypeId(vendor::LEICA, leica::DIGILUX2),
            0,
            0,
            [ 11340, -4069, -1275, -7555, 15266, 2448, -2960, 3426, 7685 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::LEICA, leica::DLUX_3),
            0,
            0,
            [ 8048, -2810, -623, -6450, 13519, 3272, -1700, 2146, 7049 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::LEICA, leica::DLUX_TYP109),
            0,
            0,
            [ 8844, -3538, -768, -3709, 11762, 2200, -698, 1792, 5220 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::LEICA, leica::DLUX_4),
            0,
            0,
            [ 8128, -2668, -655, -6134, 13307, 3161, -1782, 2568, 6083 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::LEICA, leica::DLUX_5),
            143,
            0,
            [ 10909, -4295, -948, -1333, 9306, 2399, 22, 1738, 4582 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::LEICA, leica::VLUX_1),
            0,
            0,
            [ 7906, -2709, -594, -6231, 13351, 3220, -1922, 2631, 6537 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::LEICA, leica::VLUX_4),
            0,
            0,
            [ 8112, -2563, -740, -3730, 11784, 2197, -941, 2075, 4933 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::LEICA, leica::VLUX_TYP114),
            0,
            0,
            [ 7830, -2696, -763, -3325, 11667, 1866, -641, 1712, 4824 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::LEICA, leica::VLUX_5),
            0,
            0,
            [ 9803, -4185, -992, -4066, 12578, 1628, -838, 1824, 5288 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::LEICA, leica::CLUX),
            15,
            0,
            [ 7790, -2736, -755, -3452, 11870, 1769, -628, 1647, 4898 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::LEICA, leica::DLUX_7),
            0,
            0,
            [ 11577, -4230, -1106, -3967, 12211, 1957, -758, 1762, 5610 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::LEICA, leica::C_TYP112),
            0,
            0,
            [ 9379, -3267, -816, -3227, 11560, 1881, -926, 1928, 5340 ] ),
    ];
}

/// Panasonic Rw2 File
pub struct Rw2File {
    reader: Rc<Viewer>,
    type_id: OnceCell<TypeId>,
    container: OnceCell<tiff::Container>,
    thumbnails: OnceCell<Vec<(u32, thumbnail::ThumbDesc)>>,
}

impl Rw2File {
    pub fn factory(reader: Box<dyn ReadAndSeek>) -> Box<dyn RawFile> {
        let viewer = Viewer::new(reader, 0);
        Box::new(Rw2File {
            reader: viewer,
            type_id: OnceCell::new(),
            container: OnceCell::new(),
            thumbnails: OnceCell::new(),
        })
    }

    /// Will identify the magic header for Panasonic and return the endian
    /// Panasonic slightly change over the standard TIFF header.
    fn is_magic_header(buf: &[u8]) -> Result<Endian> {
        if buf.len() < 4 {
            log::error!(
                "Panasonic magic header buffer too small: {} bytes",
                buf.len()
            );
            return Err(Error::BufferTooSmall);
        }

        if &buf[0..4] == b"IIU\0" {
            Ok(Endian::Little)
        } else {
            log::error!("Incorrect Panasonic IFD magic: {:?}", buf);
            Err(Error::FormatError)
        }
    }

    fn jpeg_data_offset(&self) -> Option<thumbnail::DataOffset> {
        self.ifd(tiff::IfdType::Main).and_then(|dir| {
            dir.entry(exif::RW2_TAG_JPEG_FROM_RAW).and_then(|entry| {
                let offset = entry.offset()? as u64;
                let len = entry.count as u64;
                Some(thumbnail::DataOffset { offset, len })
            })
        })
    }

    fn jpeg_preview(&self) -> Result<jpeg::Container> {
        self.jpeg_data_offset()
            .ok_or(Error::NotFound)
            .and_then(|offset| {
                let view = Viewer::create_view(&self.reader, offset.offset)?;
                Ok(jpeg::Container::new(view, self.type_()))
            })
    }
}

impl RawFileImpl for Rw2File {
    fn identify_id(&self) -> TypeId {
        *self.type_id.get_or_init(|| {
            self.container();
            let container = self.container.get().unwrap();
            tiff::identify_with_exif(container, &MAKE_TO_ID_MAP)
                .unwrap_or(TypeId(vendor::PANASONIC, 0))
        })
    }

    fn container(&self) -> &dyn RawContainer {
        self.container.get_or_init(|| {
            // XXX we should be faillible here.
            let view = Viewer::create_view(&self.reader, 0).expect("Created view");
            let mut container = tiff::Container::new(
                view,
                vec![
                    tiff::IfdType::Main,
                    tiff::IfdType::Other,
                    tiff::IfdType::Other,
                    tiff::IfdType::Other,
                ],
                self.type_(),
            );
            container
                .load(Some(Self::is_magic_header))
                .expect("Rw2 container error");
            container
        })
    }

    fn thumbnails(&self) -> &Vec<(u32, thumbnail::ThumbDesc)> {
        self.thumbnails.get_or_init(|| {
            let mut thumbnails = vec![];
            if let Ok(jpeg) = self.jpeg_preview() {
                if let Some(jpeg_offset) = self.jpeg_data_offset() {
                    // The JPEG preview has a preview.
                    jpeg.exif().and_then(|exif| {
                        let dir = exif.directory(1)?;
                        let len =
                            dir.value::<u32>(exif::EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LENGTH)? as u64;
                        let offset =
                            dir.value::<u32>(exif::EXIF_TAG_JPEG_INTERCHANGE_FORMAT)? as u64;
                        // XXX this +12 should be "calculated"
                        let offset = jpeg_offset.offset + offset + 12;
                        // XXX as a shortcut we assume it's Exif 160x120
                        thumbnails.push((
                            160,
                            thumbnail::ThumbDesc {
                                width: 160,
                                height: 120,
                                data_type: DataType::Jpeg,
                                data: thumbnail::Data::Offset(thumbnail::DataOffset {
                                    offset,
                                    len,
                                }),
                            },
                        ));

                        Some(())
                    });

                    // The JPEG is a large preview. Get that too.
                    let width = jpeg.width() as u32;
                    let height = jpeg.height() as u32;
                    let dim = std::cmp::max(width, height);
                    thumbnails.push((
                        dim,
                        thumbnail::ThumbDesc {
                            width,
                            height,
                            data_type: DataType::Jpeg,
                            data: thumbnail::Data::Offset(jpeg_offset),
                        },
                    ));
                }
            }

            thumbnails
        })
    }

    fn ifd(&self, ifd_type: tiff::IfdType) -> Option<Rc<Dir>> {
        self.container();
        let container = self.container.get().unwrap();
        match ifd_type {
            tiff::IfdType::Main | tiff::IfdType::Raw => container.directory(0),
            tiff::IfdType::Exif => self
                .jpeg_preview()
                .ok()
                .and_then(|jpeg| jpeg.exif().and_then(|exif| exif.directory(0)))
                .or_else(|| {
                    container
                        .directory(0)
                        .and_then(|dir| dir.get_exif_ifd(container))
                }),
            tiff::IfdType::MakerNote => self
                .jpeg_preview()
                .ok()
                .and_then(|jpeg| jpeg.exif().and_then(|exif| exif.mnote_dir())),
            _ => None,
        }
    }

    fn load_rawdata(&self, _skip_decompress: bool) -> Result<RawData> {
        if let Some(cfa) = self.ifd(tiff::IfdType::Raw) {
            let offset: thumbnail::DataOffset =
                if let Some(offset) = cfa.uint_value(exif::RW2_TAG_RAW_OFFSET) {
                    if offset as u64 > self.reader.length() {
                        return Err(Error::FormatError);
                    }
                    let len = self.reader.length() - offset as u64;
                    log::debug!("Panasonic Raw offset: {}", offset);
                    thumbnail::DataOffset {
                        offset: offset as u64,
                        len,
                    }
                } else {
                    let offset = cfa
                        .uint_value(exif::EXIF_TAG_STRIP_OFFSETS)
                        .ok_or(Error::NotFound)? as u64;
                    let len = cfa
                        .uint_value(exif::EXIF_TAG_STRIP_BYTE_COUNTS)
                        .ok_or(Error::NotFound)? as u64;
                    log::debug!("Panasonic TIFF Raw offset: {} {} bytes", offset, len);
                    thumbnail::DataOffset { offset, len }
                };
            let width = cfa
                .uint_value(exif::RW2_TAG_SENSOR_WIDTH)
                .ok_or(Error::NotFound)?;
            let height = cfa
                .uint_value(exif::RW2_TAG_SENSOR_HEIGHT)
                .ok_or(Error::NotFound)?;
            let bpc = cfa
                .value::<u16>(exif::RW2_TAG_IMAGE_BITSPERSAMPLE)
                .unwrap_or(16);
            let pixel_count = width.checked_mul(height).ok_or_else(|| {
                log::error!("Panasonic: dimensions too large");
                Error::FormatError
            })?;

            let mosaic_pattern =
                cfa.value::<u16>(exif::RW2_TAG_IMAGE_CFAPATTERN)
                    .map(|p| match p {
                        1 => Pattern::Rggb,
                        2 => Pattern::Grbg,
                        3 => Pattern::Gbrg,
                        4 => Pattern::Bggr,
                        _ => Pattern::default(),
                    });

            // in the case of TIFF Raw offset, the byte count is incorrect
            if offset.offset > self.reader.length() {
                log::error!("RW2: wanting to read past the EOF");
                return Err(Error::FormatError);
            }
            let real_len = self.reader.length() - offset.offset;
            log::debug!("real_len {} width {} height {}", real_len, width, height);
            let mut packed = false;
            let data_type = if real_len > (pixel_count * 2) as u64 {
                DataType::Raw
            } else if real_len > (pixel_count * 3 / 2) as u64 {
                // Need to unpack
                packed = true;
                DataType::Raw
            } else {
                DataType::CompressedRaw
            };
            let mut raw_data = match data_type {
                DataType::CompressedRaw => {
                    let raw = self.container().load_buffer8(offset.offset, offset.len);
                    RawData::new8(
                        width,
                        height,
                        bpc,
                        data_type,
                        raw,
                        mosaic_pattern.unwrap_or_default(),
                    )
                }
                DataType::Raw => {
                    let raw = if packed {
                        let raw = self.container().load_buffer8(offset.offset, offset.len);
                        let mut out = Vec::with_capacity(width as usize * height as usize);
                        decompress::unpack_be12to16(&raw, &mut out, tiff::Compression::None)?;
                        out
                    } else {
                        self.container().load_buffer16(offset.offset, offset.len)
                    };
                    RawData::new16(
                        width,
                        height,
                        bpc,
                        data_type,
                        raw,
                        mosaic_pattern.unwrap_or_default(),
                    )
                }
                _ => return Err(Error::NotFound),
            };
            if let Some(compression) = cfa.value::<u16>(exif::RW2_TAG_IMAGE_COMPRESSION) {
                raw_data.set_compression((compression as u32).into());
            }
            let x = cfa
                .value::<u16>(exif::RW2_TAG_SENSOR_LEFTBORDER)
                .unwrap_or(0) as i32;
            let y = cfa
                .value::<u16>(exif::RW2_TAG_SENSOR_TOPBORDER)
                .unwrap_or(0) as i32;
            let mut h = cfa
                .value::<u16>(exif::RW2_TAG_SENSOR_BOTTOMBORDER)
                .unwrap_or(0) as i32;
            h -= y;
            if h < 0 {
                h = 0;
            }

            let mut w = cfa
                .value::<u16>(exif::RW2_TAG_SENSOR_RIGHTBORDER)
                .unwrap_or(0) as i32;
            w -= x;
            if w < 0 {
                w = 0;
            }

            raw_data.set_active_area(Some(bitmap::Rect {
                x: x as u32,
                y: y as u32,
                width: w as u32,
                height: h as u32,
            }));

            Ok(raw_data)
        } else {
            Err(Error::NotFound)
        }
    }

    fn get_builtin_colour_matrix(&self) -> Result<Vec<f64>> {
        MATRICES
            .iter()
            .find(|m| m.camera == self.type_id())
            .map(|m| Vec::from(m.matrix))
            .ok_or(Error::NotFound)
    }
}

impl RawFile for Rw2File {
    fn type_(&self) -> Type {
        Type::Rw2
    }
}

impl Dump for Rw2File {
    #[cfg(feature = "dump")]
    fn write_dump<W>(&self, out: &mut W, indent: u32)
    where
        W: std::io::Write + ?Sized,
    {
        dump_writeln!(out, indent, "<Panasonic RW2 File>");
        {
            let indent = indent + 1;
            self.container();
            self.container.get().unwrap().write_dump(out, indent);
        }
        dump_writeln!(out, indent, "</Panasonic RW2 File>");
    }
}

dumpfile_impl!(Rw2File);
