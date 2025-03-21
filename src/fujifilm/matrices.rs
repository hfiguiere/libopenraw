// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - fujifilm/matrices.rs
 *
 * Copyright (C) 2022-2025 Hubert Figuière
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

//! Fujifilm colour matrices

use crate::colour::BuiltinMatrix;
use crate::fuji;
use crate::TypeId;

lazy_static::lazy_static! {
    pub(super) static ref MATRICES: [BuiltinMatrix; 73] = [
        BuiltinMatrix::new(
            fuji!(F550EXR),
            0,
            0,
            [ 1369, -5358, -1474, -3369, 11600, 1998, -132, 1554, 4395 ] ),
        BuiltinMatrix::new(
            fuji!(F700),
            0,
            0,
            [ 10004, -3219, -1201, -7036, 15047, 2107, -1863, 2565, 7736 ] ),
        BuiltinMatrix::new(
            fuji!(F810),
            0,
            0,
            [ 11044, -3888, -1120, -7248, 15168, 2208, -1531, 2277, 8069 ] ),
        BuiltinMatrix::new(
            fuji!(E900),
            0,
            0,
            [ 9183, -2526, -1078, -7461, 15071, 2574, -2022, 2440, 8639 ] ),
        BuiltinMatrix::new(
            fuji!(S2PRO),
            128,
            0,
            [ 12492, -4690, -1402, -7033, 15423, 1647, -1507, 2111, 7697 ] ),
        BuiltinMatrix::new(
            fuji!(S3PRO),
            0,
            0,
            [ 11807, -4612, -1294, -8927, 16968, 1988, -2120, 2741, 8006 ] ),
        BuiltinMatrix::new(
            fuji!(S5PRO),
            0,
            0,
            [ 12300, -5110, -1304, -9117, 17143, 1998, -1947, 2448, 8100 ] ),
        BuiltinMatrix::new(
            fuji!(S5000),
            0,
            0,
            [ 8754, -2732, -1019, -7204, 15069, 2276, -1702, 2334, 6982 ] ),
        BuiltinMatrix::new(
            fuji!(S5600),
            0,
            0,
            [ 9636, -2804, -988, -7442, 15040, 2589, -1803, 2311, 8621 ] ),
        BuiltinMatrix::new(
            fuji!(S9500),
            0,
            0,
            [ 10491, -3423, -1145, -7385, 15027, 2538, -1809, 2275, 8692 ] ),
        BuiltinMatrix::new(
            fuji!(S6000FD),
            0,
            0,
            [ 12628, -4887, -1401, -6861, 14996, 1962, -2198, 2782, 7091 ] ),
        BuiltinMatrix::new(
            fuji!(S6500FD),
            0,
            0,
            [ 12628, -4887, -1401, -6861, 14996, 1962, -2198, 2782, 7091 ] ),
        BuiltinMatrix::new(
            fuji!(SL1000),
            0,
            0,
            [ 11705, -4262, -1107, -2282, 10791, 1709, -555, 1713, 4945 ] ),
        BuiltinMatrix::new(
            fuji!(HS10),
            0,
            0xf68,
            [ 12440, -3954, -1183, -1123, 9674, 1708, -83, 1614, 4086 ] ),
        // HS33EXR is an alias of this.
        BuiltinMatrix::new(
            fuji!(HS30EXR),
            0,
            0,
            [ 1369, -5358, -1474, -3369, 11600, 1998, -132, 1554, 4395 ] ),
        BuiltinMatrix::new(
            fuji!(HS50EXR),
            0,
            0,
            [ 12085, -4727, -953, -3257, 11489, 2002, -511, 2046, 4592 ] ),
        BuiltinMatrix::new(
            fuji!(X100),
            0,
            0,
            [ 12161, -4457, -1069, -5034, 12874, 2400, -795, 1724, 6904 ] ),
        BuiltinMatrix::new(
            fuji!(X100S),
            0,
            0,
            [ 10592, -4262, -1008, -3514, 11355, 2465, -870, 2025, 6386 ] ),
        BuiltinMatrix::new(
            fuji!(X100T),
            0,
            0,
            [ 10592, -4262, -1008, -3514, 11355, 2465, -870, 2025, 6386 ] ),
        BuiltinMatrix::new(
            fuji!(X100F),
            0,
            0,
            [ 11434, -4948, -1210, -3746, 12042, 1903, -666, 1479, 5235 ] ),
        BuiltinMatrix::new(
            fuji!(X100V),
            0,
            0,
            [ 13426, -6334, -1177, -4244, 12136, 2371, -580, 1303, 5980 ] ),
        BuiltinMatrix::new(
            fuji!(X100VI),
            0,
            0,
            [ 11809, -5358, -1141, -4248, 12164, 2343, -514, 1097, 5848 ] ),
        BuiltinMatrix::new(
            fuji!(X10),
            0,
            0,
            [ 13509, -6199, -1254, -4430, 12733, 1865, -331, 1441, 5022 ] ),
        BuiltinMatrix::new(
            fuji!(X20),
            0,
            0,
            [ 11768, -4971, -1133, -4904, 12927, 2183, -480, 1723, 4605 ] ),
        BuiltinMatrix::new(
            fuji!(X30),
            0,
            0,
            [ 12328, -5256, -1144, -4469, 12927, 1675, -87, 1291, 4351 ] ),
        BuiltinMatrix::new(
            fuji!(X70),
            0,
            0,
            [ 10450, -4329, -878, -3217, 11105, 2421, -752, 1758, 6519 ] ),
        BuiltinMatrix::new(
            fuji!(XPRO1),
            0,
            0,
            [ 10413, -3996, -993, -3721, 11640, 2361, -733, 1540, 6011 ] ),
        BuiltinMatrix::new(
            fuji!(XPRO2),
            0,
            0,
            [ 11434, -4948, -1210, -3746, 12042, 1903, -666, 1479, 5235 ] ),
        BuiltinMatrix::new(
            fuji!(XPRO3),
            0,
            0,
            [ 13426, -6334, -1177, -4244, 12136, 2371, -580, 1303, 5980 ] ),
        BuiltinMatrix::new(
            fuji!(XA1),
            0,
            0,
            [ 11086, -4555, -839, -3512, 11310, 2517, -815, 1341, 5940 ] ),
        BuiltinMatrix::new(
            fuji!(XA10),
            0,
            0,
            [ 11540, -4999, -991, -2949, 10963, 2278, -382, 1049, 5605 ] ),
        BuiltinMatrix::new(
            fuji!(XA2),
            0,
            0,
            [ 10763, -4560, -917, -3346, 11311, 2322, -475, 1135, 5843 ] ),
        BuiltinMatrix::new(
            fuji!(XA3),
            0,
            0,
            [ 12407, -5222, -1086, -2971, 11116, 2120, -294, 1029, 5284 ] ),
        BuiltinMatrix::new(
            fuji!(XA5),
            0,
            0,
            [ 11673, -476, -1041, -3988, 12058, 2166, -771, 1417, 5569 ] ),
        BuiltinMatrix::new(
            fuji!(XA7),
            0,
            0,
            [ 15055, -7391, -1274, -4062, 12071, 2238, -610, 1217, 6147 ] ),
        BuiltinMatrix::new(
            fuji!(XQ1),
            0,
            0,
            [ 9252, -2704, -1064, -5893, 14265, 1717, -1101, 2341, 4349 ] ),
        BuiltinMatrix::new(
            fuji!(XQ2),
            0,
            0,
            [ 9252, -2704, -1064, -5893, 14265, 1717, -1101, 2341, 4349 ] ),
        BuiltinMatrix::new(
            fuji!(XE1),
            0,
            0,
            [ 10413, -3996, -993, -3721, 11640, 2361, -733, 1540, 6011 ] ),
        BuiltinMatrix::new(
            fuji!(XE2),
            0,
            0,
            [ 8458, -2451, -855, -4597, 12447, 2407, -1475, 2482, 6526 ] ),
        BuiltinMatrix::new(
            fuji!(XE2S),
            0,
            0,
            [ 11562, -5118, -961, -3022, 11007, 2311, -525, 1569, 6097 ] ),
        BuiltinMatrix::new(
            fuji!(XE3),
            0,
            0,
            [ 11434, -4948, -1210, -3746, 12042, 1903, -666, 1479, 5235 ] ),
        BuiltinMatrix::new(
            fuji!(XE4),
            0,
            0,
            [ 13426, -6334, -1177, -4244, 12136, 2371, -580, 1303, 5980 ] ),
        BuiltinMatrix::new(
            fuji!(XH1),
            0,
            0,
            [ 11434, -4948, -1210, -3746, 12042, 1903, -666, 1479, 5235 ] ),
        BuiltinMatrix::new(
            fuji!(XH2),
            0,
            0,
            [ 11809, -5358, -1141, -4248, 12164, 2343, -514, 1097, 5848 ] ),
        BuiltinMatrix::new(
            fuji!(XH2S),
            0,
            0,
            [ 12836, -5909, -1032, -3087, 11132, 2236, -35, 872, 5330 ] ),
        BuiltinMatrix::new(
            fuji!(XM1),
            0,
            0,
            [ 10413, -3996, -993, -3721, 11640, 2361, -733, 1540, 6011 ] ),
        // XM5 is just a guess ATM, that it's the same sensor as the XS20.
        BuiltinMatrix::new(
            fuji!(XM5),
            0,
            0,
            [ 12836, -5909, -1032, -3086, 11132, 2236, -35, 872, 5330 ] ),
        BuiltinMatrix::new(
            fuji!(XT1),
            0,
            0,
            [ 8458, -2451, -855, -4597, 12447, 2407, -1475, 2482, 6526 ] ),
        BuiltinMatrix::new(
            fuji!(XT10),
            0,
            0,
            [ 8458, -2451, -855, -4597, 12447, 2407, -1475, 2482, 6526 ] ),
        BuiltinMatrix::new(
            fuji!(XT100),
            0,
            0,
            [ 11673, -476, -1041, -3988, 12058, 2166, -771, 1417, 5569 ] ),
        BuiltinMatrix::new(
            fuji!(XT2),
            0,
            0,
            [ 11434, -4948, -1210, -3746, 12042, 1903, -666, 1479, 5235 ] ),
        BuiltinMatrix::new(
            fuji!(XT20),
            0,
            0,
            [ 11434, -4948, -1210, -3746, 12042, 1903, -666, 1479, 5235 ] ),
        BuiltinMatrix::new(
            fuji!(XT200),
            0,
            0,
            [ 15055, -7391, -1274, -4062, 12071, 2238, -610, 1217, 6147 ] ),
        BuiltinMatrix::new(
            fuji!(XT3),
            0,
            0,
            [ 16393, -7740, -1436, -4238, 12131, 2371, -633, 1424, 6553 ] ),
        BuiltinMatrix::new(
            fuji!(XT30),
            0,
            0,
            [ 13426, -6334, -1177, -4244, 12136, 2371, -580, 1303, 5980 ] ),
        BuiltinMatrix::new(
            fuji!(XT30_II),
            0,
            0,
            [ 13426, -6334, -1177, -4244, 12136, 2371, -580, 1303, 5980 ] ),
        BuiltinMatrix::new(
            fuji!(XT4),
            0,
            0,
            [ 13426, -6334, -1177, -4244, 12136, 2371, -580, 1303, 5980 ] ),
        BuiltinMatrix::new(
            fuji!(XT5),
            0,
            0,
            [ 11809, -5358, -1141, -4248, 12164, 2343, -514, 1097, 5848 ] ),
        BuiltinMatrix::new(
            fuji!(XT50),
            0,
            0,
            [ 11809, -5358, -1141, -4248, 12164, 2343, -514, 1097, 5848 ] ),
        BuiltinMatrix::new(
            fuji!(XS1),
            0,
            0,
            [ 13509, -6199, -1254, -4430, 12733, 1865, -331, 1441, 5022 ] ),
        BuiltinMatrix::new(
            fuji!(XS10),
            0,
            0,
            [ 13426, -6334, -1177, -4244, 12136, 2371, -580, 1303, 5980 ] ),
        BuiltinMatrix::new(
            fuji!(XS20),
            0,
            0,
            [ 12836, -5909, -1032, -3086, 11132, 2236, -35, 872, 5330 ] ),
        BuiltinMatrix::new(
            fuji!(XF1),
            0,
            0,
            [ 13509, -6199, -1254, -4430, 12733, 1865, -331, 1441, 5022 ] ),
        BuiltinMatrix::new(
            fuji!(XF10),
            0,
            0,
            [ 11673, -476, -1041, -3988, 12058, 2166, -771, 1417, 5569 ] ),
        BuiltinMatrix::new(
            fuji!(S200EXR),
            512,
            0x3fff,
            [ 11401, -4498, -1312, -5088, 12751, 2613, -838, 1568, 5941 ] ),
        BuiltinMatrix::new(
            fuji!(S100FS),
            512,
            0x3fff,
            [ 11521, -4355, -1065, -6524, 13768, 3059, -1466, 1984, 6045 ] ),
        BuiltinMatrix::new(
            fuji!(GFX50S),
            0,
            0,
            [ 11756, -4754, -874, -3056, 11045, 2305, -381, 1457, 6006 ] ),
        BuiltinMatrix::new(
            fuji!(GFX50S_II),
            0,
            0,
            [ 11756, -4754, -874, -3056, 11045, 2305, -381, 1457, 6006 ] ),
        // For now we assume it is the same sensor as the GFX50S
        BuiltinMatrix::new(
            fuji!(GFX50R),
            0,
            0,
            [ 11756, -4754, -874, -3056, 11045, 2305, -381, 1457, 6006 ] ),
        BuiltinMatrix::new(
            fuji!(GFX100),
            0,
            0,
            [ 16212, -8423, -1583, -4336, 12583, 1937, -195, 726, 6199 ] ),
        BuiltinMatrix::new(
            fuji!(GFX100_II),
            0,
            0,
            [ 12806, -5779, -1110, -3546, 11507, 2318, -177, 995, 5715 ] ),
        BuiltinMatrix::new(
            fuji!(GFX100S),
            0,
            0,
            [ 16212, -8423, -1583, -4336, 12583, 1937, -195, 726, 6199 ] ),
        BuiltinMatrix::new(
            fuji!(GFX100RF),
            0,
            0,
            [ 12806, -5779, -1110, -3546, 11507, 2318, -177, 995, 5715 ] ),
    ];
}
