/* -*- mode:c++; tab-width:4; c-basic-offset:4 -*- */
/*
 * libopenraw - raffile.cpp
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
#include <sys/types.h>
#include <algorithm>
#include <cstdint>

#include <memory>

#include <libopenraw/cameraids.h>
#include <libopenraw/debug.h>
#include <libopenraw/metadata.h>

#include "rawdata.hpp"
#include "rawfile.hpp"
#include "metavalue.hpp"

#include "ifd.hpp"
#include "ifddir.hpp"
#include "ifdentry.hpp"
#include "ifdfilecontainer.hpp"
#include "rawfile_private.hpp"
#include "raffile.hpp"
#include "rafcontainer.hpp"
#include "rafmetacontainer.hpp"
#include "jfifcontainer.hpp"
#include "makernotedir.hpp"
#include "unpack.hpp"
#include "trace.hpp"
#include "io/streamclone.hpp"
#include "xtranspattern.hpp"

namespace OpenRaw {
namespace Internals {

#define OR_MAKE_FUJIFILM_TYPEID(camid)                                         \
    OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_FUJIFILM, camid)

/* taken from dcraw, by default */
static const BuiltinColourMatrix s_matrices[] = {
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_F550EXR),
      0,
      0,
      { 1369, -5358, -1474, -3369, 11600, 1998, -132, 1554, 4395 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_F700),
      0,
      0,
      { 10004, -3219, -1201, -7036, 15047, 2107, -1863, 2565, 7736 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_F810),
      0,
      0,
      { 11044, -3888, -1120, -7248, 15168, 2208, -1531, 2277, 8069 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_E900),
      0,
      0,
      { 9183, -2526, -1078, -7461, 15071, 2574, -2022, 2440, 8639 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_S2PRO),
      128,
      0,
      { 12492, -4690, -1402, -7033, 15423, 1647, -1507, 2111, 7697 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_S3PRO),
      0,
      0,
      { 11807, -4612, -1294, -8927, 16968, 1988, -2120, 2741, 8006 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_S5PRO),
      0,
      0,
      { 12300, -5110, -1304, -9117, 17143, 1998, -1947, 2448, 8100 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_S5000),
      0,
      0,
      { 8754, -2732, -1019, -7204, 15069, 2276, -1702, 2334, 6982 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_S5600),
      0,
      0,
      { 9636, -2804, -988, -7442, 15040, 2589, -1803, 2311, 8621 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_S9500),
      0,
      0,
      { 10491, -3423, -1145, -7385, 15027, 2538, -1809, 2275, 8692 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_S6500FD),
      0,
      0,
      { 12628, -4887, -1401, -6861, 14996, 1962, -2198, 2782, 7091 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_HS10),
      0,
      0xf68,
      { 12440, -3954, -1183, -1123, 9674, 1708, -83, 1614, 4086 } },
    // HS33EXR is an alias of this.
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_HS30EXR),
      0,
      0,
      { 1369, -5358, -1474, -3369, 11600, 1998, -132, 1554, 4395 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_X100),
      0,
      0,
      { 12161, -4457, -1069, -5034, 12874, 2400, -795, 1724, 6904 } },
    // From DNG Convert 7.4
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_X100S),
      0,
      0,
      { 10592, -4262, -1008, -3514, 11355, 2465, -870, 2025, 6386 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_X100T),
      0,
      0,
      { 10592, -4262, -1008, -3514, 11355, 2465, -870, 2025, 6386 } },
    // From DNG Converter 10.3
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_X100F),
      0,
      0,
      { 11434, -4948, -1210, -3746, 12042, 1903, -666, 1479, 5235 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_X100V),
      0,
      0,
      { 13426, -6334, -1177, -4244, 12136, 2371, -580, 1303, 5980 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_X10),
      0,
      0,
      { 13509, -6199, -1254, -4430, 12733, 1865, -331, 1441, 5022 } },
    // From DNG Convert 7.4
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_X20),
      0,
      0,
      { 11768, -4971, -1133, -4904, 12927, 2183, -480, 1723, 4605 } },
    // From DNG Convert 8.7-rc
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_X30),
      0,
      0,
      { 12328, -5256, -1144, -4469, 12927, 1675, -87, 1291, 4351 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_X70),
      0,
      0,
      { 10450, -4329, -878, -3217, 11105, 2421, -752, 1758, 6519 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XPRO1),
      0,
      0,
      { 10413, -3996, -993, -3721, 11640, 2361, -733, 1540, 6011 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XPRO2),
      0,
      0,
      { 11434, -4948, -1210, -3746, 12042, 1903, -666, 1479, 5235 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XPRO3),
      0,
      0,
      { 13426, -6334, -1177, -4244, 12136, 2371, -580, 1303, 5980 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XA1),
      0,
      0,
      { 11086, -4555, -839, -3512, 11310, 2517, -815, 1341, 5940 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XA2),
      0,
      0,
      { 10763, -4560, -917, -3346, 11311, 2322, -475, 1135, 5843 } },
    // From DNG Converter 10.3
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XA3),
      0,
      0,
      { 12407, -5222, -1086, -2971, 11116, 2120, -294, 1029, 5284 } },
    // From DNG Converter 10.3
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XA5),
      0,
      0,
      { 11673, -476, -1041, -3988, 12058, 2166, -771, 1417, 5569 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XA7),
      0,
      0,
      { 15055, -7391, -1274, -4062, 12071, 2238, -610, 1217, 6147 } },
    // From DNG Converter 10.3
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XQ1),
      0,
      0,
      { 9252, -2704, -1064, -5893, 14265, 1717, -1101, 2341, 4349 } },
    // From DNG Converter 10.3
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XQ2),
      0,
      0,
      { 9252, -2704, -1064, -5893, 14265, 1717, -1101, 2341, 4349 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XE1),
      0,
      0,
      { 10413, -3996, -993, -3721, 11640, 2361, -733, 1540, 6011 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XE2),
      0,
      0,
      { 8458, -2451, -855, -4597, 12447, 2407, -1475, 2482, 6526 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XE2S),
      0,
      0,
      { 11562, -5118, -961, -3022, 11007, 2311, -525, 1569, 6097 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XE3),
      0,
      0,
      { 11434, -4948, -1210, -3746, 12042, 1903, -666, 1479, 5235 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XE4),
      0,
      0,
      { 13426, -6334, -1177, -4244, 12136, 2371, -580, 1303, 5980 } },
    // From DNG Converter 10.3
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XH1),
      0,
      0,
      { 11434, -4948, -1210, -3746, 12042, 1903, -666, 1479, 5235 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XM1),
      0,
      0,
      { 10413, -3996, -993, -3721, 11640, 2361, -733, 1540, 6011 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XT1),
      0,
      0,
      { 8458, -2451, -855, -4597, 12447, 2407, -1475, 2482, 6526 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XT10),
      0,
      0,
      { 8458, -2451, -855, -4597, 12447, 2407, -1475, 2482, 6526 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XT100),
      0,
      0,
      { 11673, -476, -1041, -3988, 12058, 2166, -771, 1417, 5569 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XT2),
      0,
      0,
      { 11434, -4948, -1210, -3746, 12042, 1903, -666, 1479, 5235 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XT20),
      0,
      0,
      { 11434, -4948, -1210, -3746, 12042, 1903, -666, 1479, 5235 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XT200),
      0,
      0,
      { 15055, -7391, -1274, -4062, 12071, 2238, -610, 1217, 6147 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XT3),
      0,
      0,
      { 16393, -7740, -1436, -4238, 12131, 2371, -633, 1424, 6553 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XT30),
      0,
      0,
      { 13426, -6334, -1177, -4244, 12136, 2371, -580, 1303, 5980 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XT30_II),
      0,
      0,
      { 13426, -6334, -1177, -4244, 12136, 2371, -580, 1303, 5980 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XT4),
      0,
      0,
      { 13426, -6334, -1177, -4244, 12136, 2371, -580, 1303, 5980 } },
    // From DNG Converter 7.1-rc
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XS1),
      0,
      0,
      { 13509, -6199, -1254, -4430, 12733, 1865, -331, 1441, 5022 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XS10),
      0,
      0,
      { 13426, -6334, -1177, -4244, 12136, 2371, -580, 1303, 5980 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XF1),
      0,
      0,
      { 13509, -6199, -1254, -4430, 12733, 1865, -331, 1441, 5022 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XF10),
      0,
      0,
      { 11673, -476, -1041, -3988, 12058, 2166, -771, 1417, 5569 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_S200EXR),
      512,
      0x3fff,
      { 11401, -4498, -1312, -5088, 12751, 2613, -838, 1568, 5941 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_S100FS),
      512,
      0x3fff,
      { 11521, -4355, -1065, -6524, 13768, 3059, -1466, 1984, 6045 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_GFX50S),
      0,
      0,
      { 11756, -4754, -874, -3056, 11045, 2305, -381, 1457, 6006 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_GFX50S_II),
      0,
      0,
      { 11756, -4754, -874, -3056, 11045, 2305, -381, 1457, 6006 } },
    // For now we assume it is the same sensor as the GFX50S
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_GFX50R),
      0,
      0,
      { 11756, -4754, -874, -3056, 11045, 2305, -381, 1457, 6006 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_GFX100),
      0,
      0,
      { 16212, -8423, -1583, -4336, 12583, 1937, -195, 726, 6199 } },
    { OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_GFX100S),
      0,
      0,
      { 16212, -8423, -1583, -4336, 12583, 1937, -195, 726, 6199 } },

    { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } }
};

