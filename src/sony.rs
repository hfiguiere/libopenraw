// SPDX-License-Identifier: LGPL-3.0-or-later
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
use std::rc::Rc;

use once_cell::unsync::OnceCell;

use crate::camera_ids::{hasselblad, sony, vendor};
use crate::colour::BuiltinMatrix;
use crate::container::RawContainer;
use crate::io::Viewer;
use crate::rawfile::ReadAndSeek;
use crate::thumbnail;
use crate::tiff;
use crate::tiff::exif;
use crate::tiff::Dir;
use crate::{Dump, Error, RawData, RawFile, RawFileImpl, Result, Type, TypeId};

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
        (0x2032, "Shadows"),
        (0x2033, "Highlights"),
        (0x2034, "Fade"),
        (0x2035, "SharpnessRange"),
        (0x2036, "Clarity"),
        (0x2037, "FocusFrameSize"),
        (0x2039, "JPEG-HEIFSwitch"),
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
        (0x9416, "Sony_0x9416"),
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

    static ref SONY_MODEL_ID_MAP: HashMap<u32, TypeId> = HashMap::from([
        /* source: https://exiftool.org/TagNames/Sony.html */
        /* SR2 */
        (2, TypeId(vendor::SONY, sony::R1)),
        /* ARW */
        (256, TypeId(vendor::SONY, sony::A100)),
        (257, TypeId(vendor::SONY, sony::A900)),
        (258, TypeId(vendor::SONY, sony::A700)),
        (259, TypeId(vendor::SONY, sony::A200)),
        (260, TypeId(vendor::SONY, sony::A350)),
        (261, TypeId(vendor::SONY, sony::A300)),
        // 262 DSLR-A900 (APS-C mode)
        (263, TypeId(vendor::SONY, sony::A380)),
        (263, TypeId(vendor::SONY, sony::A390)),
        (264, TypeId(vendor::SONY, sony::A330)),
        (265, TypeId(vendor::SONY, sony::A230)),
        (266, TypeId(vendor::SONY, sony::A290)),
        (269, TypeId(vendor::SONY, sony::A850)),
        // 270 DSLR-A850 (APS-C mode)
        (273, TypeId(vendor::SONY, sony::A550)),
        (274, TypeId(vendor::SONY, sony::A500)),
        (275, TypeId(vendor::SONY, sony::A450)),
        (278, TypeId(vendor::SONY, sony::NEX5)),
        (279, TypeId(vendor::SONY, sony::NEX3)),
        (280, TypeId(vendor::SONY, sony::SLTA33)),
        (281, TypeId(vendor::SONY, sony::SLTA55)),
        (282, TypeId(vendor::SONY, sony::A560)),
        (283, TypeId(vendor::SONY, sony::A580)),
        (284, TypeId(vendor::SONY, sony::NEXC3)),
        (285, TypeId(vendor::SONY, sony::SLTA35)),
        (286, TypeId(vendor::SONY, sony::SLTA65)),
        (287, TypeId(vendor::SONY, sony::SLTA77)),
        (288, TypeId(vendor::SONY, sony::NEX5N)),
        (289, TypeId(vendor::SONY, sony::NEX7)),
        // 290 NEX-VG20E
        (291, TypeId(vendor::SONY, sony::SLTA37)),
        (292, TypeId(vendor::SONY, sony::SLTA57)),
        (293, TypeId(vendor::SONY, sony::NEXF3)),
        (294, TypeId(vendor::SONY, sony::SLTA99)),
        (295, TypeId(vendor::SONY, sony::NEX6)),
        (296, TypeId(vendor::SONY, sony::NEX5R)),
        (297, TypeId(vendor::SONY, sony::RX100)),
        (298, TypeId(vendor::SONY, sony::RX1)),
        // 299 NEX-VG900
        // 300 NEX-VG30E
        (302, TypeId(vendor::SONY, sony::ILCE3000)),
        (303, TypeId(vendor::SONY, sony::SLTA58)),
        (305, TypeId(vendor::SONY, sony::NEX3N)),
        (306, TypeId(vendor::SONY, sony::ILCE7)),
        (307, TypeId(vendor::SONY, sony::NEX5T)),
        (308, TypeId(vendor::SONY, sony::RX100M2)),
        (309, TypeId(vendor::SONY, sony::RX10)),
        (310, TypeId(vendor::SONY, sony::RX1R)),
        (311, TypeId(vendor::SONY, sony::ILCE7R)),
        (312, TypeId(vendor::SONY, sony::ILCE6000)),
        (313, TypeId(vendor::SONY, sony::ILCE5000)),
        (317, TypeId(vendor::SONY, sony::RX100M3)),
        (318, TypeId(vendor::SONY, sony::ILCE7S)),
        (319, TypeId(vendor::SONY, sony::ILCA77M2)),
        (339, TypeId(vendor::SONY, sony::ILCE5100)),
        (340, TypeId(vendor::SONY, sony::ILCE7M2)),
        (341, TypeId(vendor::SONY, sony::RX100M4)),
        (342, TypeId(vendor::SONY, sony::RX10M2)),
        (344, TypeId(vendor::SONY, sony::RX1RM2)),
        (346, TypeId(vendor::SONY, sony::ILCEQX1)),
        (347, TypeId(vendor::SONY, sony::ILCE7RM2)),
        (350, TypeId(vendor::SONY, sony::ILCE7SM2)),
        (353, TypeId(vendor::SONY, sony::ILCA68)),
        (354, TypeId(vendor::SONY, sony::ILCA99M2)),
        (355, TypeId(vendor::SONY, sony::RX10M3)),
        (356, TypeId(vendor::SONY, sony::RX100M5)),
        (357, TypeId(vendor::SONY, sony::ILCE6300)),
        (358, TypeId(vendor::SONY, sony::ILCE9)),
        (360, TypeId(vendor::SONY, sony::ILCE6500)),
        (362, TypeId(vendor::SONY, sony::ILCE7RM3)),
        (363, TypeId(vendor::SONY, sony::ILCE7M3)),
        (364, TypeId(vendor::SONY, sony::RX0)),
        (365, TypeId(vendor::SONY, sony::RX10M4)),
        (366, TypeId(vendor::SONY, sony::RX100M6)),
        (367, TypeId(vendor::SONY, sony::HX99)),
        (369, TypeId(vendor::SONY, sony::RX100M5A)),
        (371, TypeId(vendor::SONY, sony::ILCE6400)),
        (372, TypeId(vendor::SONY, sony::RX0M2)),
        (373, TypeId(vendor::SONY, sony::HX95)),
        (374, TypeId(vendor::SONY, sony::RX100M7)),
        (375, TypeId(vendor::SONY, sony::ILCE7RM4)),
        (376, TypeId(vendor::SONY, sony::ILCE9M2)),
        (378, TypeId(vendor::SONY, sony::ILCE6600)),
        (379, TypeId(vendor::SONY, sony::ILCE6100)),
        (380, TypeId(vendor::SONY, sony::ZV1)),
        (381, TypeId(vendor::SONY, sony::ILCE7C)),
        (382, TypeId(vendor::SONY, sony::ZVE10)),
        (383, TypeId(vendor::SONY, sony::ILCE7SM3)),
        (384, TypeId(vendor::SONY, sony::ILCE1)),
        (386, TypeId(vendor::SONY, sony::ILCE7RM3A)),
        (387, TypeId(vendor::SONY, sony::ILCE7RM4A)),
        (388, TypeId(vendor::SONY, sony::ILCE7M4)),
    ]);

    static ref MAKE_TO_ID_MAP: tiff::MakeToIdMap = HashMap::from([
        ("DSLR-A100", TypeId(vendor::SONY, sony::A100)),
        ("DSLR-A200", TypeId(vendor::SONY, sony::A200)),
        ("DSLR-A230", TypeId(vendor::SONY, sony::A230)),
        ("DSLR-A290", TypeId(vendor::SONY, sony::A290)),
        ("DSLR-A300", TypeId(vendor::SONY, sony::A300)),
        ("DSLR-A330", TypeId(vendor::SONY, sony::A330)),
        ("DSLR-A350", TypeId(vendor::SONY, sony::A350)),
        ("DSLR-A380", TypeId(vendor::SONY, sony::A380)),
        ("DSLR-A390", TypeId(vendor::SONY, sony::A390)),
        ("DSLR-A450", TypeId(vendor::SONY, sony::A450)),
        ("DSLR-A500", TypeId(vendor::SONY, sony::A500)),
        ("DSLR-A550", TypeId(vendor::SONY, sony::A550)),
        ("DSLR-A560", TypeId(vendor::SONY, sony::A560)),
        ("DSLR-A580", TypeId(vendor::SONY, sony::A580)),
        ("DSLR-A700", TypeId(vendor::SONY, sony::A700)),
        ("DSLR-A850", TypeId(vendor::SONY, sony::A850)),
        ("DSLR-A900", TypeId(vendor::SONY, sony::A900)),
        ("SLT-A33", TypeId(vendor::SONY, sony::SLTA33)),
        ("SLT-A35", TypeId(vendor::SONY, sony::SLTA35)),
        ("SLT-A37", TypeId(vendor::SONY, sony::SLTA37)),
        ("SLT-A55V", TypeId(vendor::SONY, sony::SLTA55)),
        ("SLT-A57", TypeId(vendor::SONY, sony::SLTA57)),
        ("SLT-A58", TypeId(vendor::SONY, sony::SLTA58)),
        ("SLT-A65V", TypeId(vendor::SONY, sony::SLTA65)),
        ("SLT-A77V", TypeId(vendor::SONY, sony::SLTA77)),
        ("SLT-A99V", TypeId(vendor::SONY, sony::SLTA99)),
        ("NEX-3", TypeId(vendor::SONY, sony::NEX3)),
        ("NEX-3N", TypeId(vendor::SONY, sony::NEX3N)),
        ("NEX-5", TypeId(vendor::SONY, sony::NEX5)),
        ("NEX-5N", TypeId(vendor::SONY, sony::NEX5N)),
        ("NEX-5R", TypeId(vendor::SONY, sony::NEX5R)),
        ("NEX-5T", TypeId(vendor::SONY, sony::NEX5T)),
        ("NEX-C3", TypeId(vendor::SONY, sony::NEXC3)),
        ("NEX-F3", TypeId(vendor::SONY, sony::NEXF3)),
        ("NEX-6", TypeId(vendor::SONY, sony::NEX6)),
        ("NEX-7", TypeId(vendor::SONY, sony::NEX7)),
        ("DSC-HX95", TypeId(vendor::SONY, sony::HX95)),
        ("DSC-HX99", TypeId(vendor::SONY, sony::HX99)),
        ("DSC-R1", TypeId(vendor::SONY, sony::R1)),
        ("DSC-RX10", TypeId(vendor::SONY, sony::RX10)),
        ("DSC-RX10M2", TypeId(vendor::SONY, sony::RX10M2)),
        ("DSC-RX10M3", TypeId(vendor::SONY, sony::RX10M3)),
        ("DSC-RX10M4", TypeId(vendor::SONY, sony::RX10M4)),
        ("DSC-RX100", TypeId(vendor::SONY, sony::RX100)),
        ("DSC-RX100M2", TypeId(vendor::SONY, sony::RX100M2)),
        ("DSC-RX100M3", TypeId(vendor::SONY, sony::RX100M3)),
        ("DSC-RX100M4", TypeId(vendor::SONY, sony::RX100M4)),
        ("DSC-RX100M5", TypeId(vendor::SONY, sony::RX100M5)),
        ("DSC-RX100M5A", TypeId(vendor::SONY, sony::RX100M5A)),
        ("DSC-RX100M6", TypeId(vendor::SONY, sony::RX100M6)),
        ("DSC-RX100M7", TypeId(vendor::SONY, sony::RX100M7)),
        ("DSC-RX0", TypeId(vendor::SONY, sony::RX0)),
        ("DSC-RX0M2", TypeId(vendor::SONY, sony::RX0M2)),
        ("DSC-RX1", TypeId(vendor::SONY, sony::RX1)),
        ("DSC-RX1R", TypeId(vendor::SONY, sony::RX1R)),
        ("DSC-RX1RM2", TypeId(vendor::SONY, sony::RX1RM2)),
        ("ILCA-68", TypeId(vendor::SONY, sony::ILCA68)),
        ("ILCA-77M2", TypeId(vendor::SONY, sony::ILCA77M2)),
        ("ILCA-99M2", TypeId(vendor::SONY, sony::ILCA99M2)),
        ("ILCE-1", TypeId(vendor::SONY, sony::ILCE1)),
        ("ILCE-3000", TypeId(vendor::SONY, sony::ILCE3000)),
        ("ILCE-3500", TypeId(vendor::SONY, sony::ILCE3500)),
        ("ILCE-5000", TypeId(vendor::SONY, sony::ILCE5000)),
        ("ILCE-5100", TypeId(vendor::SONY, sony::ILCE5100)),
        ("ILCE-6000", TypeId(vendor::SONY, sony::ILCE6000)),
        ("ILCE-6100", TypeId(vendor::SONY, sony::ILCE6100)),
        ("ILCE-6300", TypeId(vendor::SONY, sony::ILCE6300)),
        ("ILCE-6400", TypeId(vendor::SONY, sony::ILCE6400)),
        ("ILCE-6500", TypeId(vendor::SONY, sony::ILCE6500)),
        ("ILCE-6600", TypeId(vendor::SONY, sony::ILCE6600)),
        ("ILCE-7", TypeId(vendor::SONY, sony::ILCE7)),
        ("ILCE-7C", TypeId(vendor::SONY, sony::ILCE7C)),
        ("ILCE-7M2", TypeId(vendor::SONY, sony::ILCE7M2)),
        ("ILCE-7M3", TypeId(vendor::SONY, sony::ILCE7M3)),
        ("ILCE-7M4", TypeId(vendor::SONY, sony::ILCE7M4)),
        ("ILCE-7R", TypeId(vendor::SONY, sony::ILCE7R)),
        ("ILCE-7RM2", TypeId(vendor::SONY, sony::ILCE7RM2)),
        ("ILCE-7RM3", TypeId(vendor::SONY, sony::ILCE7RM3)),
        ("ILCE-7RM3A", TypeId(vendor::SONY, sony::ILCE7RM3A)),
        ("ILCE-7RM4", TypeId(vendor::SONY, sony::ILCE7RM4)),
        ("ILCE-7RM4A", TypeId(vendor::SONY, sony::ILCE7RM4A)),
        ("ILCE-7S", TypeId(vendor::SONY, sony::ILCE7S)),
        ("ILCE-7SM2", TypeId(vendor::SONY, sony::ILCE7SM2)),
        ("ILCE-7SM3", TypeId(vendor::SONY, sony::ILCE7SM3)),
        ("ILCE-9", TypeId(vendor::SONY, sony::ILCE9)),
        ("ILCE-9M2", TypeId(vendor::SONY, sony::ILCE9M2)),
        ("ZV-1", TypeId(vendor::SONY, sony::ZV1)),
        ("ZV-E10", TypeId(vendor::SONY, sony::ZVE10)),
        ("Lunar", TypeId(vendor::HASSELBLAD, hasselblad::LUNAR)),
    ]);

    static ref MATRICES: [BuiltinMatrix; 88] = [
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::A100),
            0,
            0xfeb,
            [ 9437, -2811, -774, -8405, 16215, 2290, -710, 596, 7181 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::A200),
            0,
            0,
            [ 9847, -3091, -928, -8485, 16345, 2225, -715, 595, 7103 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::A230),
            0,
            0,
            [ 9847, -3091, -928, -8485, 16345, 2225, -715, 595, 7103 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::A290),
            0,
            0,
            [ 6038, -1484, -579, -9145, 16746, 2512, -875, 746, 7218 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::A300),
            0,
            0,
            [ 9847, -3091, -928, -8485, 16345, 2225, -715, 595, 7103 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::A330),
            0,
            0,
            [ 9847, -3091, -928, -8485, 16345, 2225, -715, 595, 7103 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::A350),
            0,
            0,
            [ 6038, -1484, -579, -9145, 16746, 2512, -875, 746, 7218 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::A380),
            0,
            0,
            [ 6038, -1484, -579, -9145, 16746, 2512, -875, 746, 7218 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::A450),
            128,
            0xfeb,
            [ 4950, -580, -103, -5228, 12542, 3029, -709, 1435, 7371 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::A500),
            0,
            0,
            [ 6046, -1127, -278, -5574, 13076, 2786, -691, 1419, 7625 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::A550),
            128,
            0xfeb,
            [ 4950, -580, -103, -5228, 12542, 3029, -709, 1435, 7371 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::A560),
            128,
            0xfeb,
            [ 4950, -580, -103, -5228, 12542, 3029, -709, 1435, 7371 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::A580),
            128,
            0,
            [ 5932, -1492, -411, -4813, 12285, 2856, -741, 1524, 6739 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::A700),
            126,
            0,
            [ 5775, -805, -359, -8574, 16295, 2391, -1943, 2341, 7249 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::A850),
            128,
            0,
            [ 5413, -1162, -365, -5665, 13098, 2866, -608, 1179, 8440 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::A900),
            128,
            0,
            [ 5209, -1072, -397, -8845, 16120, 2919, -1618, 1803, 8654 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::SLTA33),
            128,
            0,
            [ 6069, -1221, -366, -5221, 12779, 2734, -1024, 2066, 6834 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::SLTA35),
            128,
            0,
            [ 5986, -1618, -415, -4557, 11820, 3120, -681, 1404, 6971 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::SLTA37),
            128,
            0,
            [ 5991, -1456, -455, -4764, 12135, 2980, -707, 1425, 6701 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::SLTA55),
            128,
            0,
            [ 5932, -1492, -411, -4813, 12285, 2856, -741, 1524, 6739 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::SLTA57),
            128,
            0,
            [ 5991, -1456, -455, -4764, 12135, 2980, -707, 1425, 6701 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::SLTA58),
            128,
            0,
            [ 5991, -1456, -455, -4764, 12135, 2980, -707, 1425, 6701 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::SLTA65),
            128,
            0,
            [ 5491, -1192, -363, -4951, 12342, 2948, -911, 1722, 7192 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ILCA68),
            128,
            0,
            [ 6435, -1903, -536, -4722, 12449, 2550, -663, 1363, 6517 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::SLTA77),
            128,
            0,
            [ 5491, -1192, -363, -4951, 12342, 2948, -911, 1722, 7192 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ILCA77M2),
            128,
            0,
            [ 5991, -1732, -443, -4100, 11989, 2381, -704, 1467, 5992 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::SLTA99),
            0,
            0,
            [ 6344, -1612, -462, -4863, 12477, 2681, -865, 1786, 6899 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ILCA99M2),
            0,
            0,
            [ 6660, -1918, -471, -4613, 12398, 2485, -649, 1433, 6447 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::NEX3),
            128,
            0,
            [ 6549, -1550, -436, -4880, 12435, 2753, -854, 1868, 6976 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::HX95),
            0,
            0,
            [ 13076, -5686, -1481, -4027, 12851, 1251, -167, 725, 4937 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::HX99),
            0,
            0,
            [ 13076, -5686, -1481, -4027, 12851, 1251, -167, 725, 4937 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::NEX3N),
            128,
            0,
            [ 6129, -1545, -418, -4930, 12490, 2743, -977, 1693, 6615 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::NEX5),
            128,
            0,
            [ 6549, -1550, -436, -4880, 12435, 2753, -854, 1868, 6976 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::NEX5N),
            128,
            0,
            [ 5991, -1456, -455, -4764, 12135, 2980, -707, 1425, 6701 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::NEX5R),
            128,
            0,
            [ 6129, -1545, -418, -4930, 12490, 2743, -977, 1693, 6615 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::NEX5T),
            128,
            0,
            [ 6129, -1545, -418, -4930, 12490, 2743, -977, 1693, 6615 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::NEXC3),
            128,
            0,
            [ 5991, -1456, -455, -4764, 12135, 2980, -707, 1425, 6701 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::NEXF3),
            128,
            0,
            [ 5991, -1456, -455, -4764, 12135, 2980, -707, 1425, 6701 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::NEX6),
            128,
            0,
            [ 6129, -1545, -418, -4930, 12490, 2743, -977, 1693, 6615 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::NEX7),
            128,
            0,
            [ 5491, -1192, -363, -4951, 12342, 2948, -911, 1722, 7192 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::R1),
            0,
            0,
            [ 8512, -2641, -694, -8041, 15670, 2526, -1820, 2117, 7414 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::RX100),
            0,
            0,
            [ 8651, -2754, -1057, -3464, 12207, 1373, -568, 1398, 4434 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::RX100M2),
            0,
            0,
            [ 6596, -2079, -562, -4782, 13016, 1933, -970, 1581, 5181 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::RX100M3),
            0,
            0,
            [ 6596, -2079, -562, -4782, 13016, 1933, -970, 1581, 5181 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::RX100M4),
            0,
            0,
            [ 6596, -2079, -562, -4782, 13016, 1933, -970, 1581, 5181 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::RX100M5),
            0,
            0,
            [ 6596, -2079, -562, -4782, 13016, 1933, -970, 1581, 5181 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::RX100M5A),
            0,
            0,
            [ 11176, -4700, -965, -4004, 12184, 2032, -763, 1726, 5876 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::RX100M6),
            0,
            0,
            [ 7325, -2321, -596, -3494, 11674, 2055, -668, 1562, 5031 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::RX100M7),
            0,
            0,
            [ 7325, -2321, -596, -3494, 11674, 2055, -668, 1562, 5031 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::RX0),
            0,
            0,
            [ 9396, -3507, -843, -2497, 11111, 1572, -343, 1355, 5089 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::RX0M2),
            0,
            0,
            [ 9396, -3507, -843, -2497, 11111, 1572, -343, 1355, 5089 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::RX1),
            0,
            0,
            [ 6344, -1612, -462, -4863, 12477, 2681, -865, 1786, 6899 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::RX1R),
            0,
            0,
            [ 6344, -1612, -462, -4863, 12477, 2681, -865, 1786, 6899 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::RX1RM2),
            0,
            0,
            [ 6629, -1900, -483, -4618, 12349, 2550, -622, 1381, 6514 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::RX10),
            0,
            0,
            [ 6679, -1825, -745, -5047, 13256, 1953, -1580, 2422, 5183 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::RX10M2),
            0,
            0,
            [ 6679, -1825, -745, -5047, 13256, 1953, -1580, 2422, 5183 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::RX10M3),
            0,
            0,
            [ 6679, -1825, -745, -5047, 13256, 1953, -1580, 2422, 5183 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::RX10M4),
            0,
            0,
            [ 7699, -2566, -629, -2967, 11270, 1928, -378, 1286, 4807 ] ),

        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ILCE1),
            128,
            0,
            [ 8161, -2947, -739, -4811, 12668, 2389, -437, 1229, 6524 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ILCE3000),
            128,
            0,
            [ 5991, -1456, -455, -4764, 12135, 2980, -707, 1425, 6701 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ILCE5000),
            128,
            0,
            [ 5991, -1456, -455, -4764, 12135, 2980, -707, 1425, 6701 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ILCE5100),
            128,
            0,
            [ 5991, -1456, -455, -4764, 12135, 2980, -707, 1425, 6701 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ILCE6000),
            128,
            0,
            [ 5991, -1456, -455, -4764, 12135, 2980, -707, 1425, 6701 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ILCE6100),
            128,
            0,
            [ 7657, -2847, -607, -4083, 11966, 2389, -684, 1418, 5844 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ILCE6300),
            0,
            0,
            [ 5973, -1695, -419, -3826, 11797, 2293, -639, 1398, 5789 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ILCE6400),
            0,
            0,
            [ 5973, -1695, -419, -3826, 11797, 2293, -639, 1398, 5789 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ILCE6500),
            0,
            0,
            [ 5973, -1695, -419, -3826, 11797, 2293, -639, 1398, 5789 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ILCE6600),
            128,
            0,
            [ 7657, -2847, -607, -4083, 11966, 2389, -684, 1418, 5844 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ILCE7),
            128,
            0,
            [ 5271, -712, -347, -6153, 13653, 2763, -1601, 2366, 7242 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ILCE7M2),
            128,
            0,
            [ 5271, -712, -347, -6153, 13653, 2763, -1601, 2366, 7242 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ILCE7M3),
            128,
            0,
            [ 7374, -2389, -551, -5435, 13162, 2519, -1006, 1795, 6552 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ILCE7M4),
            128,
            0,
            [ 7460, -2365, -588, -5687, 13442, 2474, -624, 1156, 6584 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ILCE7R),
            128,
            0,
            [ 4913, -541, -202, -6130, 13513, 2906, -1564, 2151, 7183 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ILCE7RM2),
            0,
            0,
            [ 6629, -1900, -483, -4618, 12349, 2550, -622, 1381, 6514 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ILCE7RM3),
            0,
            0,
            [ 6640, -1847, -503, -5238, 13010, 2474, -993, 1673, 6527 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ILCE7RM3A),
            0,
            0,
            [ 6640, -1847, -503, -5238, 13010, 2474, -993, 1673, 6527 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ILCE7RM4),
            0,
            0,
            [ 6640, -1847, -503, -5238, 13010, 2474, -993, 1673, 6527 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ILCE7RM4A),
            0,
            0,
            [ 7662, -2686, -660, -5240, 12965, 2530, -796, 1508, 6167 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ILCE7S),
            128,
            0,
            [ 5838, -1430, -246, -3497, 11477, 2297, -748, 1885, 5778 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ILCE7SM2),
            128,
            0,
            [ 5838, -1430, -246, -3497, 11477, 2297, -748, 1885, 5778 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ILCE7SM3),
            128,
            0,
            [ 6912, -2127, -469, -4470, 12175, 2587, -398, 1478, 6492 ] ),

        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ILCE7C),
            128,
            0,
            [ 7374, -2389, -551, -5435, 13162, 2519, -1006, 1795, 6552 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ILCE9),
            128,
            0,
            [ 6389, -1703, -378, -4562, 12265, 2587, -670, 1489, 6550 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ILCE9M2),
            128,
            0,
            [ 6389, -1703, -378, -4562, 12265, 2587, -670, 1489, 6550 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ILCEQX1),
            128,
            0,
            [ 5991, -1456, -455, -4764, 12135, 2980, -707, 1425, 6701 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ZV1),
            128,
            0,
            [ 8280, -2987, -703, -3531, 11645, 2133, -550, 1542, 5312 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::SONY, sony::ZVE10),
            128,
            0,
            [ 6355, -2067, -490, -3653, 11542, 2400, -406, 1258, 5506 ] ),
        /* The Hasselblad Lunar is like a Nex7 */
        BuiltinMatrix::new(
            TypeId(vendor::HASSELBLAD, hasselblad::LUNAR),
            128,
            0,
            [ 5491, -1192, -363, -4951, 12342, 2948, -911, 1722, 7192 ] ),
    ];
}

