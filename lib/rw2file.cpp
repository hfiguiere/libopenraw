/*
 * libopenraw - rw2file.cpp
 *
 * Copyright (C) 2011-2022 Hubert Figuière
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

#include <stddef.h>

#include <algorithm>
#include <cstdint>
#include <memory>

#include <libopenraw/cameraids.h>
#include <libopenraw/debug.h>

#include "rawdata.hpp"
#include "trace.hpp"
#include "io/streamclone.hpp"
#include "rw2file.hpp"
#include "rw2container.hpp"
#include "rawfile_private.hpp"

using namespace Debug;

namespace OpenRaw {

namespace Internals {

#define OR_MAKE_PANASONIC_TYPEID(camid) \
    OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PANASONIC,camid)
#define OR_MAKE_LEICA_TYPEID(camid) \
    OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_LEICA,camid)

/* taken from dcraw, by default */
static const BuiltinColourMatrix s_matrices[] = {
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_CM1),
      15,
      0,
      { 8770, -3194, -820, -2871, 11281, 1803, -513, 1552, 4434 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GF1),
      15,
      0xf92,
      { 7888,-1902,-1011,-8106,16085,2099,-2353,2866,7330 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GF2),
      15,
      0xfff,
      { 7888,-1902,-1011,-8106,16085,2099,-2353,2866,7330 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GF3),
      15,
      0xfff,
      { 9051,-2468,-1204,-5212,13276,2121,-1197,2510,6890 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GF5),
      15,
      0xfff,
      { 8228,-2945,-660,-3938,11792,2430,-1094,2278,5793 } },
    // Adobe DNG convert 7.4
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GF6),
      15,
      0xfff,
      { 8130,-2801,-946,-3520,11289,2552,-1314,2511,5791 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GF7),
      15,
      0,
      { 7610, -2780, -576, -4614, 12195, 2733, -1375, 2393, 6490 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GF10),
      15,
      0,
      { 7610, -2780, -576, -4614, 12195, 2733, -1375, 2393, 6490 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GX1),
      15,
      0,
      { 6763,-1919,-863,-3868,11515,2684,-1216,2387,5879 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GX7),
      15,
      0,
      { 7610,-2780,-576,-4614,12195,2733,-1375,2393,6490 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GX7MK2),
      15,
      0,
      { 7771, -3020, -629, -4029, 1195, 2345, -821, 1977, 6119 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GX8),
      15,
      0,
      { 7564, -2263, -606, -3148, 11239, 2177, -540, 1435, 4853 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GX80),
      15,
      0,
      { 7771, -3020, -629, -4029, 1195, 2345, -821, 1977, 6119 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GX800),
      15,
      0,
      { 7610, -2780, -576, -4614, 12195, 2733, -1375, 2393, 6490 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GX850),
      15,
      0,
      { 7610, -2780, -576, -4614, 12195, 2733, -1375, 2393, 6490 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GX9),
      15,
      0,
      { 7564, -2263, -606, -3148, 11239, 2177, -540, 1435, 4853 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ8),
      0,
      0xf7f,
      { 8986,-2755,-802,-6341,13575,3077,-1476,2144,6379 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ18),
      0,
      0,
      { 9932,-3060,-935,-5809,13331,2753,-1267,2155,5575 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ28),
      15,
      0xf96,
      { 10109,-3488,-993,-5412,12812,2916,-1305,2140,5543 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ200),
      143,
      0xfff,
      { 8112,-2563,-740,-3730,11784,2197,-941,2075,4933 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ2500),
      143,
      0xfff,
      { 7386, -2443, -743, -3437, 11864, 1757, -608, 1660, 4766 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ30),
      0,
      0xf94,
      { 10976,-4029,-1141,-7918,15491,2600,-1670,2071,8246 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ330),
      15,
      0,
      { 8378, -2798, -769, -3068, 11410, 1877, -538, 1792, 4623 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ35),
      15,
      0,
      { 9938, -2780, -890, -4604, 12393, 2480, -1117, 2304, 4620 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_DMC_FZ45),
      0,
      0,
      { 13639, -5535, -1371, -1698, 9633, 2430, 316, 1152, 4108 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ50),
      0,
      0,
      { 7906,-2709,-594,-6231,13351,3220,-1922,2631,6537 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ100),
      143,
      0xfff,
      { 16197,-6146,-1761,-2393,10765,1869,366,2238,5248 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_DMC_FZ1000),
      0,
      0,
      { 7830, -2696, -763, -3325, 11667, 1866, -641, 1712, 4824 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_DC_FZ1000M2),
      0,
      0,
      { 9803, -4185, -992, -4066, 12578, 1628, -838, 1824, 5288 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ150),
      0,
      0,
      { 11904, -4541, -1189, -2355, 10899, 1662, -296, 1586, 4289 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ80),
      0,
      0,
      { 11532, -4324, -1066, -2375, 10847, 1749, -564, 1699, 4351 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_G1),
      15,
      0xf94,
      { 8199,-2065,-1056,-8124,16156,2033,-2458,3022,7220 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_G2),
      15,
      0xf3c,
      { 10113,-3400,-1114,-4765,12683,2317,-377,1437,6710 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_G3),
      143,
      0xfff,
      { 6763,-1919,-863,-3868,11515,2684,-1216,2387,5879 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_G5),
      143,
      0xfff,
      { 7798,-2562,-740,-3879,11584,2613,-1055,2248,5434 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_G6),
      143,
      0xfff,
      { 8294, -2891, -651, -3869, 11590, 2595, -1183, 2267, 5352 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_G10),
      0,
      0,
      { 10113,-3400,-1114,-4765,12683,2317,-377,1437,6710 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_G7),
      0,
      0,
      { 7610, -2780, -576, -4614, 12195, 2733, -1375, 2393, 6490 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_G80),
      15,
      0,
      { 7610, -2780, -576, -4614, 12195, 2733, -1375, 2393, 6490 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_G8),
      15,
      0,
      { 7610, -2780, -576, -4614, 12195, 2733, -1375, 2393, 6490 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_G9),
      0,
      0,
      { 7685, -2375, -634, -3687, 11700, 2249, -748, 1546, 5111 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_DC_G95),
      0,
      0,
      { 9657, -3963, -748, -3361, 11378, 2258, -568, 1415, 5158 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_DC_G99),
      0,
      0,
      { 9657, -3963, -748, -3361, 11378, 2258, -568, 1415, 5158 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_DC_G100),
      0,
      0,
      { 8370, -2869, -710, -3389, 11372, 2298, -640, 1599, 4887 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GH1),
      15,
      0xf92,
      { 6299,-1466,-532,-6535,13852,2969,-2331,3112,5984 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GH2),
      15,
      0xf95,
      { 7780,-2410,-806,-3913,11724,2484,-1018,2390,5298 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GH3),
      144,
      0,
      { 6559,-1752,-491,-3672,11407,2586,-962,1875,5130 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GH4),
      15,
      0,
      { 7122,-2108,-512,-3155,11201,2231,-541,1423,5045 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GH5),
      15,
      0,
      { 7641, -2336, -605, -3218, 11299, 2187, -485, 1338, 5121 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GH5S),
      15,
      0,
      { 6929, -2355, -708, -4192, 12534, 1828, -1097, 1989, 5195 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GH5M2),
      15,
      0,
      { 9300, -3659, -755, -2981, 10988, 2287, -190, 1077, 5016 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GH6),
      15,
      0,
      { 7949, -3491, -710, -3435, 11681, 1977, -503, 1622, 5065 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GM1),
      15,
      0,
      { 6770,-1895,-744,-5232,13145,2303,-1664,2691,5703 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GM5),
      15,
      0,
      { 8238, -3244, -679, -3921, 11814, 2384, -836, 2022, 5852 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_LF1),
      0,
      0,
      { 9379, -3267, -816, -3227, 11560, 1881, -926, 1928, 5340 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_LX1),
      0,
      0,
      { 10704, -4187, -1230, -8314, 15952, 2501, -920, 945, 8927 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_LX2),
      0,
      0,
      { 8048,-2810,-623,-6450,13519,3272,-1700,2146,7049 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_LX3),
      15,
      0,
      { 8128,-2668,-655,-6134,13307,3161,-1782,2568,6083 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_LX5),
      143,
      0,
      { 10909,-4295,-948,-1333,9306,2399,22,1738,4582 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_LX7),
      143,
      0,
      { 10148,-3743,-991,-2837,11366,1659,-701,1893,4899 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_LX10), // and LX15 (alias)
      15,
      0,
      { 7790, -2736, -755, -3452, 11870, 1769, -628, 1647, 4898 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_LX100),
      143,
      0,
      { 8844,-3538,-768,-3709,11762,2200,-698,1792,5220 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_LX100M2),
      0,
      0,
      { 11577, -4230, -1106, -3967, 12211, 1957, -758, 1762, 5610 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_L1),
      0,
      0xf7f,
      { 8054,-1885,-1025,-8349,16367,2040,-2805,3542,7629 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_L10),
      15,
      0xf96,
      { 8025,-1942,-1050,-7920,15904,2100,-2456,3005,7039 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_TZ70),
      15,
      0,
      { 8802, -3135, -789, -3151, 11468, 1904, -550, 1745, 4810 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_ZS40),
      15,
      0,
      { 8607, -2822, -808, -3755, 11930, 2049, -820, 2060, 5224 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_ZS60),
      15,
      0,
      { 8550, -2908, -842, -3195, 11529, 1881, -338, 1603, 4631 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_ZS100),
      0,
      0,
      {  7790, -2736, -755, -3452, 11870, 1769, -628, 1647, 4898 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_ZS200),
      0,
      0,
      {  7790, -2736, -755, -3452, 11870, 1769, -628, 1647, 4898 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_DC_S1),
      0,
      0,
      { 9744, -3905, -779, -4899, 12807, 2324, -798, 1630, 5827 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_DC_S1R),
      0,
      0,
      { 11822, -5321, -1249, -5958, 15114, 766, -614, 1264, 7043 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_DC_S1H),
      0,
      0,
      { 9397, -3719, -805, -5425, 13326, 2309, -972, 1715, 6034 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_DC_S5),
      0,
      0,
      { 9744, -3905, -779, -4899, 12807, 2324, -798, 1630, 5827 } },
    { OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_DC_ZS80),
      0,
      0,
      { 12194, -5340, -1329, -3035, 11394, 1858, -50, 1418, 5219 } },

    { OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_DIGILUX2),
      0,
      0,
      { 11340, -4069, -1275, -7555, 15266, 2448, -2960, 3426, 7685 } },
    { OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_DIGILUX3),
      0,
      0,
      { 8054, -1886, -1025, -8348, 16367, 2040, -2805, 3542, 7630 } },
    { OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_DLUX_3),
      0,
      0,
      { 8048,-2810,-623,-6450,13519,3272,-1700,2146,7049 } },
    { OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_DLUX_TYP109),
      0,
      0,
      { 8844, -3538, -768, -3709, 11762, 2200, -698, 1792, 5220 } },
    { OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_DLUX_4),
      0,
      0,
      { 8128, -2668, -655, -6134, 13307, 3161, -1782, 2568, 6083 } },
    { OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_DLUX_5),
      143,
      0,
      { 10909, -4295, -948, -1333, 9306, 2399, 22, 1738, 4582 } },
    { OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_VLUX_1),
      0,
      0,
      { 7906,-2709,-594,-6231,13351,3220,-1922,2631,6537 } },
    { OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_VLUX_4),
      0,
      0,
      { 8112, -2563, -740, -3730, 11784, 2197, -941, 2075, 4933 } },
    { OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_VLUX_TYP114),
      0,
      0,
      { 7830, -2696, -763, -3325, 11667, 1866, -641, 1712, 4824 } },
    { OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_VLUX_5),
      0,
      0,
      { 9803, -4185, -992, -4066, 12578, 1628, -838, 1824, 5288 } },
    { OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_CLUX),
      15,
      0,
      { 7790, -2736, -755, -3452, 11870, 1769, -628, 1647, 4898 } },
    { OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_DLUX_6),
      0,
      0,
      { 10148, -3743, -991, -2837, 11366, 1659, -701, 1893, 4899 } },
    { OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_DLUX_7),
      0,
      0,
      { 11577, -4230, -1106, -3967, 12211, 1957, -758, 1762, 5610 } },
    { OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_C_TYP112),
      0,
      0,
      { 9379, -3267, -816, -3227, 11560, 1881, -926, 1928, 5340 } },

    { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } }
};

