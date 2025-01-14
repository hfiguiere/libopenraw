// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - sony.rs
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

//! Sony specific code.

use std::collections::HashMap;
use std::rc::Rc;

use byteorder::LittleEndian;
use once_cell::unsync::OnceCell;

use crate::camera_ids::{self, hasselblad, vendor};
use crate::colour::BuiltinMatrix;
use crate::container::{Endian, RawContainer};
use crate::io::Viewer;
use crate::minolta::MrwContainer;
use crate::rawfile::{RawFileHandleType, ThumbnailStorage};
use crate::tiff::exif::{self, ExifValue};
use crate::tiff::{Dir, Ifd, LoaderFixup};
use crate::{tiff, utils};
use crate::{
    AspectRatio, Bitmap, DataType, Dump, Error, RawFile, RawFileHandle, RawFileImpl, RawImage,
    Rect, Result, Type, TypeId,
};

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

pub use tiff::exif::generated::MNOTE_SONY_TAG_NAMES as MNOTE_TAG_NAMES;

lazy_static::lazy_static! {
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
        sony!(385, ILME_FX3),
        sony!(386, ILCE7RM3A),
        sony!(387, ILCE7RM4A),
        sony!(388, ILCE7M4),
        sony!(390, ILCE7RM5),
        sony!(391, ILME_FX30),
        sony!(392, ILCE9M3),
        sony!(393, ZVE1),
        sony!(394, ILCE6700),
        sony!(395, ZV1M2),
        sony!(399, ZVE10M2),
        sony!(400, ILCE1M2),
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
        sony!("ILCE-1M2", ILCE1M2),
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
        sony!("ILCE-6700", ILCE6700),
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
        sony!("ILCE-9M3", ILCE9M3),
        sony!("ILME-FX3", ILME_FX3),
        sony!("ILME-FX30", ILME_FX30),
        sony!("ZV-1", ZV1),
        sony!("ZV-1M2", ZV1M2),
        sony!("ZV-E1", ZVE1),
        sony!("ZV-E10", ZVE10),
        sony!("ZV-E10M2", ZVE10M2),
        sony!("UMC-R10C", UMCR10C),
        ("Lunar", TypeId(vendor::HASSELBLAD, hasselblad::LUNAR)),
    ]);

    static ref MATRICES: [BuiltinMatrix; 97] = [
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
            511,
            16383,
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
        // Currently unsure if it is the right one.
        BuiltinMatrix::new(
            sony!(ILCE1M2),
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
            sony!(ILCE6700),
            128,
            0,
            [ 6972, -2408, -600, -4330, 12101, 2515, -388, 1277, 5847 ] ),
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
            sony!(ILCE9M3),
            0,
            0,
            [ 9811, -3908, -752, -3704, 11577, 2417, -73, 950, 5980 ] ),
        BuiltinMatrix::new(
            sony!(ILCEQX1),
            128,
            0,
            [ 5991, -1456, -455, -4764, 12135, 2980, -707, 1425, 6701 ] ),
        BuiltinMatrix::new(
            sony!(ILME_FX3),
            128,
            0,
            [ 6912, -2127, -469, -4470, 12175, 2587, -398, 1477, 6492 ] ),
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
            sony!(ZV1M2),
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
        BuiltinMatrix::new(
            sony!(ZVE10M2),
            128,
            0,
            [ 6972, -2408, -600, -4330, 12101, 2515, -388, 1277, 5847 ] ),
        /* The Hasselblad Lunar is like a Nex7 */
        BuiltinMatrix::new(
            TypeId(vendor::HASSELBLAD, hasselblad::LUNAR),
            128,
            0,
            [ 5491, -1192, -363, -4951, 12342, 2948, -911, 1722, 7192 ] ),
    ];
}

#[derive(Default)]
struct ArwFixup {
    is_a100: OnceCell<bool>,
}

impl ArwFixup {
    fn is_a100(&self, container: &tiff::Container) -> bool {
        *self.is_a100.get_or_init(|| {
            container
                .directory(0)
                .and_then(|dir| dir.entry(exif::EXIF_TAG_MODEL))
                .and_then(|e| e.string_value())
                .map(|s| s == "DSLR-A100")
                .unwrap_or(false)
        })
    }
}

impl LoaderFixup for ArwFixup {
    fn parse_subifd(&self, container: &tiff::Container) -> bool {
        // On an A100, the SubIfd is the RAW data. Not an IFD.
        !self.is_a100(container)
    }
}

#[derive(Debug)]
pub(crate) struct ArwFile {
    reader: Rc<Viewer>,
    type_id: OnceCell<TypeId>,
    container: OnceCell<tiff::Container>,
    thumbnails: OnceCell<ThumbnailStorage>,
    #[cfg(feature = "probe")]
    probe: Option<crate::Probe>,
}