pub struct ArwFile {
    reader: Rc<Viewer>,
    type_id: OnceCell<TypeId>,
    container: OnceCell<tiff::Container>,
    thumbnails: OnceCell<Vec<(u32, thumbnail::ThumbDesc)>>,
}

impl ArwFile {
    pub fn factory(reader: Box<dyn ReadAndSeek>) -> Box<dyn RawFile> {
        let viewer = Viewer::new(reader, 0);
        Box::new(ArwFile {
            reader: viewer,
            type_id: OnceCell::new(),
            container: OnceCell::new(),
            thumbnails: OnceCell::new(),
        })
    }

    fn is_a100(&self) -> bool {
        self.identify_id() == TypeId(vendor::SONY, sony::A100)
    }
}

impl RawFileImpl for ArwFile {
    fn identify_id(&self) -> TypeId {
        *self.type_id.get_or_init(|| {
            if let Some(maker_note) = self.maker_note_ifd() {
                if let Some(id) = maker_note.uint_value(exif::MNOTE_SONY_MODEL_ID) {
                    log::debug!("Sony model ID: {:x} ({})", id, id);
                    return SONY_MODEL_ID_MAP
                        .get(&id)
                        .copied()
                        .unwrap_or(TypeId(vendor::SONY, 0));
                } else {
                    log::error!("Sony model ID tag not found");
                }
            }
            // The A100 is broken we use a fallback
            // But when it's no longer broken, we might be able to get away with this
            let container = self.container.get().unwrap();
            tiff::identify_with_exif(container, &MAKE_TO_ID_MAP).unwrap_or(TypeId(vendor::SONY, 0))
        })
    }