const RawFile::camera_ids_t RafFile::s_def[] = {
    { "GFX 50S", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_GFX50S) },
    { "GFX50S II", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_GFX50S_II) },
    { "GFX 50R", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_GFX50R) },
    { "GFX 100", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_GFX100) },
    { "GFX100S", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_GFX100S) },
    { "FinePix F550EXR", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_F550EXR) },
    { "FinePix F700  ", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_F700) },
    { "FinePix F810   ", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_F810) },
    { "FinePix E900   ", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_E900) },
    { "FinePixS2Pro", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_S2PRO) },
    { "FinePix S3Pro  ", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_S3PRO) },
    { "FinePix S5Pro  ", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_S5PRO) },
    { "FinePix S5000 ", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_S5000) },
    { "FinePix S5600  ", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_S5600) },
    { "FinePix S9500  ", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_S9500) },
    { "FinePix S6500fd", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_S6500FD) },
    { "FinePix HS10 HS11", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_HS10) },
    { "FinePix HS30EXR", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_HS30EXR) },
    { "FinePix HS33EXR", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_HS33EXR) },
    { "FinePix S100FS ", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_S100FS) },
    { "FinePix S200EXR", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_S200EXR) },
    { "FinePix X100", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_X100) },
    { "X10", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_X10) },
    { "X20", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_X20) },
    { "X30", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_X30) },
    { "X70", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_X70) },
    { "X-Pro1", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XPRO1) },
    { "X-Pro2", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XPRO2) },
    { "X-Pro3", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XPRO3) },
    { "X-S1", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XS1) },
    { "X-S10", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XS10) },
    { "X-A1", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XA1) },
    { "X-A2", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XA2) },
    { "X-A3", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XA3) },
    { "X-A5", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XA5) },
    { "X-A7", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XA7) },
    { "XQ1", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XQ1) },
    { "XQ2", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XQ2) },
    { "X-E1", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XE1) },
    { "X-E2", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XE2) },
    { "X-E2S", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XE2S) },
    { "X-E3", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XE3) },
    { "X-E4", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XE4) },
    { "X-M1", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XM1) },
    { "X-T1", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XT1) },
    { "X-T10", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XT10) },
    { "X-T100", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XT100) },
    { "X-T2", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XT2) },
    { "X-T20", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XT20) },
    { "X-T200", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XT200) },
    { "X-T3", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XT3) },
    { "X-T30", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XT30) },
    { "X-T30 II", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XT30_II) },
    { "X-T4", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XT4) },
    { "XF1", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XF1) },
    { "XF10", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XF10) },
    { "X100S", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_X100S) },
    { "X100T", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_X100T) },
    { "X100F", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_X100F) },
    { "X100V", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_X100V) },
    { "X-H1", OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XH1) },

    { nullptr, 0 }
};