const IfdFile::camera_ids_t Rw2File::s_def[] = {
    { "DMC-CM1", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_CM1) },
    { "DMC-GF1", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GF1) },
    { "DMC-GF2", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GF2) },
    { "DMC-GF3", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GF3) },
    { "DMC-GF5", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GF5) },
    { "DMC-GF6", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GF6) },
    { "DMC-GF7", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GF7) },
    { "DC-GF10", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GF10) },
    { "DMC-GX1", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GX1) },
    { "DMC-GX7", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GX7) },
    { "DMC-GX7MK2", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GX7MK2) },
    { "DC-GX7MK3", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GX7MK3) },
    { "DMC-GX8", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GX8) },
    { "DMC-GX80", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GX80) },
    { "DMC-GX85", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GX85) },
    { "DC-GX800", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GX800) },
    { "DC-GX850", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GX850) },
    { "DC-GX880", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GX880) },
    { "DC-GX9", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GX9) },
    { "DMC-FZ8", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ8) },
    { "DMC-FZ1000", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_DMC_FZ1000) },
    { "DC-FZ10002", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_DC_FZ1000M2) },
    { "DC-FZ1000M2", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_DC_FZ1000M2) },
    { "DMC-FZ18", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ18) },
    { "DMC-FZ150", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ150) },
    { "DMC-FZ28", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ28) },
    { "DMC-FZ30", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ30) },
    { "DMC-FZ35", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ35) },
    { "DMC-FZ40", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_DMC_FZ40) },
    { "DMC-FZ45", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_DMC_FZ45) },
    // Not the same as above
    { "DC-FZ45", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_DC_FZ45) },
    { "DMC-FZ50", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ50) },
    { "DMC-FZ100", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ100) },
    { "DMC-FZ200", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ200) },
    { "DMC-FZ2500", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ2500) },
    // Alias to DMC-FZ2500
    { "DMC-FZ2000", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ2000) },
    { "DMC-FZ330", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ330) },
    { "DC-FZ80", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ80) },
    { "DC-FZ82", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_FZ82) },
    { "DMC-G1", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_G1) },
    { "DMC-G2", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_G2) },
    { "DMC-G3", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_G3) },
    { "DMC-G5", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_G5) },
    { "DMC-G6", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_G6) },
    { "DMC-G7", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_G7) },
    { "DMC-G70", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_G70) },
    { "DMC-G10", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_G10) },
    { "DMC-G80", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_G80) },
    { "DMC-G81", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_G81) },
    { "DC-G9", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_G9) },
    { "DC-G90", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_DC_G90) },
    { "DC-G91", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_DC_G91) },
    { "DC-G95", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_DC_G95) },
    { "DC-G99", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_DC_G99) },
    { "DC-G100", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_DC_G100) },
    { "DC-G110", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_DC_G110) },
    { "DMC-GH1", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GH1) },
    { "DMC-GH2", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GH2) },
    { "DMC-GH3", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GH3) },
    { "DMC-GH4", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GH4) },
    { "DC-GH5", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GH5) },
    { "DC-GH5S", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GH5S) },
    { "DC-GH5M2", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GH5M2) },
    { "DC-GH6", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GH6) },
    { "DMC-GM1", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GM1) },
    { "DMC-GM1S", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GM1S) },
    { "DMC-GM5", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_GM5) },
    { "DMC-LF1", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_LF1) },
    { "DMC-LX1", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_LX1) },
    { "DMC-LX2", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_LX2) },
    { "DMC-LX3", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_LX3) },
    { "DMC-LX5", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_LX5) },
    { "DMC-LX7", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_LX7) },
    { "DMC-LX10", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_LX10) },
    { "DMC-LX15", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_LX15) },
    { "DMC-LX100", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_LX100) },
    { "DC-LX100M2", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_LX100M2) },
    { "DMC-L1", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_L1) },
    { "DMC-L10", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_L10) },
    { "DC-S1", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_DC_S1) },
    { "DC-S1R", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_DC_S1R) },
    { "DC-S1H", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_DC_S1H) },
    { "DC-S5", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_DC_S5) },
    { "DMC-TZ70", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_TZ70) },
    { "DMC-ZS60", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_ZS60) },
    // Aliases to DMC-ZS60
    { "DMC-TZ80", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_TZ80) },
    { "DMC-ZS100", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_ZS100) },
    // Aliases to DMC-ZS100
    { "DMC-TX1", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_TX1) },
    { "DMC-TZ100", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_TZ100) },
    { "DMC-TZ110", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_TZ110) },
    { "DC-ZS200", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_ZS200) },
    // Aliases to DMC-ZS200
    { "DC-TZ202", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_TZ202) },
    { "DC-ZS80", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_DC_ZS80) },
    // Aliases to DC-ZS80
    { "DC-TZ95", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_DC_TZ95) },

    { "DMC-ZS40", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_ZS40) },
    // Alias to DMC-ZS40
    { "DMC-TZ60", OR_MAKE_PANASONIC_TYPEID(OR_TYPEID_PANASONIC_TZ60) },

    { "DIGILUX 2", OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_DIGILUX2) },
    { "DIGILUX 3", OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_DIGILUX3) },
    { "D-LUX 3", OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_DLUX_3) },
    { "D-LUX 4", OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_DLUX_4) },
    { "D-LUX 5", OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_DLUX_5) },
    { "D-LUX 6", OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_DLUX_6) },
    { "D-Lux 7", OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_DLUX_7) },
    { "V-LUX 1", OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_VLUX_1) },
    { "D-LUX (Typ 109)", OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_DLUX_TYP109) },
    { "V-LUX 4", OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_VLUX_4) },
    { "V-Lux 5", OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_VLUX_5) },
    { "V-LUX (Typ 114)", OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_VLUX_TYP114) },
    { "C-Lux", OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_CLUX) },
    { "C (Typ 112)", OR_MAKE_LEICA_TYPEID(OR_TYPEID_LEICA_C_TYP112) },

    { 0, 0 }
};

