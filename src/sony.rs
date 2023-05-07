// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - sony.rs
 *
 * Copyright (C) 2022-2023x Hubert Figuière
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

use crate::camera_ids::{hasselblad, vendor};
use crate::colour::BuiltinMatrix;
use crate::container::RawContainer;
use crate::io::Viewer;
use crate::rawfile::ReadAndSeek;
use crate::thumbnail;
use crate::tiff;
use crate::tiff::exif;
use crate::tiff::Dir;
use crate::{Dump, Error, RawData, RawFile, RawFileImpl, Result, Type, TypeId};

macro_rules! sony {
    ($id:expr, $model:ident) => {
        (
            $id,
            TypeId(
                $crate::camera_ids::vendor::SONY,
                $crate::camera_ids::sony::$model,
            ),
        )
    };
    ($model:ident) => {
        TypeId(
            $crate::camera_ids::vendor::SONY,
            $crate::camera_ids::sony::$model,
        )
    };
}

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
        sony!(2, R1),
        /* ARW */
        sony!(256, A100),
        sony!(257, A900),
        sony!(258, A700),
        sony!(259, A200),
        sony!(260, A350),
        sony!(261, A300),
        // 262 DSLR-A900 (APS-C mode)
        sony!(263, A380),
        sony!(263, A390),
        sony!(264, A330),
        sony!(265, A230),
        sony!(266, A290),
        sony!(269, A850),
        // 270 DSLR-A850 (APS-C mode)
        sony!(273, A550),
        sony!(274, A500),
        sony!(275, A450),
        sony!(278, NEX5),
        sony!(279, NEX3),
        sony!(280, SLTA33),
        sony!(281, SLTA55),
        sony!(282, A560),
        sony!(283, A580),
        sony!(284, NEXC3),
        sony!(285, SLTA35),
        sony!(286, SLTA65),
        sony!(287, SLTA77),
        sony!(288, NEX5N),
        sony!(289, NEX7),
        // 290 NEX-VG20E
        sony!(291, SLTA37),
        sony!(292, SLTA57),
        sony!(293, NEXF3),
        sony!(294, SLTA99),
        sony!(295, NEX6),
        sony!(296, NEX5R),
        sony!(297, RX100),
        sony!(298, RX1),
        // 299 NEX-VG900
        // 300 NEX-VG30E
        sony!(302, ILCE3000),
        sony!(303, SLTA58),
        sony!(305, NEX3N),
        sony!(306, ILCE7),
        sony!(307, NEX5T),
        sony!(308, RX100M2),
        sony!(309, RX10),
        sony!(310, RX1R),
        sony!(311, ILCE7R),
        sony!(312, ILCE6000),
        sony!(313, ILCE5000),
        sony!(317, RX100M3),
        sony!(318, ILCE7S),
        sony!(319, ILCA77M2),
        sony!(339, ILCE5100),
        sony!(340, ILCE7M2),
        sony!(341, RX100M4),
        sony!(342, RX10M2),
        sony!(344, RX1RM2),
        sony!(346, ILCEQX1),
        sony!(347, ILCE7RM2),
        sony!(350, ILCE7SM2),
        sony!(353, ILCA68),
        sony!(354, ILCA99M2),
        sony!(355, RX10M3),
        sony!(356, RX100M5),
        sony!(357, ILCE6300),
        sony!(358, ILCE9),
        sony!(360, ILCE6500),
        sony!(362, ILCE7RM3),
        sony!(363, ILCE7M3),
        sony!(364, RX0),
        sony!(365, RX10M4),
        sony!(366, RX100M6),
        sony!(367, HX99),
        sony!(369, RX100M5A),
        sony!(371, ILCE6400),
        sony!(372, RX0M2),
        sony!(373, HX95),
        sony!(374, RX100M7),
        sony!(375, ILCE7RM4),
        sony!(376, ILCE9M2),
        sony!(378, ILCE6600),
        sony!(379, ILCE6100),
        sony!(380, ZV1),
        sony!(381, ILCE7C),
        sony!(382, ZVE10),
        sony!(383, ILCE7SM3),
        sony!(384, ILCE1),
        sony!(386, ILCE7RM3A),
        sony!(387, ILCE7RM4A),
        sony!(388, ILCE7M4),
        sony!(390, ILCE7RM5),
        sony!(391, ILME_FX30),
        sony!(393, ZVE1),
    ]);

    static ref MAKE_TO_ID_MAP: tiff::MakeToIdMap = HashMap::from([
        sony!("DSLR-A100", A100),
        sony!("DSLR-A200", A200),
        sony!("DSLR-A230", A230),
        sony!("DSLR-A290", A290),
        sony!("DSLR-A300", A300),
        sony!("DSLR-A330", A330),
        sony!("DSLR-A350", A350),
        sony!("DSLR-A380", A380),
        sony!("DSLR-A390", A390),
        sony!("DSLR-A450", A450),
        sony!("DSLR-A500", A500),
        sony!("DSLR-A550", A550),
        sony!("DSLR-A560", A560),
        sony!("DSLR-A580", A580),
        sony!("DSLR-A700", A700),
        sony!("DSLR-A850", A850),
        sony!("DSLR-A900", A900),
        sony!("SLT-A33", SLTA33),
        sony!("SLT-A35", SLTA35),
        sony!("SLT-A37", SLTA37),
        sony!("SLT-A55V", SLTA55),
        sony!("SLT-A57", SLTA57),
        sony!("SLT-A58", SLTA58),
        sony!("SLT-A65V", SLTA65),
        sony!("SLT-A77V", SLTA77),
        sony!("SLT-A99V", SLTA99),
        sony!("NEX-3", NEX3),
        sony!("NEX-3N", NEX3N),
        sony!("NEX-5", NEX5),
        sony!("NEX-5N", NEX5N),
        sony!("NEX-5R", NEX5R),
        sony!("NEX-5T", NEX5T),
        sony!("NEX-C3", NEXC3),
        sony!("NEX-F3", NEXF3),
        sony!("NEX-6", NEX6),
        sony!("NEX-7", NEX7),
        sony!("DSC-HX95", HX95),
        sony!("DSC-HX99", HX99),
        sony!("DSC-R1", R1),
        sony!("DSC-RX10", RX10),
        sony!("DSC-RX10M2", RX10M2),
        sony!("DSC-RX10M3", RX10M3),
        sony!("DSC-RX10M4", RX10M4),
        sony!("DSC-RX100", RX100),
        sony!("DSC-RX100M2", RX100M2),
        sony!("DSC-RX100M3", RX100M3),
        sony!("DSC-RX100M4", RX100M4),
        sony!("DSC-RX100M5", RX100M5),
        sony!("DSC-RX100M5A", RX100M5A),
        sony!("DSC-RX100M6", RX100M6),
        sony!("DSC-RX100M7", RX100M7),
        sony!("DSC-RX0", RX0),
        sony!("DSC-RX0M2", RX0M2),
        sony!("DSC-RX1", RX1),
        sony!("DSC-RX1R", RX1R),
        sony!("DSC-RX1RM2", RX1RM2),
        sony!("ILCA-68", ILCA68),
        sony!("ILCA-77M2", ILCA77M2),
        sony!("ILCA-99M2", ILCA99M2),
        sony!("ILCE-1", ILCE1),
        sony!("ILCE-3000", ILCE3000),
        sony!("ILCE-3500", ILCE3500),
        sony!("ILCE-5000", ILCE5000),
        sony!("ILCE-5100", ILCE5100),
        sony!("ILCE-6000", ILCE6000),
        sony!("ILCE-6100", ILCE6100),
        sony!("ILCE-6300", ILCE6300),
        sony!("ILCE-6400", ILCE6400),
        sony!("ILCE-6500", ILCE6500),
        sony!("ILCE-6600", ILCE6600),
        sony!("ILCE-7", ILCE7),
        sony!("ILCE-7C", ILCE7C),
        sony!("ILCE-7M2", ILCE7M2),
        sony!("ILCE-7M3", ILCE7M3),
        sony!("ILCE-7M4", ILCE7M4),
        sony!("ILCE-7R", ILCE7R),
        sony!("ILCE-7RM2", ILCE7RM2),
        sony!("ILCE-7RM3", ILCE7RM3),
        sony!("ILCE-7RM3A", ILCE7RM3A),
        sony!("ILCE-7RM4", ILCE7RM4),
        sony!("ILCE-7RM4A", ILCE7RM4A),
        sony!("ILCE-7RM5", ILCE7RM5),
        sony!("ILCE-7S", ILCE7S),
        sony!("ILCE-7SM2", ILCE7SM2),
        sony!("ILCE-7SM3", ILCE7SM3),
        sony!("ILCE-9", ILCE9),
        sony!("ILCE-9M2", ILCE9M2),
        sony!("ILME-FX30", ILME_FX30),
        sony!("ZV-1", ZV1),
        sony!("ZV-E1", ZVE1),
        sony!("ZV-E10", ZVE10),
        ("Lunar", TypeId(vendor::HASSELBLAD, hasselblad::LUNAR)),
    ]);

    static ref MATRICES: [BuiltinMatrix; 91] = [
        BuiltinMatrix::new(
            sony!(A100),
            0,
            0xfeb,
            [ 9437, -2811, -774, -8405, 16215, 2290, -710, 596, 7181 ] ),
        BuiltinMatrix::new(
            sony!(A200),
            0,
            0,
            [ 9847, -3091, -928, -8485, 16345, 2225, -715, 595, 7103 ] ),
        BuiltinMatrix::new(
            sony!(A230),
            0,
            0,
            [ 9847, -3091, -928, -8485, 16345, 2225, -715, 595, 7103 ] ),
        BuiltinMatrix::new(
            sony!(A290),
            0,
            0,
            [ 6038, -1484, -579, -9145, 16746, 2512, -875, 746, 7218 ] ),
        BuiltinMatrix::new(
            sony!(A300),
            0,
            0,
            [ 9847, -3091, -928, -8485, 16345, 2225, -715, 595, 7103 ] ),
        BuiltinMatrix::new(
            sony!(A330),
            0,
            0,
            [ 9847, -3091, -928, -8485, 16345, 2225, -715, 595, 7103 ] ),
        BuiltinMatrix::new(
            sony!(A350),
            0,
            0,
            [ 6038, -1484, -579, -9145, 16746, 2512, -875, 746, 7218 ] ),
        BuiltinMatrix::new(
            sony!(A380),
            0,
            0,
            [ 6038, -1484, -579, -9145, 16746, 2512, -875, 746, 7218 ] ),
        BuiltinMatrix::new(
            sony!(A450),
            128,
            0xfeb,
            [ 4950, -580, -103, -5228, 12542, 3029, -709, 1435, 7371 ] ),
        BuiltinMatrix::new(
            sony!(A500),
            0,
            0,
            [ 6046, -1127, -278, -5574, 13076, 2786, -691, 1419, 7625 ] ),
        BuiltinMatrix::new(
            sony!(A550),
            128,
            0xfeb,
            [ 4950, -580, -103, -5228, 12542, 3029, -709, 1435, 7371 ] ),
        BuiltinMatrix::new(
            sony!(A560),
            128,
            0xfeb,
            [ 4950, -580, -103, -5228, 12542, 3029, -709, 1435, 7371 ] ),
        BuiltinMatrix::new(
            sony!(A580),
            128,
            0,
            [ 5932, -1492, -411, -4813, 12285, 2856, -741, 1524, 6739 ] ),
        BuiltinMatrix::new(
            sony!(A700),
            126,
            0,
            [ 5775, -805, -359, -8574, 16295, 2391, -1943, 2341, 7249 ] ),
        BuiltinMatrix::new(
            sony!(A850),
            128,
            0,
            [ 5413, -1162, -365, -5665, 13098, 2866, -608, 1179, 8440 ] ),
        BuiltinMatrix::new(
            sony!(A900),
            128,
            0,
            [ 5209, -1072, -397, -8845, 16120, 2919, -1618, 1803, 8654 ] ),
        BuiltinMatrix::new(
            sony!(SLTA33),
            128,
            0,
            [ 6069, -1221, -366, -5221, 12779, 2734, -1024, 2066, 6834 ] ),
        BuiltinMatrix::new(
            sony!(SLTA35),
            128,
            0,
            [ 5986, -1618, -415, -4557, 11820, 3120, -681, 1404, 6971 ] ),
        BuiltinMatrix::new(
            sony!(SLTA37),
            128,
            0,
            [ 5991, -1456, -455, -4764, 12135, 2980, -707, 1425, 6701 ] ),
        BuiltinMatrix::new(
            sony!(SLTA55),
            128,
            0,
            [ 5932, -1492, -411, -4813, 12285, 2856, -741, 1524, 6739 ] ),
        BuiltinMatrix::new(
            sony!(SLTA57),
            128,
            0,
            [ 5991, -1456, -455, -4764, 12135, 2980, -707, 1425, 6701 ] ),
        BuiltinMatrix::new(
            sony!(SLTA58),
            128,
            0,
            [ 5991, -1456, -455, -4764, 12135, 2980, -707, 1425, 6701 ] ),
        BuiltinMatrix::new(
            sony!(SLTA65),
            128,
            0,
            [ 5491, -1192, -363, -4951, 12342, 2948, -911, 1722, 7192 ] ),
        BuiltinMatrix::new(
            sony!(ILCA68),
            128,
            0,
            [ 6435, -1903, -536, -4722, 12449, 2550, -663, 1363, 6517 ] ),
        BuiltinMatrix::new(
            sony!(SLTA77),
            128,
            0,
            [ 5491, -1192, -363, -4951, 12342, 2948, -911, 1722, 7192 ] ),
        BuiltinMatrix::new(
            sony!(ILCA77M2),
            128,
            0,
            [ 5991, -1732, -443, -4100, 11989, 2381, -704, 1467, 5992 ] ),
        BuiltinMatrix::new(
            sony!(SLTA99),
            0,
            0,
            [ 6344, -1612, -462, -4863, 12477, 2681, -865, 1786, 6899 ] ),
        BuiltinMatrix::new(
            sony!(ILCA99M2),
            0,
            0,
            [ 6660, -1918, -471, -4613, 12398, 2485, -649, 1433, 6447 ] ),
        BuiltinMatrix::new(
            sony!(NEX3),
            128,
            0,
            [ 6549, -1550, -436, -4880, 12435, 2753, -854, 1868, 6976 ] ),
        BuiltinMatrix::new(
            sony!(HX95),
            0,
            0,
            [ 13076, -5686, -1481, -4027, 12851, 1251, -167, 725, 4937 ] ),
        BuiltinMatrix::new(
            sony!(HX99),
            0,
            0,
            [ 13076, -5686, -1481, -4027, 12851, 1251, -167, 725, 4937 ] ),
        BuiltinMatrix::new(
            sony!(NEX3N),
            128,
            0,
            [ 6129, -1545, -418, -4930, 12490, 2743, -977, 1693, 6615 ] ),
        BuiltinMatrix::new(
            sony!(NEX5),
            128,
            0,
            [ 6549, -1550, -436, -4880, 12435, 2753, -854, 1868, 6976 ] ),
        BuiltinMatrix::new(
            sony!(NEX5N),
            128,
            0,
            [ 5991, -1456, -455, -4764, 12135, 2980, -707, 1425, 6701 ] ),
        BuiltinMatrix::new(
            sony!(NEX5R),
            128,
            0,
            [ 6129, -1545, -418, -4930, 12490, 2743, -977, 1693, 6615 ] ),
        BuiltinMatrix::new(
            sony!(NEX5T),
            128,
            0,
            [ 6129, -1545, -418, -4930, 12490, 2743, -977, 1693, 6615 ] ),
        BuiltinMatrix::new(
            sony!(NEXC3),
            128,
            0,
            [ 5991, -1456, -455, -4764, 12135, 2980, -707, 1425, 6701 ] ),
        BuiltinMatrix::new(
            sony!(NEXF3),
            128,
            0,
            [ 5991, -1456, -455, -4764, 12135, 2980, -707, 1425, 6701 ] ),
        BuiltinMatrix::new(
            sony!(NEX6),
            128,
            0,
            [ 6129, -1545, -418, -4930, 12490, 2743, -977, 1693, 6615 ] ),
        BuiltinMatrix::new(
            sony!(NEX7),
            128,
            0,
            [ 5491, -1192, -363, -4951, 12342, 2948, -911, 1722, 7192 ] ),
        BuiltinMatrix::new(
            sony!(R1),
            0,
            0,
            [ 8512, -2641, -694, -8041, 15670, 2526, -1820, 2117, 7414 ] ),
        BuiltinMatrix::new(
            sony!(RX100),
            0,
            0,
            [ 8651, -2754, -1057, -3464, 12207, 1373, -568, 1398, 4434 ] ),
        BuiltinMatrix::new(
            sony!(RX100M2),
            0,
            0,
            [ 6596, -2079, -562, -4782, 13016, 1933, -970, 1581, 5181 ] ),
        BuiltinMatrix::new(
            sony!(RX100M3),
            0,
            0,
            [ 6596, -2079, -562, -4782, 13016, 1933, -970, 1581, 5181 ] ),
        BuiltinMatrix::new(
            sony!(RX100M4),
            0,
            0,
            [ 6596, -2079, -562, -4782, 13016, 1933, -970, 1581, 5181 ] ),
        BuiltinMatrix::new(
            sony!(RX100M5),
            0,
            0,
            [ 6596, -2079, -562, -4782, 13016, 1933, -970, 1581, 5181 ] ),
        BuiltinMatrix::new(
            sony!(RX100M5A),
            0,
            0,
            [ 11176, -4700, -965, -4004, 12184, 2032, -763, 1726, 5876 ] ),
        BuiltinMatrix::new(
            sony!(RX100M6),
            0,
            0,
            [ 7325, -2321, -596, -3494, 11674, 2055, -668, 1562, 5031 ] ),
        BuiltinMatrix::new(
            sony!(RX100M7),
            0,
            0,
            [ 7325, -2321, -596, -3494, 11674, 2055, -668, 1562, 5031 ] ),
        BuiltinMatrix::new(
            sony!(RX0),
            0,
            0,
            [ 9396, -3507, -843, -2497, 11111, 1572, -343, 1355, 5089 ] ),
        BuiltinMatrix::new(
            sony!(RX0M2),
            0,
            0,
            [ 9396, -3507, -843, -2497, 11111, 1572, -343, 1355, 5089 ] ),
        BuiltinMatrix::new(
            sony!(RX1),
            0,
            0,
            [ 6344, -1612, -462, -4863, 12477, 2681, -865, 1786, 6899 ] ),
        BuiltinMatrix::new(
            sony!(RX1R),
            0,
            0,
            [ 6344, -1612, -462, -4863, 12477, 2681, -865, 1786, 6899 ] ),
        BuiltinMatrix::new(
            sony!(RX1RM2),
            0,
            0,
            [ 6629, -1900, -483, -4618, 12349, 2550, -622, 1381, 6514 ] ),
        BuiltinMatrix::new(
            sony!(RX10),
            0,
            0,
            [ 6679, -1825, -745, -5047, 13256, 1953, -1580, 2422, 5183 ] ),
        BuiltinMatrix::new(
            sony!(RX10M2),
            0,
            0,
            [ 6679, -1825, -745, -5047, 13256, 1953, -1580, 2422, 5183 ] ),
        BuiltinMatrix::new(
            sony!(RX10M3),
            0,
            0,
            [ 6679, -1825, -745, -5047, 13256, 1953, -1580, 2422, 5183 ] ),
        BuiltinMatrix::new(
            sony!(RX10M4),
            0,
            0,
            [ 7699, -2566, -629, -2967, 11270, 1928, -378, 1286, 4807 ] ),

        BuiltinMatrix::new(
            sony!(ILCE1),
            128,
            0,
            [ 8161, -2947, -739, -4811, 12668, 2389, -437, 1229, 6524 ] ),
        BuiltinMatrix::new(
            sony!(ILCE3000),
            128,
            0,
            [ 5991, -1456, -455, -4764, 12135, 2980, -707, 1425, 6701 ] ),
        BuiltinMatrix::new(
            sony!(ILCE5000),
            128,
            0,
            [ 5991, -1456, -455, -4764, 12135, 2980, -707, 1425, 6701 ] ),
        BuiltinMatrix::new(
            sony!(ILCE5100),
            128,
            0,
            [ 5991, -1456, -455, -4764, 12135, 2980, -707, 1425, 6701 ] ),
        BuiltinMatrix::new(
            sony!(ILCE6000),
            128,
            0,
            [ 5991, -1456, -455, -4764, 12135, 2980, -707, 1425, 6701 ] ),
        BuiltinMatrix::new(
            sony!(ILCE6100),
            128,
            0,
            [ 7657, -2847, -607, -4083, 11966, 2389, -684, 1418, 5844 ] ),
        BuiltinMatrix::new(
            sony!(ILCE6300),
            0,
            0,
            [ 5973, -1695, -419, -3826, 11797, 2293, -639, 1398, 5789 ] ),
        BuiltinMatrix::new(
            sony!(ILCE6400),
            0,
            0,
            [ 5973, -1695, -419, -3826, 11797, 2293, -639, 1398, 5789 ] ),
        BuiltinMatrix::new(
            sony!(ILCE6500),
            0,
            0,
            [ 5973, -1695, -419, -3826, 11797, 2293, -639, 1398, 5789 ] ),
        BuiltinMatrix::new(
            sony!(ILCE6600),
            128,
            0,
            [ 7657, -2847, -607, -4083, 11966, 2389, -684, 1418, 5844 ] ),
        BuiltinMatrix::new(
            sony!(ILCE7),
            128,
            0,
            [ 5271, -712, -347, -6153, 13653, 2763, -1601, 2366, 7242 ] ),
        BuiltinMatrix::new(
            sony!(ILCE7M2),
            128,
            0,
            [ 5271, -712, -347, -6153, 13653, 2763, -1601, 2366, 7242 ] ),
        BuiltinMatrix::new(
            sony!(ILCE7M3),
            128,
            0,
            [ 7374, -2389, -551, -5435, 13162, 2519, -1006, 1795, 6552 ] ),
        BuiltinMatrix::new(
            sony!(ILCE7M4),
            128,
            0,
            [ 7460, -2365, -588, -5687, 13442, 2474, -624, 1156, 6584 ] ),
        BuiltinMatrix::new(
            sony!(ILCE7R),
            128,
            0,
            [ 4913, -541, -202, -6130, 13513, 2906, -1564, 2151, 7183 ] ),
        BuiltinMatrix::new(
            sony!(ILCE7RM2),
            0,
            0,
            [ 6629, -1900, -483, -4618, 12349, 2550, -622, 1381, 6514 ] ),
        BuiltinMatrix::new(
            sony!(ILCE7RM3),
            0,
            0,
            [ 6640, -1847, -503, -5238, 13010, 2474, -993, 1673, 6527 ] ),
        BuiltinMatrix::new(
            sony!(ILCE7RM3A),
            0,
            0,
            [ 6640, -1847, -503, -5238, 13010, 2474, -993, 1673, 6527 ] ),
        BuiltinMatrix::new(
            sony!(ILCE7RM4),
            0,
            0,
            [ 6640, -1847, -503, -5238, 13010, 2474, -993, 1673, 6527 ] ),
        BuiltinMatrix::new(
            sony!(ILCE7RM4A),
            0,
            0,
            [ 7662, -2686, -660, -5240, 12965, 2530, -796, 1508, 6167 ] ),
        BuiltinMatrix::new(
            sony!(ILCE7RM5),
            0,
            0,
            [ 8200, -2976, -719, -4296, 12053, 2532, -429, 1282, 5774 ] ),
        BuiltinMatrix::new(
            sony!(ILCE7S),
            128,
            0,
            [ 5838, -1430, -246, -3497, 11477, 2297, -748, 1885, 5778 ] ),
        BuiltinMatrix::new(
            sony!(ILCE7SM2),
            128,
            0,
            [ 5838, -1430, -246, -3497, 11477, 2297, -748, 1885, 5778 ] ),
        BuiltinMatrix::new(
            sony!(ILCE7SM3),
            128,
            0,
            [ 6912, -2127, -469, -4470, 12175, 2587, -398, 1478, 6492 ] ),

        BuiltinMatrix::new(
            sony!(ILCE7C),
            128,
            0,
            [ 7374, -2389, -551, -5435, 13162, 2519, -1006, 1795, 6552 ] ),
        BuiltinMatrix::new(
            sony!(ILCE9),
            128,
            0,
            [ 6389, -1703, -378, -4562, 12265, 2587, -670, 1489, 6550 ] ),
        BuiltinMatrix::new(
            sony!(ILCE9M2),
            128,
            0,
            [ 6389, -1703, -378, -4562, 12265, 2587, -670, 1489, 6550 ] ),
        BuiltinMatrix::new(
            sony!(ILCEQX1),
            128,
            0,
            [ 5991, -1456, -455, -4764, 12135, 2980, -707, 1425, 6701 ] ),
        BuiltinMatrix::new(
            sony!(ILME_FX30),
            128,
            0,
            [ 6972, -2408, -600, -4330, 12101, 2515, -388, 1277, 5847 ] ),
        BuiltinMatrix::new(
            sony!(ZV1),
            128,
            0,
            [ 8280, -2987, -703, -3531, 11645, 2133, -550, 1542, 5312 ] ),
        BuiltinMatrix::new(
            sony!(ZVE1),
            128,
            0,
            [ 6912, -2127, -469, -4470, 12175, 2587, -398, 1478, 6492 ] ),
        BuiltinMatrix::new(
            sony!(ZVE10),
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
        self.identify_id() == sony!(A100)
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
                        .unwrap_or(sony!(UNKNOWN));
                } else {
                    log::error!("Sony model ID tag not found");
                }
            }
            // The A100 is broken we use a fallback
            // But when it's no longer broken, we might be able to get away with this
            let container = self.container.get().unwrap();
            tiff::identify_with_exif(container, &MAKE_TO_ID_MAP).unwrap_or(sony!(UNKNOWN))
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

    fn ifd(&self, ifd_type: tiff::IfdType) -> Option<&Dir> {
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
            tiff::IfdType::Exif => container.exif_dir(),
            tiff::IfdType::MakerNote => container.mnote_dir(),
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
                    tiff::tiff_get_rawdata(self.container.get().unwrap(), dir, self.type_())
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
    fn write_dump<W>(&self, out: &mut W, indent: u32)
    where
        W: std::io::Write + ?Sized,
    {
        dump_writeln!(out, indent, "<Sony ARW File>");
        {
            let indent = indent + 1;
            self.container();
            self.container.get().unwrap().write_dump(out, indent);
        }
        dump_writeln!(out, indent, "</Sony ARW File>");
    }
}

dumpfile_impl!(ArwFile);
