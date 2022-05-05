// SPDX-License-Identifier: LGPL-3.0-or-later

//! Fujifilm colour matrices

use crate::camera_ids::{fujifilm, vendor};
use crate::colour::BuiltinMatrix;
use crate::TypeId;

lazy_static::lazy_static! {
    pub(super) static ref MATRICES: [BuiltinMatrix; 60] = [
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::F550EXR),
            0,
            0,
            [ 1369, -5358, -1474, -3369, 11600, 1998, -132, 1554, 4395 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::F700),
            0,
            0,
            [ 10004, -3219, -1201, -7036, 15047, 2107, -1863, 2565, 7736 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::F810),
            0,
            0,
            [ 11044, -3888, -1120, -7248, 15168, 2208, -1531, 2277, 8069 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::E900),
            0,
            0,
            [ 9183, -2526, -1078, -7461, 15071, 2574, -2022, 2440, 8639 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::S2PRO),
            128,
            0,
            [ 12492, -4690, -1402, -7033, 15423, 1647, -1507, 2111, 7697 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::S3PRO),
            0,
            0,
            [ 11807, -4612, -1294, -8927, 16968, 1988, -2120, 2741, 8006 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::S5PRO),
            0,
            0,
            [ 12300, -5110, -1304, -9117, 17143, 1998, -1947, 2448, 8100 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::S5000),
            0,
            0,
            [ 8754, -2732, -1019, -7204, 15069, 2276, -1702, 2334, 6982 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::S5600),
            0,
            0,
            [ 9636, -2804, -988, -7442, 15040, 2589, -1803, 2311, 8621 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::S9500),
            0,
            0,
            [ 10491, -3423, -1145, -7385, 15027, 2538, -1809, 2275, 8692 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::S6500FD),
            0,
            0,
            [ 12628, -4887, -1401, -6861, 14996, 1962, -2198, 2782, 7091 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::HS10),
            0,
            0xf68,
            [ 12440, -3954, -1183, -1123, 9674, 1708, -83, 1614, 4086 ] ),
        // HS33EXR is an alias of this.
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::HS30EXR),
            0,
            0,
            [ 1369, -5358, -1474, -3369, 11600, 1998, -132, 1554, 4395 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::X100),
            0,
            0,
            [ 12161, -4457, -1069, -5034, 12874, 2400, -795, 1724, 6904 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::X100S),
            0,
            0,
            [ 10592, -4262, -1008, -3514, 11355, 2465, -870, 2025, 6386 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::X100T),
            0,
            0,
            [ 10592, -4262, -1008, -3514, 11355, 2465, -870, 2025, 6386 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::X100F),
            0,
            0,
            [ 11434, -4948, -1210, -3746, 12042, 1903, -666, 1479, 5235 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::X100V),
            0,
            0,
            [ 13426, -6334, -1177, -4244, 12136, 2371, -580, 1303, 5980 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::X10),
            0,
            0,
            [ 13509, -6199, -1254, -4430, 12733, 1865, -331, 1441, 5022 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::X20),
            0,
            0,
            [ 11768, -4971, -1133, -4904, 12927, 2183, -480, 1723, 4605 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::X30),
            0,
            0,
            [ 12328, -5256, -1144, -4469, 12927, 1675, -87, 1291, 4351 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::X70),
            0,
            0,
            [ 10450, -4329, -878, -3217, 11105, 2421, -752, 1758, 6519 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::XPRO1),
            0,
            0,
            [ 10413, -3996, -993, -3721, 11640, 2361, -733, 1540, 6011 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::XPRO2),
            0,
            0,
            [ 11434, -4948, -1210, -3746, 12042, 1903, -666, 1479, 5235 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::XPRO3),
            0,
            0,
            [ 13426, -6334, -1177, -4244, 12136, 2371, -580, 1303, 5980 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::XA1),
            0,
            0,
            [ 11086, -4555, -839, -3512, 11310, 2517, -815, 1341, 5940 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::XA2),
            0,
            0,
            [ 10763, -4560, -917, -3346, 11311, 2322, -475, 1135, 5843 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::XA3),
            0,
            0,
            [ 12407, -5222, -1086, -2971, 11116, 2120, -294, 1029, 5284 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::XA5),
            0,
            0,
            [ 11673, -476, -1041, -3988, 12058, 2166, -771, 1417, 5569 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::XA7),
            0,
            0,
            [ 15055, -7391, -1274, -4062, 12071, 2238, -610, 1217, 6147 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::XQ1),
            0,
            0,
            [ 9252, -2704, -1064, -5893, 14265, 1717, -1101, 2341, 4349 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::XQ2),
            0,
            0,
            [ 9252, -2704, -1064, -5893, 14265, 1717, -1101, 2341, 4349 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::XE1),
            0,
            0,
            [ 10413, -3996, -993, -3721, 11640, 2361, -733, 1540, 6011 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::XE2),
            0,
            0,
            [ 8458, -2451, -855, -4597, 12447, 2407, -1475, 2482, 6526 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::XE2S),
            0,
            0,
            [ 11562, -5118, -961, -3022, 11007, 2311, -525, 1569, 6097 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::XE3),
            0,
            0,
            [ 11434, -4948, -1210, -3746, 12042, 1903, -666, 1479, 5235 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::XE4),
            0,
            0,
            [ 13426, -6334, -1177, -4244, 12136, 2371, -580, 1303, 5980 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::XH1),
            0,
            0,
            [ 11434, -4948, -1210, -3746, 12042, 1903, -666, 1479, 5235 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::XM1),
            0,
            0,
            [ 10413, -3996, -993, -3721, 11640, 2361, -733, 1540, 6011 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::XT1),
            0,
            0,
            [ 8458, -2451, -855, -4597, 12447, 2407, -1475, 2482, 6526 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::XT10),
            0,
            0,
            [ 8458, -2451, -855, -4597, 12447, 2407, -1475, 2482, 6526 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::XT100),
            0,
            0,
            [ 11673, -476, -1041, -3988, 12058, 2166, -771, 1417, 5569 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::XT2),
            0,
            0,
            [ 11434, -4948, -1210, -3746, 12042, 1903, -666, 1479, 5235 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::XT20),
            0,
            0,
            [ 11434, -4948, -1210, -3746, 12042, 1903, -666, 1479, 5235 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::XT200),
            0,
            0,
            [ 15055, -7391, -1274, -4062, 12071, 2238, -610, 1217, 6147 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::XT3),
            0,
            0,
            [ 16393, -7740, -1436, -4238, 12131, 2371, -633, 1424, 6553 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::XT30),
            0,
            0,
            [ 13426, -6334, -1177, -4244, 12136, 2371, -580, 1303, 5980 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::XT30_II),
            0,
            0,
            [ 13426, -6334, -1177, -4244, 12136, 2371, -580, 1303, 5980 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::XT4),
            0,
            0,
            [ 13426, -6334, -1177, -4244, 12136, 2371, -580, 1303, 5980 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::XS1),
            0,
            0,
            [ 13509, -6199, -1254, -4430, 12733, 1865, -331, 1441, 5022 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::XS10),
            0,
            0,
            [ 13426, -6334, -1177, -4244, 12136, 2371, -580, 1303, 5980 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::XF1),
            0,
            0,
            [ 13509, -6199, -1254, -4430, 12733, 1865, -331, 1441, 5022 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::XF10),
            0,
            0,
            [ 11673, -476, -1041, -3988, 12058, 2166, -771, 1417, 5569 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::S200EXR),
            512,
            0x3fff,
            [ 11401, -4498, -1312, -5088, 12751, 2613, -838, 1568, 5941 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::S100FS),
            512,
            0x3fff,
            [ 11521, -4355, -1065, -6524, 13768, 3059, -1466, 1984, 6045 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::GFX50S),
            0,
            0,
            [ 11756, -4754, -874, -3056, 11045, 2305, -381, 1457, 6006 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::GFX50S_II),
            0,
            0,
            [ 11756, -4754, -874, -3056, 11045, 2305, -381, 1457, 6006 ] ),
        // For now we assume it is the same sensor as the GFX50S
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::GFX50R),
            0,
            0,
            [ 11756, -4754, -874, -3056, 11045, 2305, -381, 1457, 6006 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::GFX100),
            0,
            0,
            [ 16212, -8423, -1583, -4336, 12583, 1937, -195, 726, 6199 ] ),
        BuiltinMatrix::new(
            TypeId(vendor::FUJIFILM, fujifilm::GFX100S),
            0,
            0,
            [ 16212, -8423, -1583, -4336, 12583, 1937, -195, 726, 6199 ] ),
    ];
}