RawFile *Rw2File::factory(const IO::Stream::Ptr & s)
{
    return new Rw2File(s);
}

Rw2File::Rw2File(const IO::Stream::Ptr & s)
    : IfdFile(s, OR_RAWFILE_TYPE_RW2, false)
    , m_jfif_offset(0)
    , m_jfif_size(0)
{
    _setIdMap(s_def);
    _setMatrices(s_matrices);
    m_container = new Rw2Container(m_io, 0);
}

Rw2File::~Rw2File()
{
}

IfdDir::Ref  Rw2File::_locateCfaIfd()
{
    return mainIfd();
}

IfdDir::Ref  Rw2File::_locateMainIfd()
{
    auto ifd = m_container->setDirectory(0);
    if (ifd) {
        ifd->setTagTable(raw_panasonic_tag_names);
        ifd->setType(OR_IFD_MAIN);
    }
    return ifd;
}

IfdDir::Ref Rw2File::_locateExifIfd()
{
    auto _mainIfd = mainIfd();
    if (!_mainIfd) {
        LOGERR("IfdFile::_locateExifIfd() main IFD not found\n");
        return IfdDir::Ref();
    }
    uint32_t offset = 0;
    uint32_t size = 0;
    auto& jfif = getJpegContainer(_mainIfd, offset, size);
    if (!jfif) {
        LOGDBG1("IfdFile::_locateExifIfd() JPEG container not found\n");
        // the fall back is the regular IFD. Older RAW file use that.
        auto exifIfd = IfdFile::_locateExifIfd();
        if (exifIfd) {
            return exifIfd;
        }
        return IfdDir::Ref();
    }
    return jfif->exifIfd();
}