RawFile *RafFile::factory(const IO::Stream::Ptr &s)
{
    return new RafFile(s);
}

RafFile::RafFile(const IO::Stream::Ptr &s)
    : RawFile(OR_RAWFILE_TYPE_RAF), m_io(s), m_container(new RafContainer(s))
{
    _setIdMap(s_def);
    _setMatrices(s_matrices);
}

RafFile::~RafFile()
{
    delete m_container;
}

IfdDir::Ref RafFile::_locateMainIfd()
{
    if (!m_mainIfd) {
        JfifContainer *jpegPreview = m_container->getJpegPreview();
        if (!jpegPreview) {
            return IfdDir::Ref();
        }
        m_mainIfd = jpegPreview->getIfdDirAt(0);
    }
    return m_mainIfd;
}

::or_error RafFile::_enumThumbnailSizes(std::vector<uint32_t> &list)
{
    or_error ret = OR_ERROR_NOT_FOUND;

    JfifContainer *jpegPreview = m_container->getJpegPreview();
    if (!jpegPreview) {
        return OR_ERROR_NOT_FOUND;
    }

    uint32_t x, y;
    if (jpegPreview->getDimensions(x, y)) {
        uint32_t size = std::max(x, y);

        list.push_back(size);
        _addThumbnail(size, ThumbDesc(x, y, OR_DATA_TYPE_JPEG,
                                      m_container->getJpegOffset(),
                                      m_container->getJpegLength()));
        ret = OR_ERROR_NONE;
    }
    IfdDir::Ref dir = jpegPreview->getIfdDirAt(1);
    if (!dir) {
        return ret;
    }

    // XXX check why this as it appear that if true there won't be
    // and thumbnail.
    auto result = dir->getIntegerValue(IFD::EXIF_TAG_IMAGE_WIDTH);
    if (result) {
        x = result.value();
        result = dir->getIntegerValue(IFD::EXIF_TAG_IMAGE_LENGTH);
        y = result.value_or(0);
    }

    if (result.empty()) {
        result =
            dir->getValue<uint32_t>(IFD::EXIF_TAG_JPEG_INTERCHANGE_FORMAT);
        if (result.empty()) {
            return ret;
        }
        uint32_t jpeg_offset = result.value() + jpegPreview->exifOffset();

        result = dir->getValue<uint32_t>(IFD::EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LENGTH);
        if (result.empty()) {
            return ret;
        }
        uint32_t jpeg_size = result.value();

        auto s = std::make_shared<IO::StreamClone>(jpegPreview->file(), jpeg_offset);
        auto thumb = std::make_unique<JfifContainer>(s, 0);

        if (thumb->getDimensions(x, y)) {
            uint32_t size = std::max(x, y);

            list.push_back(size);
            _addThumbnail(size,
                          ThumbDesc(x, y, OR_DATA_TYPE_JPEG,
                                    jpeg_offset + m_container->getJpegOffset(),
                                    jpeg_size));
            ret = OR_ERROR_NONE;
        }
    }

    return ret;
}