impl ArwFile {
    pub(crate) fn factory(reader: Rc<Viewer>) -> RawFileHandle {
        RawFileHandleType::new(ArwFile {
            reader,
            type_id: OnceCell::new(),
            container: OnceCell::new(),
            thumbnails: OnceCell::new(),
            #[cfg(feature = "probe")]
            probe: None,
        })
    }

    fn is_a100(&self) -> bool {
        self.main_ifd()
            .and_then(|dir| dir.entry(exif::EXIF_TAG_MODEL))
            .and_then(|entry| entry.string_value())
            .map(|s| s == "DSLR-A100")
            .unwrap_or(false)
    }

    fn a100_rawinfo(&self, dir: &Dir) -> Result<MrwContainer> {
        let offset = dir
            .entry(exif::DNG_TAG_DNG_PRIVATE)
            .and_then(|entry| entry.value_array::<u8>(dir.endian()))
            .map(|bytes| u32::read::<LittleEndian>(&bytes))
            .ok_or(Error::NotFound)?;
        let view = Viewer::create_view(&self.reader, offset as u64).expect("Created view");
        let mut container = MrwContainer::new(view);
        container.set_endian(Endian::Little);
        container.load().expect("Failed to load container");

        Ok(container)
    }

    fn camera_settings(&self, endian: Endian) -> Option<Vec<u16>> {
        let dir = self.maker_note_ifd()?;
        dir.entry(exif::MNOTE_SONY_CAMERA_SETTINGS)?
            .value_array::<u16>(endian)
    }

    fn camera_settings3(&self) -> Option<Vec<u8>> {
        let dir = self.maker_note_ifd()?;
        dir.entry(exif::MNOTE_SONY_CAMERA_SETTINGS)?
            .value_array::<u8>(Endian::Little)
    }

    /// Decipher table for some Sony tags including 0x2010. Source
    /// ExifTool.
    ///
    /// It is built using the following code:
    /// ```
    /// let cipher_table = (0..256).map(|v|
    ///         ((v as u32 * v as u32 * v as u32) % 249 ) as u8
    /// ).collect::<Vec<_>>();
    ///
    /// let mut decipher_table = vec![0_u8; 256];
    /// cipher_table.iter().enumerate().for_each(|(i, v)| {
    ///     if i < 249 {
    ///         decipher_table[*v as usize] = i as u8;
    ///     } else {
    ///         decipher_table[i] = i as u8;
    ///     }
    /// });
    /// ```
    const DECIPHER_TABLE: [u8; 256] = [
        0, 1, 50, 177, 10, 14, 135, 40, 2, 204, 202, 173, 27, 220, 8, 237, 100, 134, 240, 79, 140,
        108, 184, 203, 105, 196, 44, 3, 151, 182, 147, 124, 20, 243, 226, 62, 48, 142, 215, 96, 28,
        161, 171, 55, 236, 117, 190, 35, 21, 106, 89, 63, 208, 185, 150, 181, 80, 39, 136, 227,
        129, 148, 224, 192, 4, 92, 198, 232, 95, 75, 112, 56, 159, 130, 128, 81, 43, 197, 69, 73,
        155, 33, 82, 83, 84, 133, 11, 93, 97, 218, 123, 85, 38, 36, 7, 110, 54, 91, 71, 183, 217,
        74, 162, 223, 191, 18, 37, 188, 30, 127, 86, 234, 16, 230, 207, 103, 77, 60, 145, 131, 225,
        49, 179, 111, 244, 5, 138, 70, 200, 24, 118, 104, 189, 172, 146, 42, 19, 233, 15, 163, 122,
        219, 61, 212, 231, 58, 26, 87, 175, 32, 66, 178, 158, 195, 139, 242, 213, 211, 164, 126,
        31, 152, 156, 238, 116, 165, 166, 167, 216, 94, 176, 180, 52, 206, 168, 121, 119, 90, 193,
        137, 174, 154, 17, 51, 157, 245, 57, 25, 101, 120, 22, 113, 210, 169, 68, 99, 64, 41, 186,
        160, 143, 228, 214, 59, 132, 13, 194, 78, 88, 221, 153, 34, 107, 201, 187, 23, 6, 229, 125,
        102, 67, 98, 246, 205, 53, 144, 46, 65, 141, 109, 170, 9, 115, 149, 12, 241, 29, 222, 76,
        47, 45, 247, 209, 114, 235, 239, 72, 199, 248, 249, 250, 251, 252, 253, 254, 255,
    ];

