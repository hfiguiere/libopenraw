// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - panasonic.rs
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

//! Panasonic camera support (and some Leica)

use std::collections::HashMap;
use std::rc::Rc;

use once_cell::unsync::OnceCell;

use crate::colour::BuiltinMatrix;
use crate::container::{Endian, RawContainer};
use crate::decompress;
use crate::io::Viewer;
use crate::jpeg;
use crate::leica;
use crate::mosaic::Pattern;
use crate::rawfile::{RawFileHandleType, ThumbnailStorage};
use crate::thumbnail;
use crate::tiff;
use crate::tiff::exif;
use crate::tiff::{Dir, Ifd, LoaderFixup};
use crate::{
    DataType, Dump, Error, RawFile, RawFileHandle, RawFileImpl, RawImage, Result, Type, TypeId,
};

macro_rules! panasonic {
    ($id:expr, $model:ident) => {
        (
            $id,
            TypeId(
                $crate::camera_ids::vendor::PANASONIC,
                $crate::camera_ids::panasonic::$model,
            ),
        )
    };
    ($model:ident) => {
        TypeId(
            $crate::camera_ids::vendor::PANASONIC,
            $crate::camera_ids::panasonic::$model,
        )
    };
}

pub use tiff::exif::generated::MNOTE_PANASONIC_TAG_NAMES as MNOTE_TAG_NAMES;
use tiff::exif::generated::RAW_PANASONIC_CAMERAIFD_TAG_NAMES as RAW_CAMERAIFD_TAG_NAMES;
pub use tiff::exif::generated::RAW_PANASONIC_TAG_NAMES as RAW_TAG_NAMES;