const std::unique_ptr<JfifContainer>&
Rw2File::getJpegContainer(const IfdDir::Ref& dir, uint32_t& offset, uint32_t& size)
{
    if (!m_jfif) {
        m_jfif_offset = _getJpegThumbnailOffset(dir, m_jfif_size);
        if (m_jfif_size == 0) {
            return m_jfif; // is it nullptr?
        }
        LOGDBG1("Jpeg offset: %u\n", m_jfif_offset);

        IO::Stream::Ptr s(new IO::StreamClone(m_io, m_jfif_offset));
        m_jfif = std::make_unique<JfifContainer>(s, 0);
    }
    offset = m_jfif_offset;
    size = m_jfif_size;
    return m_jfif;
}

::or_error Rw2File::_locateThumbnail(const IfdDir::Ref & dir,
                                     std::vector<uint32_t> &list)
{
    uint32_t offset = 0;
    uint32_t size = 0;

    auto& jfif = getJpegContainer(dir, offset, size);
    if (!jfif) {
        return OR_ERROR_NOT_FOUND;
    }

    auto jdir = jfif->getIfdDirAt(1);
    if (jdir) {
        auto byte_count =
            jdir->getValue<uint32_t>(IFD::EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LENGTH).value_or(0);
        auto result = jdir->getValue<uint32_t>(IFD::EXIF_TAG_JPEG_INTERCHANGE_FORMAT);
        LOGDBG1("byte count %u\n", byte_count);
        LOGASSERT(result.has_value());
        if (result.has_value()) {
            auto toffset = result.value();
            LOGDBG1("toffset %u\n", toffset);
            uint32_t tnail_offset = offset + toffset + jfif->exifOffset();
            auto s = std::make_shared<IO::StreamClone>(m_io, tnail_offset);
            auto tnail = std::make_unique<JfifContainer>(s, 0);

            uint32_t x = 0;
            uint32_t y = 0;
            if (tnail->getDimensions(x, y)) {
                uint32_t dim = std::max(x, y);
                _addThumbnail(dim, ThumbDesc(x, y, OR_DATA_TYPE_JPEG, tnail_offset, byte_count));
                list.push_back(dim);
            }
        }

    }

    uint32_t x = 0;
    uint32_t y = 0;
    if (jfif->getDimensions(x,y)) {
        LOGDBG1("JPEG dimensions x=%u y=%u\n", x, y);
        uint32_t dim = std::max(x, y);
        _addThumbnail(dim, ThumbDesc(x, y, OR_DATA_TYPE_JPEG, offset, size));
        list.push_back(dim);
    }

    return OR_ERROR_NONE;
}