    fn decipher(data: &mut [u8]) {
        data.iter_mut()
            .for_each(|v| *v = Self::DECIPHER_TABLE[*v as usize]);
    }

    /// Decipher the content of tag 0x2010
    fn camera_settings_2010(&self) -> Option<Vec<u8>> {
        let dir = self.maker_note_ifd()?;
        dir.entry(exif::MNOTE_SONY_CAMERA_SETTINGS_2010)?
            .value_array::<u8>(Endian::Little)
            .map(|mut data| {
                Self::decipher(&mut data);
                data
            })
    }

    /// AspectRatio as found in tag 0x2010. The structure might differ so
    /// get it at the specified `offset`.
    fn aspect_ratio_2010(&self, offset: usize) -> Option<AspectRatio> {
        let camera_settings = self.camera_settings_2010()?;
        match camera_settings[offset] {
            0 => Some(AspectRatio(16, 9)),
            1 => Some(AspectRatio(4, 3)),
            2 => Some(AspectRatio(3, 2)),
            3 => Some(AspectRatio(1, 1)),
            5 =>
            /* Panorama */
            {
                None
            }
            _ => None,
        }
    }

    fn aspect_ratio_cs(&self, offset: usize) -> Option<AspectRatio> {
        let camera_settings = self.camera_settings(Endian::Big)?;
        match camera_settings[offset] {
            1 => Some(AspectRatio(3, 2)),
            2 => Some(AspectRatio(16, 9)),
            _ => None,
        }
    }

    fn aspect_ratio_cs3(&self) -> Option<AspectRatio> {
        let camera_settings = self.camera_settings3()?;
        match camera_settings[10] {
            4 => Some(AspectRatio(3, 2)),
            8 => Some(AspectRatio(16, 9)),
            _ => None,
        }
    }

    fn aspect_ratio(&self) -> Option<AspectRatio> {
        use camera_ids::sony::*;
        // Heuristics based off ExifTool
        match self.type_id().1 {
            A200 | A300 | A350 | A700 | A850 | A900 => {
                // CameraSettings, idx = 85, BigEndian
                probe!(self.probe, "arw.aspect_ratio.cs1", true);
                self.aspect_ratio_cs(85)
            },
            A230 | A290 | A330 | A380 /* also A390 */ => {
                // CameraSettings2, idx = 85, BigEndian
                probe!(self.probe, "arw.aspect_ratio.cs2", true);
                self.aspect_ratio_cs(85)
            },
            SLTA33 | SLTA35 | SLTA55 | A450 | A500 | A550 | A560 | A580 |
            NEX3 | NEX5 | NEXC3 /*| NEXVG10E */ => {
                // CameraSettings3, idx = 10, LittleEndian
                probe!(self.probe, "arw.aspect_ratio.cs3", true);
                self.aspect_ratio_cs3()
            },
            SLTA58 | SLTA99 | ILCE3000 /* and ILCE3500 */ | NEX3N | NEX5R |
            NEX5T | NEX6 /* | NEXVG30 | NEXVG900 */ | RX100 | RX1 | RX1R => {
                // ExifTool 2010e
                probe!(self.probe, "arw.aspect_ratio.2010e", true);
                self.aspect_ratio_2010(6444)
            },
            RX100M2 => {
                // ExifTool 2010f
                probe!(self.probe, "arw.aspect_ratio.2010f", true);
                self.aspect_ratio_2010(6444)
            }
            RX10 | RX100M3 | ILCE7 | ILCE7R | ILCE7S | ILCE7M2 | ILCE5000 |
            ILCE5100 | ILCE6000 | ILCEQX1 | ILCA68 | ILCA77M2 => {
                // ExifTool 2010g
                probe!(self.probe, "arw.aspect_ratio.2010g", true);
                self.aspect_ratio_2010(6488)
            }
            RX0 | RX1RM2 | RX10M2 | RX10M3 | RX100M4 | RX100M5 | ILCE6300 |
            ILCE6500 | ILCE7RM2 | ILCE7SM2 | ILCA99M2 => {
                // ExifTool 2010h
                probe!(self.probe, "arw.aspect_ratio.2010h", true);
                self.aspect_ratio_2010(6444)
            }
            ILCE6100 | ILCE6400 | ILCE6600 | ILCE7C | ILCE7M3 | ILCE7RM3 |
            ILCE7RM4 | ILCE9 | ILCE9M2 | RX0M2 | RX10M4 | RX100M6 | RX100M5A |
            RX100M7 | HX99 | ZV1 | ZV1M2 /* | ZV1F */ | ZVE10 => {
                // ExifTool 2010i
                probe!(self.probe, "arw.aspect_ratio.2010i", true);
                self.aspect_ratio_2010(6284)
            }
            _ => {
                probe!(self.probe, "arw.aspect_ratio.unknown", true);
                None
            }
        }
    }