    fn container(&self) -> &dyn RawContainer {
        self.container.get_or_init(|| {
            // XXX we should be faillible here.
            let view = Viewer::create_view(&self.reader, 0).expect("Created view");
            let mut container = tiff::Container::new(view, vec![tiff::IfdType::Main], self.type_());
            container.load(None).expect("Arw container error");
            container
        })
    }

    fn thumbnails(&self) -> &Vec<(u32, thumbnail::ThumbDesc)> {
        self.thumbnails.get_or_init(|| {
            self.container();
            let container = self.container.get().unwrap();
            tiff::tiff_thumbnails(container)
        })
    }

    fn ifd(&self, ifd_type: tiff::IfdType) -> Option<Rc<Dir>> {
        self.container();
        let container = self.container.get().unwrap();
        match ifd_type {
            tiff::IfdType::Main => container.directory(0),
            tiff::IfdType::Raw => {
                if self.is_a100() {
                    container.directory(0)
                } else {
                    tiff::tiff_locate_raw_ifd(container)
                }
            }
            tiff::IfdType::Exif => self
                .ifd(tiff::IfdType::Main)
                .and_then(|dir| dir.get_exif_ifd(container)),
            tiff::IfdType::MakerNote => self
                .ifd(tiff::IfdType::Exif)
                .and_then(|dir| dir.get_mnote_ifd(container)),
            _ => None,
        }
    }

    fn load_rawdata(&self, _skip_decompress: bool) -> Result<RawData> {
        if self.is_a100() {
            Err(Error::NotFound)
        } else {
            self.ifd(tiff::IfdType::Raw)
                .ok_or(Error::NotFound)
                .and_then(|dir| {
                    tiff::tiff_get_rawdata(self.container.get().unwrap(), &dir, self.type_())
                })
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

impl RawFile for ArwFile {
    fn type_(&self) -> Type {
        Type::Arw
    }
}

impl Dump for ArwFile {
    #[cfg(feature = "dump")]
    fn print_dump(&self, indent: u32) {
        dump_println!(indent, "<Sony ARW File>");
        {
            let indent = indent + 1;
            self.container().print_dump(indent);
        }
        dump_println!(indent, "</Sony ARW File>");
    }
}
