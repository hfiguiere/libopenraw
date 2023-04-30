/*
 * libopenraw - olympus/matrices.rs
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

//! Olympus matrices

use crate::colour::BuiltinMatrix;
use crate::olympus;
use crate::TypeId;

lazy_static::lazy_static! {
    pub(super) static ref MATRICES: [BuiltinMatrix; 63] = [
    BuiltinMatrix::new( olympus!(E1),
      0,
      0,
      [ 11846, -4767, -945, -7027, 15878, 1089, -2699, 4122, 8311 ] ),
    BuiltinMatrix::new( olympus!(E10),
      0,
      0xffc,
      [ 12745, -4500, -1416, -6062, 14542, 1580, -1934, 2256, 6603 ] ),
    BuiltinMatrix::new( olympus!(E3),
      0,
      0xf99,
      [ 9487, -2875, -1115, -7533, 15606, 2010, -1618, 2100, 7389 ] ),
    BuiltinMatrix::new( olympus!(E5),
      0,
      0,
      [ 11200, -3783, -1325, -4576, 12593, 2206, -695, 1742, 7504 ] ),
    BuiltinMatrix::new( olympus!(E30),
      0,
      0,
      [ 8144, -1861, -1111, -7763, 15894, 1929, -1865, 2542, 7607 ] ),
    BuiltinMatrix::new( olympus!(E300),
      0,
      0,
      [ 7828, -1761, -348, -5788, 14071, 1830, -2853, 4518, 6557 ] ),
    BuiltinMatrix::new( olympus!(E330),
      0,
      0,
      [ 8961, -2473, -1084, -7979, 15990, 2067, -2319, 3035, 8249 ] ),
    BuiltinMatrix::new( olympus!(E400),
      0,
      0,
      [ 6169, -1483, -21, -7107, 14761, 2536, -2904, 3580, 8568 ] ),
    BuiltinMatrix::new( olympus!(E410),
      0,
      0xf6a,
      [ 8856, -2582, -1026, -7761, 15766, 2082, -2009, 2575, 7469 ] ),
    BuiltinMatrix::new( olympus!(E420),
      0,
      0,
      [ 8745, -2425, -1095, -7594, 15613, 2073, -1780, 2309, 7416 ] ),
    BuiltinMatrix::new( olympus!(E450),
      0,
      0,
      [ 8745, -2425, -1095, -7594, 15613, 2073, -1780, 2309, 7416 ] ),
    BuiltinMatrix::new( olympus!(E500),
      0,
      0,
      [ 8136, -1968, -299, -5481, 13742, 1871, -2556, 4205, 6630 ] ),
    BuiltinMatrix::new( olympus!(E510),
      0,
      0xf6a,
      [ 8785, -2529, -1033, -7639, 15624, 2112, -1783, 2300, 7817 ] ),
    BuiltinMatrix::new( olympus!(E520),
      0,
      0,
      [ 8785, -2529, -1033, -7639, 15624, 2112, -1783, 2300, 7817 ] ),
    BuiltinMatrix::new( olympus!(E600),
      0,
      0,
      [ 8453, -2198, -1092, -7609, 15681, 2008, -1725, 2337, 7824 ] ),
    BuiltinMatrix::new( olympus!(E620),
      0,
      0xfaf,
      [ 8453, -2198, -1092, -7609, 15681, 2008, -1725, 2337, 7824 ] ),
    BuiltinMatrix::new( olympus!(SP350),
      0,
      0,
      [ 12078, -4836, -1069, -6671, 14306, 2578, -786, 939, 7418 ] ),
    BuiltinMatrix::new( olympus!(SP500UZ),
      0,
      0xfff,
      [ 9493, -3415, -666, -5211, 12334, 3260, -1548, 2262, 6482 ] ),
    BuiltinMatrix::new( olympus!(SP510UZ),
      0,
      0xffe,
      [ 10593, -3607, -1010, -5881, 13127, 3084, -1200, 1805, 6721 ] ),
    BuiltinMatrix::new( olympus!(SP550UZ),
      0,
      0xffe,
      [ 11597, -4006, -1049, -5432, 12799, 2957, -1029, 1750, 6516 ] ),
    BuiltinMatrix::new( olympus!(SP565UZ),
      0,
      0xffe,
      [ 11856, -4470, -1159, -4814, 12368, 2756, -994, 1780, 5589 ] ),
    BuiltinMatrix::new( olympus!(SP570UZ),
      0,
      0xffe,
      [ 11522, -4044, -1145, -4737, 12172, 2903, -987, 1829, 6039 ] ),
    BuiltinMatrix::new( olympus!(EP1),
      0,
      0xffd,
      [ 8343, -2050, -1021, -7715, 15705, 2103, -1831, 2380, 8235 ] ),
    BuiltinMatrix::new( olympus!(EP2),
      0,
      0xffd,
      [ 8343, -2050, -1021, -7715, 15705, 2103, -1831, 2380, 8235 ] ),
    BuiltinMatrix::new( olympus!(EP3),
      0,
      0,
      [ 7575, -2159, -571, -3722, 11341, 2725, -1434, 2819, 6271 ] ),
    BuiltinMatrix::new( olympus!(EP5),
      0,
      0,
      [ 8745, -2425, -1095, -7594, 15613, 2073, -1780, 2309, 7416 ] ),
    BuiltinMatrix::new( olympus!(EP7),
      0,
      0,
      [ 9476, -3182, -765, -2613, 10958, 1893, -449, 1315, 5268 ] ),
    BuiltinMatrix::new( olympus!(EPL1),
      0,
      0,
      [ 11408, -4289, -1215, -4286, 12385, 2118, -387, 1467, 7787 ] ),
    BuiltinMatrix::new( olympus!(EPL2),
      0,
      0,
      [ 15030, -5552, -1806, -3987, 12387, 1767, -592, 1670, 7023 ] ),
    BuiltinMatrix::new( olympus!(EPL3),
      0,
      0,
      [ 7575, -2159, -571, -3722, 11341, 2725, -1434, 2819, 6271 ] ),
    BuiltinMatrix::new( olympus!(EPL5),
      0,
      0xfcb,
      [ 8380, -2630, -639, -2887, 10725, 2496, -627, 1427, 5438 ] ),
    BuiltinMatrix::new( olympus!(EPL6),
      0,
      0xfcb,
      [ 8380, -2630, -639, -2887, 10725, 2496, -627, 1427, 5438 ] ),
    BuiltinMatrix::new( olympus!(EPL7),
      0,
      0xfcb,
      [ 9197, -3190, -659, -2606, 10830, 2039, -458, 1250, 5458 ] ),
    BuiltinMatrix::new( olympus!(EPL8),
      0,
      0xfcb,
      [ 9197, -3190, -659, -2606, 10830, 2039, -458, 1250, 5458 ] ),
    BuiltinMatrix::new( olympus!(EPL9),
      0,
      0xfcb,
      [ 8380, -2630, -639, -2887, 10725, 2496, -627, 1427, 5438 ] ),
    BuiltinMatrix::new( olympus!(EPL10),
      0,
      0xfcb,
      [ 9197, -3190, -659, -2606, 10830, 2039, -458, 1250, 5458 ] ),
    BuiltinMatrix::new( olympus!(EPM1),
      0,
      0,
      [ 7575, -2159, -571, -3722, 11341, 2725, -1434, 2819, 6271 ] ),
    BuiltinMatrix::new( olympus!(EPM2),
      0,
      0,
      [ 8380, -2630, -639, -2887, 10725, 2496, -627, 1427, 5438 ] ),
    BuiltinMatrix::new( olympus!(XZ1),
      0,
      0,
      [ 10901, -4095, -1074, -1141, 9208, 2293, -62, 1417, 5158 ] ),
    BuiltinMatrix::new( olympus!(XZ10),
      0,
      0,
      [ 9777, -3483, -925, -2886, 11297, 1800, -602, 1663, 5134 ] ),
    BuiltinMatrix::new( olympus!(XZ2),
      0,
      0,
      [ 9777, -3483, -925, -2886, 11297, 1800, -602, 1663, 5134 ] ),
    BuiltinMatrix::new( olympus!(EM5),
      0,
      0xfe1,
      [ 8380, -2630, -639, -2887, 725, 2496, -627, 1427, 5438 ] ),
    BuiltinMatrix::new( olympus!(EM5II),
      0,
      0,
      [ 9422, -3258, -711, -2655, 10898, 2015, -512, 1354, 5512 ] ),
    BuiltinMatrix::new( olympus!(EM5III),
      0,
      0,
      [ 11896, -5110, -1076, -3181, 11378, 2048, -519, 1224, 5166 ] ),
    BuiltinMatrix::new( olympus!(EM1),
      0,
      0,
      [ 7687, -1984, -606, -4327, 11928, 2721, -1381, 2339, 6452 ] ),
    BuiltinMatrix::new( olympus!(EM1II),
      0,
      0,
      [ 9383, -3170, -763, -2457, 10702, 2020, -384, 1236, 5552 ] ),
    BuiltinMatrix::new( olympus!(EM1III),
      0,
      0,
      [ 11896, -5110, -1076, -3181, 11378, 2048, -519, 1224, 5166 ] ),
    BuiltinMatrix::new( olympus!(EM10),
      0,
      0,
      [ 8380, -2630, -639, -2887, 10725, 2496, -627, 1427, 5438 ] ),
    BuiltinMatrix::new( olympus!(EM10II), // Identical to MarkI
      0,
      0,
      [ 8380, -2630, -639, -2887, 10725, 2496, -627, 1427, 5438 ] ),
    BuiltinMatrix::new( olympus!(EM10III),
      0,
      0,
      [ 8380, -2630, -639, -2887, 10725, 2496, -627, 1427, 5438 ] ),
    BuiltinMatrix::new( olympus!(EM10IIIS),
      0,
      0,
      [ 8380, -2630, -639, -2887, 10725, 2496, -627, 1427, 5438 ] ),
    BuiltinMatrix::new( olympus!(EM10IV),
      0,
      0,
      [ 9476, -3182, -765, -2613, 10958, 1893, -449, 1315, 5268 ] ),
    BuiltinMatrix::new( olympus!(OM1),
      0,
      0,
      [ 9488, -3984, -714, -2887, 10945, 2229, -137, 960, 5786 ] ),
    BuiltinMatrix::new( olympus!(OM5),
      0,
      0,
      [ 11896, -5110, -1076, -3181, 11378, 2048, -519, 1224, 5166 ] ),
    BuiltinMatrix::new( olympus!(EM1X),
      0,
      0,
      [ 11896, -5110, -1076, -3181, 11378, 2048, -519, 1224, 5166 ] ),
    BuiltinMatrix::new( olympus!(STYLUS1),
      0,
      0,
      [ 8360, -2420, -880, -3928, 12353, 1739, -1381, 2416, 5173 ] ),
    BuiltinMatrix::new( olympus!(STYLUS1_1S),
      0,
      0,
      [ 8360, -2420, -880, -3928, 12353, 1739, -1381, 2416, 5173 ] ),
    BuiltinMatrix::new( olympus!(PEN_F),
      0,
      0,
      [ 9476, -3182, -765, -2613, 10958, 1893, -449, 1315, 5268 ] ),
    BuiltinMatrix::new( olympus!(SH2),
      0,
      0,
      [ 10156, -3425, -1077, -2611, 11177, 1624, -385, 1592, 5080 ] ),
    BuiltinMatrix::new( olympus!(TG4),
      0,
      0,
      [ 11426, -4159, -1126, -2066, 10678, 1593, -120, 1327, 4998 ] ),
    BuiltinMatrix::new( olympus!(TG5),
      0,
      0,
      [ 10899, -3833, -1082, -2112, 10736, 1575, -267, 1452, 5269 ] ),
    BuiltinMatrix::new( olympus!(TG6),
      0,
      0,
      [ 10899, -3833, -1082, -2112, 10736, 1575, -267, 1452, 5269 ] ),
    BuiltinMatrix::new( olympus!(C5060WZ),
      0,
      0,
      [ 10445, -3362, -1307, -7662, 15690, 2058, -1135, 1176, 7602 ] ),
    ];
}