    fn load_rawdata_arw(&self, dir: &Dir) -> Result<RawImage> {
        let container = self.container.get().unwrap();
        let rawdata_endian = if self.type_id().1 == camera_ids::sony::R1 {
            Endian::Big
        } else {
            container.endian()
        };
        tiff::tiff_get_rawdata_with_endian(container, dir, self.type_(), rawdata_endian).map(
            |mut rawimage| {
                rawimage.set_active_area(Some(Rect {
                    x: 0,
                    y: 0,
                    width: rawimage.width(),
                    height: rawimage.height(),
                }));
                let user_crop = dir
                    .uint_value_array(exif::ARW_TAG_SONY_CROP_TOP_LEFT)
                    .and_then(|top_left| {
                        if top_left.len() < 2 {
                            log::error!("Top Left: not enough elements");
                            return None;
                        }
                        dir.uint_value_array(exif::ARW_TAG_SONY_CROP_SIZE)
                            .and_then(|size| {
                                if size.len() < 2 {
                                    log::error!("Crop Size: not enough elements");
                                    return None;
                                }
                                Some(Rect {
                                    x: top_left[1],
                                    y: top_left[0],
                                    width: size[0],
                                    height: size[1],
                                })
                            })
                    });
                let aspect_ratio = self.aspect_ratio();
                rawimage.set_user_crop(user_crop, aspect_ratio);
                if let Some(wb) = dir.int_value_array(exif::ARW_TAG_WB_RGGB_LEVELS).map(|v| {
                    let g = v[1] as f64;
                    [g / v[0] as f64, 1.0, g / v[3] as f64]
                }) {
                    rawimage.set_as_shot_neutral(&wb);
                }
                // XXX This here and Pentax get the levels from the builtins.
                // Make this in a common area.
                let levels = MATRICES
                    .iter()
                    .find(|m| m.camera == self.type_id())
                    .map(|m| (m.black, m.white));

                if let Some(blacks) = dir.uint_value_array(exif::ARW_TAG_BLACK_LEVELS) {
                    rawimage.set_blacks(utils::to_quad(&blacks));
                } else if let Some((black, _)) = levels {
                    rawimage.set_blacks([black; 4]);
                }
                if let Some(whites) = dir.uint_value_array(exif::DNG_TAG_WHITE_LEVEL) {
                    rawimage.set_whites(utils::to_quad(&whites));
                } else if let Some((_, white)) = levels {
                    rawimage.set_whites([white; 4]);
                }
                rawimage
            },
        )
    }

    fn load_rawdata_a100(&self, dir: &Dir) -> Result<RawImage> {
        let container = self.container.get().unwrap();
        let offset = dir
            .uint_value(exif::EXIF_TAG_SUB_IFDS)
            .ok_or(Error::NotFound)?;
        let raw_info = self.a100_rawinfo(dir)?;

        let prd = raw_info.minolta_prd()?;

        // XXX figure out where to find this. It must be somewhere.
        // They don't match the ImageWidth and ImageHeight
        let width = prd.x; //3881;
        let height = prd.y; //2608;
        let byte_len = width * height * 2;
        let data = container.load_buffer8(offset as u64, byte_len as u64);

        let mut rawimage = RawImage::with_data8(
            width,
            height,
            prd.bps,
            DataType::CompressedRaw,
            data,
            prd.mosaic,
        );
        if let Some(wb) = raw_info.get_wb() {
            rawimage.set_as_shot_neutral(&wb);
        }
        Ok(rawimage)
    }
}

impl RawFileImpl for ArwFile {
    #[cfg(feature = "probe")]
    probe_imp!();

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
            let mut container =
                tiff::Container::new(view, vec![(tiff::IfdType::Main, None)], self.type_());
            container
                .load(Some(Box::<ArwFixup>::default()))
                .expect("Arw container error");
            container
        })
    }

    fn thumbnails(&self) -> &ThumbnailStorage {
        self.thumbnails.get_or_init(|| {
            self.container();
            let container = self.container.get().unwrap();
            ThumbnailStorage::with_thumbnails(tiff::tiff_thumbnails(container))
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

    fn load_rawdata(&self, _skip_decompress: bool) -> Result<RawImage> {
        self.ifd(tiff::IfdType::Raw)
            .ok_or(Error::NotFound)
            .and_then(|dir| {
                if self.is_a100() {
                    self.load_rawdata_a100(dir)
                } else {
                    self.load_rawdata_arw(dir)
                }
            })
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