lazy_static::lazy_static! {
    static ref MAKE_TO_ID_MAP: tiff::MakeToIdMap = HashMap::from([
        panasonic!("DMC-CM1", CM1),
        panasonic!("DMC-GF1", GF1),
        panasonic!("DMC-GF2", GF2),
        panasonic!("DMC-GF3", GF3),
        panasonic!("DMC-GF5", GF5),
        panasonic!("DMC-GF6", GF6),
        panasonic!("DMC-GF7", GF7),
        panasonic!("DMC-GF8", GF8),
        panasonic!("DC-GF10", GF10),
        panasonic!("DMC-GX1", GX1),
        panasonic!("DMC-GX7", GX7),
        panasonic!("DMC-GX7MK2", GX7MK2),
        panasonic!("DC-GX7MK3", GX7MK3),
        panasonic!("DMC-GX8", GX8),
        panasonic!("DMC-GX80", GX80),
        panasonic!("DMC-GX85", GX85),
        panasonic!("DC-GX800", GX800),
        panasonic!("DC-GX850", GX850),
        panasonic!("DC-GX880", GX880),
        panasonic!("DC-GX9", GX9),
        panasonic!("DMC-FX150", FX150),
        panasonic!("DMC-FZ8", FZ8),
        panasonic!("DMC-FZ1000", DMC_FZ1000),
        panasonic!("DC-FZ10002", DC_FZ1000M2),
        panasonic!("DC-FZ1000M2", DC_FZ1000M2),
        panasonic!("DMC-FZ18", FZ18),
        panasonic!("DMC-FZ150", FZ150),
        panasonic!("DMC-FZ28", FZ28),
        panasonic!("DMC-FZ30", FZ30),
        panasonic!("DMC-FZ300", FZ300),
        panasonic!("DMC-FZ35", FZ35),
        panasonic!("DMC-FZ38", FZ38),
        panasonic!("DMC-FZ40", DMC_FZ40),
        panasonic!("DMC-FZ45", DMC_FZ45),
        // Not the same as above
        panasonic!("DC-FZ45", DC_FZ45),
        panasonic!("DMC-FZ50", FZ50),
        panasonic!("DMC-FZ70", FZ70),
        panasonic!("DMC-FZ72", FZ72),
        panasonic!("DMC-FZ100", FZ100),
        panasonic!("DMC-FZ200", FZ200),
        panasonic!("DMC-FZ2500", FZ2500),
        // Alias to DMC-FZ2500
        panasonic!("DMC-FZ2000", FZ2000),
        panasonic!("DMC-FZ330", FZ330),
        panasonic!("DC-FZ80", FZ80),
        panasonic!("DC-FZ82", FZ82),
        panasonic!("DMC-G1", G1),
        panasonic!("DMC-G2", G2),
        panasonic!("DMC-G3", G3),
        panasonic!("DMC-G5", G5),
        panasonic!("DMC-G6", G6),
        panasonic!("DMC-G7", G7),
        panasonic!("DMC-G70", G70),
        panasonic!("DMC-G10", G10),
        panasonic!("DMC-G80", G80),
        panasonic!("DMC-G81", G81),
        panasonic!("DC-G9", G9),
        panasonic!("DC-G9M2", G9M2),
        panasonic!("DC-G90", DC_G90),
        panasonic!("DC-G91", DC_G91),
        panasonic!("DC-G95", DC_G95),
        panasonic!("DC-G95D", DC_G95D),
        panasonic!("DC-G99", DC_G99),
        panasonic!("DC-G100", DC_G100),
        panasonic!("DC-G100D", DC_G100D),
        panasonic!("DC-G110", DC_G110),
        panasonic!("DMC-GH1", GH1),
        panasonic!("DMC-GH2", GH2),
        panasonic!("DMC-GH3", GH3),
        panasonic!("DMC-GH4", GH4),
        panasonic!("DC-GH5", GH5),
        panasonic!("DC-GH5S", GH5S),
        panasonic!("DC-GH5M2", GH5M2),
        panasonic!("DC-GH6", GH6),
        panasonic!("DMC-GM1", GM1),
        panasonic!("DMC-GM1S", GM1S),
        panasonic!("DMC-GM5", GM5),
        panasonic!("DMC-LF1", LF1),
        panasonic!("DMC-LX1", LX1),
        panasonic!("DMC-LX2", LX2),
        panasonic!("DMC-LX3", LX3),
        panasonic!("DMC-LX5", LX5),
        panasonic!("DMC-LX7", LX7),
        panasonic!("DMC-LX10", LX10),
        panasonic!("DMC-LX15", LX15),
        panasonic!("DMC-LX100", LX100),
        panasonic!("DC-LX100M2", LX100M2),
        panasonic!("DMC-L1", L1),
        panasonic!("DMC-L10", L10),
        panasonic!("DC-S1", DC_S1),
        panasonic!("DC-S1R", DC_S1R),
        panasonic!("DC-S1H", DC_S1H),
        panasonic!("DC-S5", DC_S5),
        panasonic!("DC-S5M2", DC_S5M2),
        panasonic!("DC-S5M2X", DC_S5M2X),
        panasonic!("DC-S9", DC_S9),
        panasonic!("DMC-TZ70", TZ70),
        panasonic!("DMC-ZS60", ZS60),
        // Aliases to DMC-ZS60 (2)
        panasonic!("DMC-TZ80", TZ80),
        panasonic!("DMC-TZ81", TZ81),
        panasonic!("DMC-ZS100", ZS100),
        // Aliases to DMC-ZS100
        panasonic!("DMC-TX1", TX1),
        panasonic!("DMC-TZ100", TZ100),
        panasonic!("DMC-TZ101", TZ101),
        panasonic!("DMC-TZ110", TZ110),
        panasonic!("DC-ZS200", ZS200),
        // Aliases to DC-ZS200
        panasonic!("DC-TZ202", TZ202),
        panasonic!("DC-ZS80", DC_ZS80),
        panasonic!("DC-ZS200D", ZS200D),
        // Aliases to DC-ZS80
        panasonic!("DC-TZ95", DC_TZ95),
        panasonic!("DC-TZ96", DC_TZ96),
        panasonic!("DMC-ZS40", ZS40),
        // Aliases to DMC-ZS40
        panasonic!("DMC-TZ60", TZ60),
        panasonic!("DMC-TZ61", TZ61),
        // Aliases to DMC-ZS50
        panasonic!("DMC-TZ71", TZ71),
        // Aliases to DMC-ZS70
        panasonic!("DMC-TZ90", TZ90),

        leica!("DIGILUX 2", DIGILUX2),
        leica!("DIGILUX 3", DIGILUX3),
        leica!("D-LUX 3", DLUX_3),
        leica!("D-LUX 4", DLUX_4),
        leica!("D-LUX 5", DLUX_5),
        leica!("D-LUX 6", DLUX_6),
        leica!("D-Lux 7", DLUX_7),
        leica!("V-LUX 1", VLUX_1),
        leica!("D-LUX (Typ 109)", DLUX_TYP109),
        leica!("V-LUX 4", VLUX_4),
        leica!("V-Lux 5", VLUX_5),
        leica!("V-LUX (Typ 114)", VLUX_TYP114),
        leica!("C-Lux", CLUX),
        leica!("C (Typ 112)", C_TYP112),
    ]);

    static ref MATRICES: [BuiltinMatrix; 100] = [
        BuiltinMatrix::new(
            panasonic!(CM1),
            15,
            0,
            [ 8770, -3194, -820, -2871, 11281, 1803, -513, 1552, 4434 ] ),
        BuiltinMatrix::new(
            panasonic!(GF1),
            15,
            0xf92,
            [ 7888, -1902, -1011, -8106, 16085, 2099, -2353, 2866, 7330 ] ),
        BuiltinMatrix::new(
            panasonic!(GF2),
            15,
            0xfff,
            [ 7888, -1902, -1011, -8106, 16085, 2099, -2353, 2866, 7330 ] ),
        BuiltinMatrix::new(
            panasonic!(GF3),
            15,
            0xfff,
            [ 9051, -2468, -1204, -5212, 13276, 2121, -1197, 2510, 6890 ] ),
        BuiltinMatrix::new(
            panasonic!(GF5),
            15,
            0xfff,
            [ 8228, -2945, -660, -3938, 11792, 2430, -1094, 2278, 5793 ] ),
        BuiltinMatrix::new(
            panasonic!(GF6),
            15,
            0xfff,
            [ 8130, -2801, -946, -3520, 11289, 2552, -1314, 2511, 5791 ] ),
        BuiltinMatrix::new(
            panasonic!(GF7),
            15,
            0,
            [ 7610, -2780, -576, -4614, 12195, 2733, -1375, 2393, 6490 ] ),
        BuiltinMatrix::new(
            panasonic!(GF8),
            15,
            0,
            [ 7610, -2780, -576, -4614, 12195, 2733, -1375, 2393, 6490 ] ),
        BuiltinMatrix::new(
            panasonic!(GF10),
            15,
            0,
            [ 7610, -2780, -576, -4614, 12195, 2733, -1375, 2393, 6490 ] ),
        BuiltinMatrix::new(
            panasonic!(GX1),
            15,
            0,
            [ 6763, -1919, -863, -3868, 11515, 2684, -1216, 2387, 5879 ] ),
        BuiltinMatrix::new(
            panasonic!(GX7),
            15,
            0,
            [ 7610, -2780, -576, -4614, 12195, 2733, -1375, 2393, 6490 ] ),
        BuiltinMatrix::new(
            panasonic!(GX7MK2),
            15,
            0,
            [ 7771, -3020, -629, -4029, 1195, 2345, -821, 1977, 6119 ] ),
        BuiltinMatrix::new(
            panasonic!(GX8),
            15,
            0,
            [ 7564, -2263, -606, -3148, 11239, 2177, -540, 1435, 4853 ] ),
        BuiltinMatrix::new(
            panasonic!(GX80),
            15,
            0,
            [ 7771, -3020, -629, -4029, 1195, 2345, -821, 1977, 6119 ] ),
        BuiltinMatrix::new(
            panasonic!(GX800),
            15,
            0,
            [ 7610, -2780, -576, -4614, 12195, 2733, -1375, 2393, 6490 ] ),
        BuiltinMatrix::new(
            panasonic!(GX850),
            15,
            0,
            [ 7610, -2780, -576, -4614, 12195, 2733, -1375, 2393, 6490 ] ),
        BuiltinMatrix::new(
            panasonic!(GX9),
            15,
            0,
            [ 7564, -2263, -606, -3148, 11239, 2177, -540, 1435, 4853 ] ),
        BuiltinMatrix::new(
            panasonic!(FX150),
            0,
            0,
            [ 9082, -2907, -925, -6119, 13376, 3058, -1797, 2641, 5608 ] ),
        BuiltinMatrix::new(
            panasonic!(FZ8),
            0,
            0xf7f,
            [ 8986, -2755, -802, -6341, 13575, 3077, -1476, 2144, 6379 ] ),
        BuiltinMatrix::new(
            panasonic!(FZ18),
            0,
            0,
            [ 9932, -3060, -935, -5809, 13331, 2753, -1267, 2155, 5575 ] ),
        BuiltinMatrix::new(
            panasonic!(FZ28),
            15,
            0xf96,
            [ 10109, -3488, -993, -5412, 12812, 2916, -1305, 2140, 5543 ] ),
        BuiltinMatrix::new(
            panasonic!(FZ200),
            143,
            0xfff,
            [ 8112, -2563, -740, -3730, 11784, 2197, -941, 2075, 4933 ] ),
        BuiltinMatrix::new(
            panasonic!(FZ2500),
            143,
            0xfff,
            [ 7386, -2443, -743, -3437, 11864, 1757, -608, 1660, 4766 ] ),
        BuiltinMatrix::new(
            panasonic!(FZ30),
            0,
            0xf94,
            [ 10976, -4029, -1141, -7918, 15491, 2600, -1670, 2071, 8246 ] ),
        BuiltinMatrix::new(
            panasonic!(FZ300),
            15,
            0,
            [ 8378, -2798, -769, -3068, 11410, 1877, -538, 1792, 4623 ] ),
        BuiltinMatrix::new(
            panasonic!(FZ330),
            15,
            0,
            [ 8378, -2798, -769, -3068, 11410, 1877, -538, 1792, 4623 ] ),
        BuiltinMatrix::new(
            panasonic!(FZ35),
            15,
            0,
            [ 9938, -2780, -890, -4604, 12393, 2480, -1117, 2304, 4620 ] ),
        BuiltinMatrix::new(
            panasonic!(DMC_FZ45),
            0,
            0,
            [ 13639, -5535, -1371, -1698, 9633, 2430, 316, 1152, 4108 ] ),
        BuiltinMatrix::new(
            panasonic!(FZ50),
            0,
            0,
            [ 7906, -2709, -594, -6231, 13351, 3220, -1922, 2631, 6537 ] ),
        BuiltinMatrix::new(
            panasonic!(FZ70),
            0,
            0,
            [ 11532, -4324, -1066, -2375, 10847, 1749, -564, 1699, 4351 ] ),
        BuiltinMatrix::new(
            panasonic!(FZ100),
            143,
            0xfff,
            [ 16197, -6146, -1761, -2393, 10765, 1869, 366, 2238, 5248 ] ),
        BuiltinMatrix::new(
            panasonic!(DMC_FZ1000),
            0,
            0,
            [ 7830, -2696, -763, -3325, 11667, 1866, -641, 1712, 4824 ] ),
        BuiltinMatrix::new(
            panasonic!(DC_FZ1000M2),
            0,
            0,
            [ 9803, -4185, -992, -4066, 12578, 1628, -838, 1824, 5288 ] ),
        BuiltinMatrix::new(
            panasonic!(FZ150),
            0,
            0,
            [ 11904, -4541, -1189, -2355, 10899, 1662, -296, 1586, 4289 ] ),
        BuiltinMatrix::new(
            panasonic!(FZ80),
            0,
            0,
            [ 11532, -4324, -1066, -2375, 10847, 1749, -564, 1699, 4351 ] ),
        BuiltinMatrix::new(
            panasonic!(G1),
            15,
            0xf94,
            [ 8199, -2065, -1056, -8124, 16156, 2033, -2458, 3022, 7220 ] ),
        BuiltinMatrix::new(
            panasonic!(G2),
            15,
            0xf3c,
            [ 10113, -3400, -1114, -4765, 12683, 2317, -377, 1437, 6710 ] ),
        BuiltinMatrix::new(
            panasonic!(G3),
            143,
            0xfff,
            [ 6763, -1919, -863, -3868, 11515, 2684, -1216, 2387, 5879 ] ),
        BuiltinMatrix::new(
            panasonic!(G5),
            143,
            0xfff,
            [ 7798, -2562, -740, -3879, 11584, 2613, -1055, 2248, 5434 ] ),
        BuiltinMatrix::new(
            panasonic!(G10),
            0,
            0,
            [ 10113, -3400, -1114, -4765, 12683, 2317, -377, 1437, 6710 ] ),
        BuiltinMatrix::new(
            panasonic!(G6),
            143,
            0xfff,
            [ 8294, -2891, -651, -3869, 11590, 2595, -1183, 2267, 5352 ] ),
        BuiltinMatrix::new(
            panasonic!(G7),
            0,
            0,
            [ 7610, -2780, -576, -4614, 12195, 2733, -1375, 2393, 6490 ] ),
        BuiltinMatrix::new(
            panasonic!(G8),
            15,
            0,
            [ 7610, -2780, -576, -4614, 12195, 2733, -1375, 2393, 6490 ] ),
        BuiltinMatrix::new(
            panasonic!(G80),
            15,
            0,
            [ 7610, -2780, -576, -4614, 12195, 2733, -1375, 2393, 6490 ] ),
        BuiltinMatrix::new(
            panasonic!(G9),
            0,
            0,
            [ 7685, -2375, -634, -3687, 11700, 2249, -748, 1546, 5111 ] ),
        BuiltinMatrix::new(
            panasonic!(G9M2),
            0,
            0,
            [ 8325, -3456, -623, -4330, 12089, 2528, -859, 2646, 5984 ] ),
        BuiltinMatrix::new(
            panasonic!(DC_G95),
            0,
            0,
            [ 9657, -3963, -748, -3361, 11378, 2258, -568, 1415, 5158 ] ),
        BuiltinMatrix::new(
            panasonic!(DC_G99),
            0,
            0,
            [ 9657, -3963, -748, -3361, 11378, 2258, -568, 1415, 5158 ] ),
        BuiltinMatrix::new(
            panasonic!(DC_G100),
            0,
            0,
            [ 8370, -2869, -710, -3389, 11372, 2298, -640, 1599, 4887 ] ),
        BuiltinMatrix::new(
            panasonic!(DC_G100D),
            0,
            0,
            [ 8370, -2869, -710, -3389, 11372, 2298, -640, 1599, 4887 ] ),
        BuiltinMatrix::new(
            panasonic!(GH1),
            15,
            0xf92,
            [ 6299, -1466, -532, -6535, 13852, 2969, -2331, 3112, 5984 ] ),
        BuiltinMatrix::new(
            panasonic!(GH2),
            15,
            0xf95,
            [ 7780, -2410, -806, -3913, 11724, 2484, -1018, 2390, 5298 ] ),
        BuiltinMatrix::new(
            panasonic!(GH3),
            144,
            0,
            [ 6559, -1752, -491, -3672, 11407, 2586, -962, 1875, 5130 ] ),
        BuiltinMatrix::new(
            panasonic!(GH4),
            15,
            0,
            [ 7122, -2108, -512, -3155, 11201, 2231, -541, 1423, 5045 ] ),
        BuiltinMatrix::new(
            panasonic!(GH5),
            15,
            0,
            [ 7641, -2336, -605, -3218, 11299, 2187, -485, 1338, 5121 ] ),
        BuiltinMatrix::new(
            panasonic!(GH5S),
            15,
            0,
            [ 6929, -2355, -708, -4192, 12534, 1828, -1097, 1989, 5195 ] ),
        BuiltinMatrix::new(
            panasonic!(GH5M2),
            15,
            0,
            [ 9300, -3659, -755, -2981, 10988, 2287, -190, 1077, 5016 ] ),
        BuiltinMatrix::new(
            panasonic!(GH6),
            15,
            0,
            [ 7949, -3491, -710, -3435, 11681, 1977, -503, 1622, 5065 ] ),
        BuiltinMatrix::new(
            panasonic!(GM1),
            15,
            0,
            [ 6770, -1895, -744, -5232, 13145, 2303, -1664, 2691, 5703 ] ),
        BuiltinMatrix::new(
            panasonic!(GM5),
            15,
            0,
            [ 8238, -3244, -679, -3921, 11814, 2384, -836, 2022, 5852 ] ),
        BuiltinMatrix::new(
            panasonic!(LF1),
            0,
            0,
            [ 9379, -3267, -816, -3227, 11560, 1881, -926, 1928, 5340 ] ),
        BuiltinMatrix::new(
            panasonic!(LX1),
            0,
            0,
            [ 10704, -4187, -1230, -8314, 15952, 2501, -920, 945, 8927 ] ),
        BuiltinMatrix::new(
            panasonic!(LX2),
            0,
            0,
            [ 8048, -2810, -623, -6450, 13519, 3272, -1700, 2146, 7049 ] ),
        BuiltinMatrix::new(
            panasonic!(LX3),
            15,
            0,
            [ 8128, -2668, -655, -6134, 13307, 3161, -1782, 2568, 6083 ] ),
        BuiltinMatrix::new(
            panasonic!(LX5),
            143,
            0,
            [ 10909, -4295, -948, -1333, 9306, 2399, 22, 1738, 4582 ] ),
        BuiltinMatrix::new(
            panasonic!(LX7),
            143,
            0,
            [ 10148, -3743, -991, -2837, 11366, 1659, -701, 1893, 4899 ] ),
        BuiltinMatrix::new(
            panasonic!(LX10), // and LX15 (alias)
            15,
            0,
            [ 7790, -2736, -755, -3452, 11870, 1769, -628, 1647, 4898 ] ),
        BuiltinMatrix::new(
            panasonic!(LX100),
            143,
            0,
            [ 8844, -3538, -768, -3709, 11762, 2200, -698, 1792, 5220 ] ),
        BuiltinMatrix::new(
            panasonic!(LX100M2),
            0,
            0,
            [ 11577, -4230, -1106, -3967, 12211, 1957, -758, 1762, 5610 ] ),
        BuiltinMatrix::new(
            panasonic!(L1),
            0,
            0xf7f,
            [ 8054, -1885, -1025, -8349, 16367, 2040, -2805, 3542, 7629 ] ),
        BuiltinMatrix::new(
            panasonic!(L10),
            15,
            0xf96,
            [ 8025, -1942, -1050, -7920, 15904, 2100, -2456, 3005, 7039 ] ),
        BuiltinMatrix::new(
            panasonic!(TZ70),
            15,
            0,
            [ 8802, -3135, -789, -3151, 11468, 1904, -550, 1745, 4810 ] ),
        BuiltinMatrix::new(
            panasonic!(ZS40),
            15,
            0,
            [ 8607, -2822, -808, -3755, 11930, 2049, -820, 2060, 5224 ] ),
        BuiltinMatrix::new(
            panasonic!(ZS50),
            15,
            0,
            [ 8802, -3135, -789, -3151, 11468, 1904, -550, 1745, 4810 ] ),
        BuiltinMatrix::new(
            panasonic!(ZS60),
            15,
            0,
            [ 8550, -2908, -842, -3195, 11529, 1881, -338, 1603, 4631 ] ),
        BuiltinMatrix::new(
            panasonic!(ZS100),
            0,
            0,
            [  7790, -2736, -755, -3452, 11870, 1769, -628, 1647, 4898 ] ),
        BuiltinMatrix::new(
            panasonic!(ZS200),
            0,
            0,
            [  7790, -2736, -755, -3452, 11870, 1769, -628, 1647, 4898 ] ),
        BuiltinMatrix::new(
            panasonic!(DC_S1),
            0,
            0,
            [ 9744, -3905, -779, -4899, 12807, 2324, -798, 1630, 5827 ] ),
        BuiltinMatrix::new(
            panasonic!(DC_S1R),
            0,
            0,
            [ 11822, -5321, -1249, -5958, 15114, 766, -614, 1264, 7043 ] ),
        BuiltinMatrix::new(
            panasonic!(DC_S1H),
            0,
            0,
            [ 9397, -3719, -805, -5425, 13326, 2309, -972, 1715, 6034 ] ),
        BuiltinMatrix::new(
            panasonic!(DC_S5),
            0,
            0,
            [ 9744, -3905, -779, -4899, 12807, 2324, -798, 1630, 5827 ] ),
        BuiltinMatrix::new(
            panasonic!(DC_S5M2),
            0,
            0,
            [ 10308, -4206, -783, -4088, 12102, 2229, -125, 1051, 5912 ] ),
        BuiltinMatrix::new(
            panasonic!(DC_S5M2X),
            0,
            0,
            [ 10308, -4206, -783, -4088, 12102, 2229, -125, 1051, 5912 ] ),
        BuiltinMatrix::new(
            panasonic!(DC_S9),
            0,
            0,
            [ 9983, -3890, -840, -4180, 12164, 2263, -248, 1139, 5766 ] ),
        BuiltinMatrix::new(
            panasonic!(ZS70),
            0,
            0,
            [ 9052, -3117, -883, -3045, 11346, 1927, -205, 1520, 4730 ] ),
        BuiltinMatrix::new(
            panasonic!(DC_ZS80),
            0,
            0,
            [ 12194, -5340, -1329, -3035, 11394, 1858, -50, 1418, 5219 ] ),

        BuiltinMatrix::new(
            leica!(DIGILUX2),
            0,
            0,
            [ 11340, -4069, -1275, -7555, 15266, 2448, -2960, 3426, 7685 ] ),
        BuiltinMatrix::new(
            leica!(DIGILUX3),
            0,
            0,
            [ 8054, -1886, -1025, -8348, 16367, 2040, -2805, 3542, 7630 ] ),
        BuiltinMatrix::new(
            leica!(DLUX_3),
            0,
            0,
            [ 8048, -2810, -623, -6450, 13519, 3272, -1700, 2146, 7049 ] ),
        BuiltinMatrix::new(
            leica!(DLUX_TYP109),
            0,
            0,
            [ 8844, -3538, -768, -3709, 11762, 2200, -698, 1792, 5220 ] ),
        BuiltinMatrix::new(
            leica!(DLUX_4),
            0,
            0,
            [ 8128, -2668, -655, -6134, 13307, 3161, -1782, 2568, 6083 ] ),
        BuiltinMatrix::new(
            leica!(DLUX_5),
            143,
            0,
            [ 10909, -4295, -948, -1333, 9306, 2399, 22, 1738, 4582 ] ),
        BuiltinMatrix::new(
            leica!(VLUX_1),
            0,
            0,
            [ 7906, -2709, -594, -6231, 13351, 3220, -1922, 2631, 6537 ] ),
        BuiltinMatrix::new(
            leica!(VLUX_4),
            0,
            0,
            [ 8112, -2563, -740, -3730, 11784, 2197, -941, 2075, 4933 ] ),
        BuiltinMatrix::new(
            leica!(VLUX_TYP114),
            0,
            0,
            [ 7830, -2696, -763, -3325, 11667, 1866, -641, 1712, 4824 ] ),
        BuiltinMatrix::new(
            leica!(VLUX_5),
            0,
            0,
            [ 9803, -4185, -992, -4066, 12578, 1628, -838, 1824, 5288 ] ),
        BuiltinMatrix::new(
            leica!(CLUX),
            15,
            0,
            [ 7790, -2736, -755, -3452, 11870, 1769, -628, 1647, 4898 ] ),
        BuiltinMatrix::new(
            leica!(DLUX_6),
            0,
            0,
            [ 10148, -3743, -991, -2837, 11366, 1659, -701, 1893, 4899 ] ),
        BuiltinMatrix::new(
            leica!(DLUX_7),
            0,
            0,
            [ 11577, -4230, -1106, -3967, 12211, 1957, -758, 1762, 5610 ] ),
        BuiltinMatrix::new(
            leica!(C_TYP112),
            0,
            0,
            [ 9379, -3267, -816, -3227, 11560, 1881, -926, 1928, 5340 ] ),
    ];
}