RawContainer *RafFile::getContainer() const
{
    return m_container;
}

bool
RafFile::isXTrans(RawFile::TypeId type_) const
{
    switch (type_) {
    case OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XPRO1):
    case OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XPRO2):
    case OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XPRO3):
    case OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XE1):
    case OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XE2):
    case OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XE2S):
    case OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XE3):
    case OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XE4):
    case OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XH1):
    case OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XM1):
    case OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XQ1):
    case OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XQ2):
    case OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XT1):
    case OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XT10):
    case OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XT2):
    case OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XT20):
    case OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XT3):
    case OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XT30):
    case OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XT30_II):
    case OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XT4):
    case OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_X100S):
    case OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_X100T):
    case OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_X100F):
    case OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_X100V):
    case OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_X20):
    case OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_X30):
    case OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_X70):
    case OR_MAKE_FUJIFILM_TYPEID(OR_TYPEID_FUJIFILM_XS10):
        return true;

    default:
        return false;
    }
}

::or_error RafFile::_getRawData(RawData &data, uint32_t /*options*/)
{
    ::or_error ret = OR_ERROR_NOT_FOUND;

    RafMetaContainer *meta = m_container->getMetaContainer();
    if (!meta) {
        LOGERR("RAF: Can't get meta container\n");
        return ret;
    }

    RafMetaValue::Ref value = meta->getValue(RAF_TAG_SENSOR_DIMENSION);
    if (!value) {
        // use this tag if the other is missing
        value = meta->getValue(RAF_TAG_IMG_HEIGHT_WIDTH);
    }
    uint32_t dims = value->get().getUInteger(0);
    uint16_t h = (dims & 0xFFFF0000) >> 16;
    uint16_t w = (dims & 0x0000FFFF);

    value = meta->getValue(RAF_TAG_RAW_INFO);
    uint32_t rawProps = value->get().getUInteger(0);
    // TODO re-enable if needed.
    // uint8_t layout = (rawProps & 0xFF000000) >> 24 >> 7; // MSBit in byte.
    uint8_t compressed = ((rawProps & 0xFF0000) >> 16) & 8; // 8 == compressed

    // printf("layout %x - compressed %x\n", layout, compressed);

    data.setDataType(OR_DATA_TYPE_RAW);
    data.setDimensions(w, h);
    if (isXTrans(typeId())) {
        data.setMosaicInfo(XTransPattern::xtransPattern());
    } else {
        data.setCfaPatternType(OR_CFA_PATTERN_GBRG);
    }
    // TODO actually read the 2048.
    // TODO make sure this work for the other file formats...
    size_t byte_size = m_container->getCfaLength() - 2048;
    size_t fetched = 0;
    off_t offset = m_container->getCfaOffset() + 2048;

    uint32_t finaldatalen = 2 * h * w;
    bool is_compressed = byte_size < finaldatalen; //(compressed == 8);
    uint32_t datalen = (is_compressed ? byte_size : finaldatalen);
    void *buf = data.allocData(finaldatalen);

    LOGDBG2("byte_size = %lu finaldatalen = %u compressed = %u", (LSIZE)byte_size,
            finaldatalen, compressed);

    ret = OR_ERROR_NONE;

    if (is_compressed) {
        Unpack unpack(w, IFD::COMPRESS_NONE);
        size_t blocksize = unpack.block_size();
        std::unique_ptr<uint8_t[]> block(new uint8_t[blocksize]);
        uint16_t *outdata = (uint16_t *)data.data();
        size_t outsize = finaldatalen;
        size_t got;
        do {
            got = m_container->fetchData(block.get(), offset, blocksize);
            fetched += got;
            offset += got;

            if (got) {
                size_t out;
                or_error err = unpack.unpack_be12to16(outdata, outsize,
                                                      block.get(), got, out);
                outdata += out / 2;
                outsize -= out;
                if (err != OR_ERROR_NONE) {
                    LOGDBG2("error is %d\n", static_cast<int>(err));
                    ret = err;
                    break;
                }
            }
        } while ((got != 0) && (fetched < datalen));
    } else {
        m_container->fetchData(buf, offset, datalen);
    }

    return ret;
}

MetaValue *RafFile::_getMetaValue(int32_t meta_index)
{
    if (META_INDEX_MASKOUT(meta_index) == META_NS_EXIF ||
        META_INDEX_MASKOUT(meta_index) == META_NS_TIFF) {

        JfifContainer *jpegPreview = m_container->getJpegPreview();
        if (!jpegPreview) {
            LOGERR("RAF: Can't get JPEG preview\n");
            return nullptr;
        }

        IfdDir::Ref dir = jpegPreview->mainIfd();
        IfdEntry::Ref e = dir->getEntry(META_NS_MASKOUT(meta_index));
        if (e) {
            return dir->makeMetaValue(*e);
        }
    }

    return nullptr;
}

void RafFile::_identifyId()
{
    _setTypeId(_typeIdFromModel("FUJIFILM", m_container->getModel()));
}
}
}
