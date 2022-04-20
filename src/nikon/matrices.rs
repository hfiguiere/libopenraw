/*
 * libopenraw - nikon/matrices.rs
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

//! Nikon xyz to rgb matrices.

use crate::camera_ids::{nikon, vendor};
use crate::colour::BuiltinMatrix;
use crate::TypeId;

lazy_static::lazy_static! {
    pub(super) static ref MATRICES: [BuiltinMatrix; 83] = [
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D1),
            0,
            0, /* multiplied by 2.218750, 1.0, 1.148438 */
            [16772, -4726, -2141, -7611, 15713, 1972, -2846, 3494, 9521],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D100),
            0,
            0,
            [5902, -933, -782, -8983, 16719, 2354, -1402, 1455, 6464],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D1X),
            0,
            0,
            [7702, -2245, -975, -9114, 17242, 1875, -2679, 3055, 8521],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D200),
            0,
            0xfbc,
            [8367, -2248, -763, -8758, 16447, 2422, -1527, 1550, 8053],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D2H),
            0,
            0,
            [5710, -901, -615, -8594, 16617, 2024, -2975, 4120, 6830],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D2HS),
            0,
            0,
            [5710, -901, -615, -8594, 16617, 2024, -2975, 4120, 6830],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D2X),
            0,
            0,
            [10231, -2769, -1255, -8301, 15900, 2552, -797, 680, 7148],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D2XS),
            0,
            0,
            [10231, -2769, -1255, -8301, 15900, 2552, -797, 680, 7148],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D3),
            0,
            0,
            [8139, -2171, -663, -8747, 16541, 2295, -1925, 2008, 8093],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D3S),
            0,
            0,
            [8828, -2406, -694, -4874, 12603, 2541, -660, 1509, 7587],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D3X),
            0,
            0,
            [7171, -1986, -648, -8085, 15555, 2718, -2170, 2512, 7457],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D300),
            0,
            0,
            [9030, -1992, -715, -8465, 16302, 2255, -2689, 3217, 8069],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D300S),
            0,
            0,
            [9030, -1992, -715, -8465, 16302, 2255, -2689, 3217, 8069],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D3000),
            0,
            0,
            [8736, -2458, -935, -9075, 16894, 2251, -1354, 1242, 8263],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D3100),
            0,
            0,
            [7911, -2167, -813, -5327, 13150, 2408, -1288, 2483, 7968],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D3200),
            0,
            0xfb9,
            [7013, -1408, -635, -5268, 12902, 2640, -1470, 2801, 7379],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D3300),
            0,
            0,
            [6988, -1384, -714, -5631, 13410, 2447, -1485, 2204, 7318],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D3400),
            0,
            0,
            [6988, -1384, -714, -5631, 13410, 2447, -1485, 2204, 7318],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D3500),
            0,
            0,
            [8821, -2938, -785, -4178, 12142, 2287, -824, 1651, 6860],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D4),
            0,
            0,
            [8598, -2848, -857, -5618, 13606, 2195, -1002, 1773, 7137],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D4S),
            0,
            0,
            [8598, -2848, -857, -5618, 13606, 2195, -1002, 1773, 7137],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D40),
            0,
            0,
            [6992, -1668, -806, -8138, 15748, 2543, -874, 850, 7897],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D40X),
            0,
            0,
            [8819, -2543, -911, -9025, 16928, 2151, -1329, 1213, 8449],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D5),
            0,
            0,
            [9200, -3522, -992, -5755, 13803, 2117, -753, 1486, 6338],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D50),
            0,
            0,
            [7732, -2422, -789, -8238, 15884, 2498, -859, 783, 7330],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D500),
            0,
            0,
            [8813, -3210, -1036, -4703, 12868, 2021, -1054, 1940, 6129],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D5000),
            0,
            0xf00,
            [7309, -1403, -519, -8474, 16008, 2622, -2433, 2826, 8064],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D5100),
            0,
            0x3de6,
            [8198, -2239, -724, -4871, 12389, 2798, -1043, 2050, 7181],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D5200),
            0,
            0,
            [8322, -3112, -1047, -6367, 14342, 2179, -988, 1638, 6394],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D5300),
            0,
            0,
            [6988, -1384, -714, -5631, 13410, 2447, -1485, 2204, 7318],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D5500),
            0,
            0,
            [8821, -2938, -785, -4178, 12142, 2287, -824, 1651, 6860],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D5600),
            0,
            0,
            [8821, -2938, -785, -4178, 12142, 2287, -824, 1651, 6860],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D6),
            0,
            0,
            [9028, -3423, -1035, -6321, 14265, 2217, -1013, 1683, 6928],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D60),
            0,
            0,
            [8736, -2458, -935, -9075, 16894, 2251, -1354, 1242, 8263],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D600),
            0,
            0,
            [8139, -2171, -663, -8747, 16541, 2295, -1925, 2008, 8093],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D610),
            0,
            0,
            [8139, -2171, -663, -8747, 16541, 2295, -1925, 2008, 8093],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D70),
            0,
            0,
            [7732, -2422, -789, -8238, 15884, 2498, -859, 783, 7330],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D70S),
            0,
            0,
            [7732, -2422, -789, -8238, 15884, 2498, -859, 783, 7330],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D700),
            0,
            0,
            [8139, -2171, -663, -8747, 16541, 2295, -1925, 2008, 8093],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D7000),
            0,
            0,
            [8198, -2239, -724, -4871, 12389, 2798, -1043, 2050, 7181],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D7100),
            0,
            0,
            [8322, -3112, -1047, -6367, 14342, 2179, -988, 1638, 6394],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D7200),
            0,
            0,
            [8322, -3112, -1047, -6367, 14342, 2179, -988, 1638, 6394],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D750),
            0,
            0,
            [9020, -2890, -715, -4535, 12436, 2348, -934, 1919, 7086],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D780),
            0,
            0,
            [9943, -3269, -839, -5323, 13269, 2259, -1198, 2083, 7557],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D80),
            0,
            0,
            [8629, -2410, -883, -9055, 16940, 2171, -1490, 1363, 8520],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D800),
            0,
            0,
            [7866, -2108, -555, -4869, 12483, 2681, -1176, 2069, 7501],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D800E),
            0,
            0,
            [7866, -2108, -555, -4869, 12483, 2681, -1176, 2069, 7501],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D810),
            0,
            0,
            [9369, -3195, -791, -4488, 12430, 2301, -893, 1796, 6872],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::D90),
            0,
            0xf00,
            [7309, -1403, -519, -8474, 16008, 2622, -2434, 2826, 8064],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::Z6),
            0,
            0,
            [8210, -2534, -683, -5355, 13338, 2212, -1143, 1929, 6464],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::Z6_2),
            0,
            0,
            [9943, -3269, -839, -5323, 13269, 2259, -1198, 2083, 7557],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::Z7),
            0,
            0,
            [10405, -3755, -1270, -5461, 13787, 1793, -1040, 2015, 6785],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::Z7_2),
            0,
            0,
            [13705, -6004, -1400, -5464, 13568, 2062, -940, 1706, 7618],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::Z9),
            0,
            0,
            [13389, -6049, -1441, -4544, 12757, 1969, 229, 498, 7390],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::Z50),
            0,
            0,
            [11640, -4829, -1079, -5107, 13006, 2325, -972, 1711, 7380],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::Z5),
            0,
            0,
            [8695, -2558, -648, -5015, 12711, 2575, -1279, 2215, 7514],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::ZFC),
            0,
            0,
            [11640, -4829, -1079, -5107, 13006, 2325, -972, 1711, 7380],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::DF),
            0,
            0,
            [8598, -2848, -857, -5618, 13606, 2195, -1002, 1773, 7137],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::E5400),
            0,
            0,
            [9349, -2987, -1001, -7919, 15766, 2266, -2098, 2680, 6839],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::E8400),
            0,
            0,
            [7842, -2320, -992, -8154, 15718, 2599, -1098, 1342, 7560],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::E8800),
            0,
            0,
            [7971, -2314, -913, -8451, 15762, 2894, -1442, 1520, 7610],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::COOLPIX_B700),
            200,
            0,
            [14387, -6014, -1299, -1357, 9975, 1616, 467, 1047, 4744],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::COOLPIX_P1000),
            0,
            0,
            [14294, -6116, -1333, -1628, 10219, 1637, -14, 1158, 5022],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::COOLPIX_P330),
            200,
            0,
            [10321, -3920, -931, -2750, 11146, 1824, -442, 1545, 5539],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::COOLPIX_P340),
            200,
            0,
            [10321, -3920, -931, -2750, 11146, 1824, -442, 1545, 5539],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::COOLPIX_P950),
            0,
            0,
            [13307, -5641, -1290, -2048, 10581, 1689, -64, 1222, 5176],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::COOLPIX_P6000),
            0,
            0,
            [9698, -3367, -914, -4706, 12584, 2368, -837, 968, 5801],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::COOLPIX_P7000),
            0,
            0,
            [11432, -3679, -1111, -3169, 11239, 2202, -791, 1380, 4455],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::COOLPIX_P7100),
            0,
            0,
            [11053, -4269, -1024, -1976, 10182, 2088, -526, 1263, 4469],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::COOLPIX_P7700),
            0,
            0,
            [10321, -3920, -931, -2750, 11146, 1824, -442, 1545, 5539],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::NIKON1_AW1),
            0,
            0,
            [6588, -1305, -693, -3277, 10987, 2634, -355, 2016, 5106],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::NIKON1_J1),
            0,
            0,
            [8994, -2667, -865, -4594, 12324, 2552, -699, 1786, 6260],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::NIKON1_J2),
            0,
            0,
            [8994, -2667, -865, -4594, 12324, 2552, -699, 1786, 6260],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::NIKON1_J3),
            0,
            0,
            [6588, -1305, -693, -3277, 10987, 2634, -355, 2016, 5106],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::NIKON1_J4),
            0,
            0,
            [5958, -1559, -571, -4021, 11453, 2939, -634, 1548, 5087],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::NIKON1_J5),
            0,
            0,
            [7520, -2518, -645, -3844, 12102, 1945, -913, 2249, 6835],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::NIKON1_V1),
            0,
            0,
            [8994, -2667, -865, -4594, 12324, 2552, -699, 1786, 6260],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::NIKON1_V2),
            0,
            0,
            [6588, -1305, -693, -3277, 10987, 2634, -355, 2016, 5106],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::NIKON1_V3),
            0,
            0,
            [5958, -1559, -571, -4021, 11453, 2939, -634, 1548, 5087],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::NIKON1_S1),
            0,
            0,
            [8994, -2667, -865, -4594, 12324, 2552, -699, 1786, 6260],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::NIKON1_S2),
            0,
            0,
            [6612, -1342, -618, -3338, 11055, 2623, -174, 1792, 5075],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::COOLPIX_A),
            0,
            0,
            [8198, -2239, -724, -4871, 12389, 2798, -1043, 2050, 7181],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::NIKON, nikon::COOLPIX_A1000),
            0,
            0,
            [10601, -3487, -1127, -2931, 11443, 1676, -587, 1740, 5278],
        ),
    ];

}