struct Rw2Fixup {}

impl LoaderFixup for Rw2Fixup {
    fn check_magic_header(&self, buf: &[u8]) -> Result<Endian> {
        Rw2File::is_magic_header(buf)
    }
}

#[derive(Debug)]
/// Panasonic Rw2 File
pub(crate) struct Rw2File {
    reader: Rc<Viewer>,
    type_id: OnceCell<TypeId>,
    container: OnceCell<tiff::Container>,
    thumbnails: OnceCell<ThumbnailStorage>,
    jpeg_preview: OnceCell<Option<jpeg::Container>>,
    probe: Option<crate::Probe>,
}

impl Rw2File {
    pub(crate) fn factory(reader: Rc<Viewer>) -> RawFileHandle {
        RawFileHandleType::new(Rw2File {
            reader,
            type_id: OnceCell::new(),
            container: OnceCell::new(),
            thumbnails: OnceCell::new(),
            jpeg_preview: OnceCell::new(),
            probe: None,
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

    fn jpeg_preview(&self) -> Option<&jpeg::Container> {
        self.jpeg_preview
            .get_or_init(|| {
                self.jpeg_data_offset().and_then(|offset| {
                    let view = Viewer::create_view(&self.reader, offset.offset).ok()?;
                    Some(jpeg::Container::new(view, self.type_()))
                })
            })
            .as_ref()
    }
}

impl RawFileImpl for Rw2File {
    #[cfg(feature = "probe")]
    probe_imp!();

    fn identify_id(&self) -> TypeId {
        *self.type_id.get_or_init(|| {
            self.container();
            let container = self.container.get().unwrap();
            tiff::identify_with_exif(container, &MAKE_TO_ID_MAP).unwrap_or(panasonic!(UNKNOWN))
        })
    }

    fn container(&self) -> &dyn RawContainer {
        self.container.get_or_init(|| {
            // XXX we should be faillible here.
            let view = Viewer::create_view(&self.reader, 0).expect("Created view");
            let mut container = tiff::Container::new(
                view,
                vec![
                    (tiff::IfdType::Main, Some(&RAW_TAG_NAMES)),
                    (tiff::IfdType::Other, None),
                    (tiff::IfdType::Other, None),
                    (tiff::IfdType::Other, None),
                ],
                self.type_(),
            );
            container
                .load(Some(Box::new(Rw2Fixup {})))
                .expect("Rw2 container error");
            probe!(
                self.probe,
                "raw.container.endian",
                &format!("{:?}", container.endian())
            );
            container
        })
    }

    fn thumbnails(&self) -> &ThumbnailStorage {
        self.thumbnails.get_or_init(|| {
            let mut thumbnails = vec![];
            if let Some(jpeg) = self.jpeg_preview() {
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

                        probe!(self.probe, "rw2.jpeg_preview.thumbnail", "true");
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

            ThumbnailStorage::with_thumbnails(thumbnails)
        })
    }

    fn ifd(&self, ifd_type: tiff::IfdType) -> Option<&Dir> {
        self.container();
        let container = self.container.get().unwrap();
        match ifd_type {
            tiff::IfdType::Main | tiff::IfdType::Raw => container.directory(0),
            tiff::IfdType::Exif => self
                .jpeg_preview()
                .and_then(|jpeg| jpeg.exif_dir())
                .or_else(|| container.exif_dir()),
            tiff::IfdType::MakerNote => self.jpeg_preview().and_then(|jpeg| jpeg.mnote_dir()),
            _ => None,
        }
    }

    fn load_rawdata(&self, _skip_decompress: bool) -> Result<RawImage> {
        if let Some(cfa) = self.ifd(tiff::IfdType::Raw) {
            let offset: thumbnail::DataOffset =
                if let Some(offset) = cfa.uint_value(exif::RW2_TAG_RAW_OFFSET) {
                    if offset as u64 > self.reader.length() {
                        return Err(Error::FormatError);
                    }
                    let len = self.reader.length() - offset as u64;
                    log::debug!("Panasonic Raw offset: {}", offset);
                    probe!(self.probe, "rw2.raw_offset", true);
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
                    probe!(self.probe, "rw2.raw_tiff_offset", true);
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
            log::debug!(
                "real_len {real_len} width {width} height {height} pixel_count {pixel_count}"
            );
            let mut packed = false;
            let data_type = if real_len >= (pixel_count * 2) as u64 {
                probe!(self.probe, "rw2.compression", "unpacked");
                DataType::Raw
            } else if real_len >= (pixel_count * 3 / 2) as u64 {
                // Need to unpack
                probe!(self.probe, "rw2.compression", "packed");
                packed = true;
                DataType::Raw
            } else {
                probe!(self.probe, "rw2.compression", "panasonic");
                DataType::CompressedRaw
            };
            let mut raw_data = match data_type {
                DataType::CompressedRaw => {
                    let raw = self.container().load_buffer8(offset.offset, offset.len);
                    RawImage::with_data8(
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
                        log::debug!("Panasonic: packed data");
                        let raw = self.container().load_buffer8(offset.offset, offset.len);
                        let mut out = Vec::with_capacity(width as usize * height as usize);
                        decompress::unpack_be12to16(&raw, &mut out, tiff::Compression::None)?;
                        out
                    } else {
                        log::debug!("Panasonic: unpacked data");
                        self.container().load_buffer16(offset.offset, offset.len)
                    };
                    RawImage::with_data16(
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
                probe!(self.probe, "rw2.compression.value", compression);
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

            raw_data.set_active_area(Some(crate::Rect {
                x: x as u32,
                y: y as u32,
                width: w as u32,
                height: h as u32,
            }));

            let wr = cfa
                .uint_value(exif::RW2_TAG_LINEARITY_LIMIT_RED)
                .unwrap_or((1 << bpc) - 1) as u16;
            let wg = cfa
                .uint_value(exif::RW2_TAG_LINEARITY_LIMIT_GREEN)
                .unwrap_or((1 << bpc) - 1) as u16;
            let wb = cfa
                .uint_value(exif::RW2_TAG_LINEARITY_LIMIT_BLUE)
                .unwrap_or((1 << bpc) - 1) as u16;

            raw_data.set_whites([wr, wg, wb, wg]);

            let br = cfa
                .uint_value(exif::RW2_TAG_BLACK_LEVEL_RED)
                .unwrap_or((1 << bpc) - 1) as u16;
            let bg = cfa
                .uint_value(exif::RW2_TAG_BLACK_LEVEL_GREEN)
                .unwrap_or((1 << bpc) - 1) as u16;
            let bb = cfa
                .uint_value(exif::RW2_TAG_BLACK_LEVEL_BLUE)
                .unwrap_or((1 << bpc) - 1) as u16;
            raw_data.set_blacks([br, bg, bb, bg]);

            let wbr = cfa.uint_value(exif::RW2_TAG_WB_RED_LEVEL);
            let wbg = cfa.uint_value(exif::RW2_TAG_WB_GREEN_LEVEL);
            let wbb = cfa.uint_value(exif::RW2_TAG_WB_BLUE_LEVEL);

            if let Some(wbg) = wbg {
                probe!(self.probe, "rw2.wb.rgb", true);
                let wbg = wbg as f64;
                let wbr = wbr.map(|wbr| wbg / wbr as f64).unwrap_or(1.0);
                let wbb = wbb.map(|wbb| wbg / wbb as f64).unwrap_or(1.0);
                raw_data.set_as_shot_neutral(&[wbr, 1.0, wbb, f64::NAN]);
            } else {
                probe!(self.probe, "rw2.wb.rb_balance", true);
                let wbr = cfa.uint_value(exif::RW2_TAG_RED_BALANCE);
                let wbb = cfa.uint_value(exif::RW2_TAG_BLUE_BALANCE);
                #[allow(clippy::unnecessary_unwrap)]
                if wbr.is_some() && wbb.is_some() {
                    raw_data.set_as_shot_neutral(&[
                        256.0 / wbr.unwrap() as f64,
                        1.0,
                        256.0 / wbb.unwrap() as f64,
                        f64::NAN,
                    ]);
                }
            }
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
            let container = self.container.get().unwrap();
            container.write_dump(out, indent);
            if let Some(camera_tiff) = self.main_ifd().and_then(|dir| {
                // id = "PanasonicRaw.CameraIfd"
                dir.tiff_in_entry(
                    container,
                    exif::RW2_TAG_CAMERA_IFD,
                    Some(&RAW_CAMERAIFD_TAG_NAMES),
                )
            }) {
                if let Some(camera_ifd) = camera_tiff.directory(0) {
                    camera_ifd.write_dump(out, indent);
                }
            }
        }
        dump_writeln!(out, indent, "</Panasonic RW2 File>");
    }
}

dumpfile_impl!(Rw2File);