uint32_t Rw2File::_getJpegThumbnailOffset(const IfdDir::Ref& dir, uint32_t & len)
{
    IfdEntry::Ref e = dir->getEntry(IFD::RW2_TAG_JPEG_FROM_RAW);
    if (!e) {
        len = 0;
        LOGDBG1("JpegFromRaw not found\n");
        return 0;
    }
    uint32_t offset = e->offset();
    len = e->count();
    return offset;
}


::or_error Rw2File::_getRawData(RawData & data, uint32_t /*options*/)
{
    ::or_error ret = OR_ERROR_NONE;
    const IfdDir::Ref & _cfaIfd = cfaIfd();
    if (!_cfaIfd) {
        LOGDBG1("cfa IFD not found\n");
        return OR_ERROR_NOT_FOUND;
    }

    LOGDBG1("_getRawData()\n");
    uint32_t offset = 0;
    uint32_t byte_length = 0;
    // RW2 file
    auto result = _cfaIfd->getIntegerValue(IFD::RW2_TAG_STRIP_OFFSETS);
    if (result) {
        offset = result.value();
        byte_length = m_container->file()->filesize() - offset;
    } else {
        // RAW file alternative.
        result = _cfaIfd->getIntegerValue(IFD::EXIF_TAG_STRIP_OFFSETS);
        if (result.empty()) {
            LOGDBG1("offset not found\n");
            return OR_ERROR_NOT_FOUND;
        }
        offset = result.value();
        result = _cfaIfd->getIntegerValue(IFD::EXIF_TAG_STRIP_BYTE_COUNTS);
        if (result.empty()) {
            LOGDBG1("byte len not found\n");
            return OR_ERROR_NOT_FOUND;
        }
        byte_length = result.value();
    }

    result = _cfaIfd->getIntegerValue(IFD::RW2_TAG_SENSOR_WIDTH);
    if (result.empty()) {
        LOGDBG1("X not found\n");
        return OR_ERROR_NOT_FOUND;
    }
    uint32_t x = result.value();

    result = _cfaIfd->getIntegerValue(IFD::RW2_TAG_SENSOR_HEIGHT);
    if (result.empty()) {
        LOGDBG1("Y not found\n");
        return OR_ERROR_NOT_FOUND;
    }
    uint32_t y = result.value();

    // this is were things are complicated. The real size of the raw data
    // is whatever is read (if compressed)
    void *p = data.allocData(byte_length);
    size_t real_size = m_container->fetchData(p, offset,
                                              byte_length);
    if (real_size < byte_length) {
        LOGDBG1("adjusting size");
        data.adjustSize(real_size);
    }
    bool packed = false;
    if ((x * y * 2) == real_size) {
        data.setDataType(OR_DATA_TYPE_RAW);
    } else if ((x * y * 3 / 2) == real_size) {
        data.setDataType(OR_DATA_TYPE_RAW);
        packed = true;
    } else {
        data.setDataType(OR_DATA_TYPE_COMPRESSED_RAW);
        auto v = _cfaIfd->getValue<uint16_t>(RW2_TAG_IMAGE_COMPRESSION);
        if (v) {
            data.setCompression(v.value());
        }
    }
    // It seems that they are all RGB
    auto pattern = _cfaIfd->getValue<uint16_t>(RW2_TAG_IMAGE_CFAPATTERN);
    if (!pattern) {
        LOGERR("Pattern not found.\n");
    } else {
        auto v = pattern.value();
        switch (v) {
        case 1:
            data.setCfaPatternType(OR_CFA_PATTERN_RGGB);
            break;
        case 2:
            data.setCfaPatternType(OR_CFA_PATTERN_GRBG);
            break;
        case 3:
            data.setCfaPatternType(OR_CFA_PATTERN_GBRG);
            break;
        case 4:
            data.setCfaPatternType(OR_CFA_PATTERN_BGGR);
            break;
        default:
            LOGERR("Pattern is %u (UNKNOWN).\n", v);
        }
    }

    data.setDimensions(x, y);
    auto bpc = _cfaIfd->getValue<uint16_t>(RW2_TAG_IMAGE_BITSPERSAMPLE).value_or(0);
    if (bpc != 0) {
        data.setBpc(bpc);
    }

    LOGDBG1("In size is %ux%u\n", data.width(), data.height());
    // get the sensor info
    // XXX what if it is not found?
    x = _cfaIfd->getValue<uint16_t>(IFD::RW2_TAG_SENSOR_LEFTBORDER).value_or(0);
    y = _cfaIfd->getValue<uint16_t>(IFD::RW2_TAG_SENSOR_TOPBORDER).value_or(0);
    auto v = _cfaIfd->getValue<uint16_t>(IFD::RW2_TAG_SENSOR_BOTTOMBORDER);
    int32_t h = v.value_or(0);
    h -= y;
    if (h < 0) {
        h = 0;
    }

    v = _cfaIfd->getValue<uint16_t>(IFD::RW2_TAG_SENSOR_RIGHTBORDER);
    int32_t w = v.value_or(0);
    w -= x;
    if (w < 0) {
        w = 0;
    }

    data.setActiveArea(x, y, w, h);

    return ret;
}

}
}
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  tab-width:4
  c-basic-offset:4
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
