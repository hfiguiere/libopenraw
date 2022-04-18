/* -*- mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil; -*- */
/*
 * libopenraw - cr2file.cpp
 *
 * Copyright (C) 2006-2022 Hubert Figuière
 * Copyright (C) 2008 Novell, Inc.
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
#include <cstdint>
#include <vector>
#include <limits>
#include <memory>

#include <libopenraw/cameraids.h>
#include <libopenraw/consts.h>
#include <libopenraw/debug.h>

#include "rawdata.hpp"
#include "rawfile.hpp"
#include "mosaicinfo.hpp"
#include "trace.hpp"
#include "io/memstream.hpp"
#include "ifdfilecontainer.hpp"
#include "ifdentry.hpp"
#include "makernotedir.hpp"
#include "cr2file.hpp"
#include "jfifcontainer.hpp"
#include "ljpegdecompressor.hpp"
#include "rawfile_private.hpp"
#include "canon.hpp"

using namespace Debug;

namespace OpenRaw {

namespace Internals {

/** @addtogroup canon
 * @{
 */

#define OR_MAKE_CANON_TYPEID(camid)                                            \
    OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_CANON, camid)

/* taken from dcraw, by default */
/* all relative to the D65 calibration illuminant */
static const BuiltinColourMatrix s_matrices[] = {
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1DMKII),
      0,
      0xe80,
      { 6264, -582, -724, -8312, 15948, 2504, -1744, 1919, 8664 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1DMKIIN),
      0,
      0xe80,
      { 6240, -466, -822, -8180, 15825, 2500, -1801, 1938, 8042 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1DMKIII),
      0,
      0xe80,
      { 6291, -540, -976, -8350, 16145, 2311, -1714, 1858, 7326 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1DMKIV),
      0,
      0x3bb0,
      { 6014, -220, -795, -4109, 12014, 2361, -561, 1824, 5787 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1DSMKII),
      0,
      0xe80,
      { 6517, -602, -867, -8180, 15926, 2378, -1618, 1771, 7633 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1DSMKIII),
      0,
      0x3bb0,
      { 5859, -211, -930, -8255, 16017, 2353, -1732, 1887, 7448 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1DX),
      0,
      0x3c4e,
      { 6847, -614, -1014, -4669, 12737, 2139, -1197, 2488, 6846 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1DXMKII),
      0,
      0x3c4e,
      { 7596, -978, -967, -4808, 12571, 2503, -1398, 2567, 5752 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_20D),
      0,
      0xfff,
      { 6599, -537, -891, -8071, 15783, 2424, -1983, 2234, 7462 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_20DA),
      0,
      0,
      { 14155, -5065, -1382, -6550, 14633, 2039, -1623, 1824, 6561 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_30D),
      0,
      0,
      { 6257, -303, -1000, -7880, 15621, 2396, -1714, 1904, 7046 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_350D),
      0,
      0xfff,
      { 6018, -617, -965, -8645, 15881, 2975, -1530, 1719, 7642 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_40D),
      0,
      0x3f60,
      { 6071, -747, -856, -7653, 15365, 2441, -2025, 2553, 7315 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_400D),
      0,
      0xe8e,
      { 7054, -1501, -990, -8156, 15544, 2812, -1278, 1414, 7796 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_450D),
      0,
      0x390d,
      { 5784, -262, -821, -7539, 15064, 2672, -1982, 2681, 7427 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_50D),
      0,
      0x3d93,
      { 4920, 616, -593, -6493, 13964, 2784, -1774, 3178, 7005 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_500D),
      0,
      0x3479,
      { 4763, 712, -646, -6821, 14399, 2640, -1921, 3276, 6561 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_550D),
      0,
      0x3dd7,
      { 6941, -1164, -857, -3825, 11597, 2534, -416, 1540, 6039 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_600D),
      0,
      0x3510,
      { 6461, -907, -882, -4300, 12184, 2378, -819, 1944, 5931 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_60D),
      0,
      0x2ff7,
      { 6719, -994, -925, -4408, 12426, 2211, -887, 2129, 6051 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_650D),
      0,
      0x354d,
      { 6602, -841, -939, -4472, 12458, 2247, -975, 2039, 6148 } },
    // from DNG Convert 7.4
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_700D),
      0,
      0x3c00,
      { 6602, -841, -939, -4472, 12458, 2247, -975, 2039, 6148 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_80D),
      0,
      0,
      { 7457,-671,-937,-4849,12495,2643,-1213,2354,5492 } },
    // From DNG Converter 10.3
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_800D),
      0,
      0,
      { 6970, -512, -968, -4425, 12161, 2553, -739, 1982, 5601 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_100D),
      0,
      0x350f,
      { 6602, -841, -939, -4472, 12458, 2247, -975, 2039, 6148 } },
    // From DNG Converter 10.3
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_200D),
      0,
      0x350f,
      { 7377, -742, -998, -4235, 11981, 2549, -673, 1918, 5538 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1000D),
      0,
      0xe43,
      { 6771, -1139, -977, -7818, 15123, 2928, -1244, 1437, 7533 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1100D),
      0,
      0x3510,
      { 6444, -904, -893, -4563, 12308, 2535, -903, 2016, 6728 } },
    // from DNG Convert 7.4
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_100D),
      0,
      0x3806,
      { 6602, -841, -939, -4472, 12458, 2247, -975, 2039, 6148 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1200D), // Rebel T5
      0,
      0x37c2,
      { 6461, -907, -882, -4300, 12184, 2378, -819, 1944, 5931 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1300D), // Rebel T6
      0,
      0x3510,
      { 6939, -1016, -866, -4428, 12473, 2177, -1175, 2178, 6162 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_2000D), // Rebel T7
      0,
      0,
      { 8532, -701, -1167, -4095, 11879, 2508, -797, 2424, 7010 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_3000D), // 4000D, Rebel T100
      0,
      0,
      { 6939, -1016, -866, -4428, 12473, 2177, -1175, 2178, 6162 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_5D),
      0,
      0xe6c,
      { 6347, -479, -972, -8297, 15954, 2480, -1968, 2131, 7649 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_5DMKII),
      0,
      0x3cf0,
      { 4716, 603, -830, -7798, 15474, 2480, -1496, 1937, 6651 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_5DMKIII),
      0,
      0,
      { 6722, -635, -963, -4287, 12460, 2028, -908, 2162, 5668 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_5DMKIV),
      0,
      0x3bb0,
      {  6014, -220, -795, -4109, 12014, 2361, -561, 1824, 5787 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_5DS),
      0,
      0xe6c,
      { 6250, -711, -808, -5153, 12794, 2636, -1249, 2198, 5610 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_5DS_R),
      0,
      0xe6c,
      { 6250, -711, -808, -5153, 12794, 2636, -1249, 2198, 5610 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_6D),
      0,
      0x3c82,
      { 7034, -804, -1014, -4420, 12564, 2058, -851, 1994, 5758 } },
    // From DNG Converter 10.3
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_6DMKII),
      0,
      0,
      { 6875, -970, -932, -4691, 12459, 2501, -874, 1953, 5809 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_7D),
      0,
      0x3510,
      { 6844, -996, -856, -3876, 11761, 2396, -593, 1772, 6198 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_7DMKII),
      0,
      0x3510,
      { 7268, -1082, -969, -4186, 11839, 2663, -825, 2029, 5839 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_70D),
      0,
      0x3bc7,
      { 7034, -804, -1014, -4420, 12564, 2058, -851, 1994, 5758 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_750D),
      0,
      0x368e,
      { 6362, -823, -847, -4426, 12109, 2616, -743, 1857, 5635 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_760D),
      0,
      0x350f,
      { 6362, -823, -847, -4426, 12109, 2616, -743, 1857, 5635 } },
    // Fron DNG Converter 10.3
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_77D),
      0,
      0,
      { 7377, -742, -998, -4235, 11981, 2549, -673, 1918, 5538 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M),
      0,
      0,
      { 6602, -841, -939, -4472, 12458, 2247, -975, 2039, 6148 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M2),
      0,
      0,
      { 6400, -480, -888, -5294, 13416, 2047, -1296, 2203, 6137 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M3),
      0,
      0,
      { 6362, -823, -847, -4426, 12109, 2616, -743, 1857, 5635 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M5),
      0,
      0,
      { 8532, -701, -1167, -4095, 11879, 2508, -797, 2424, 7010 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M6),
      0,
      0,
      { 8532, -701, -1167, -4095, 11879, 2508, -797, 2424, 7010 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M10),
      0,
      0,
      { 6400, -480, -888, -5294, 13416, 2047, -1296, 2203, 6137 } },
    // From DNG Converter 10.3
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M100),
      0,
      0,
      { 8532, -701, -1167, -4095, 11879, 2508, -797, 2424, 7010 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G9),
      0,
      0,
      { 7368, -2141, -598, -5621, 13254, 2625, -1418, 1696, 5743 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G10),
      0,
      0,
      { 11093, -3906, -1028, -5047, 12492, 2879, -1003, 1750, 5561 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G11),
      0,
      0,
      { 12177, -4817, -1069, -1612, 9864, 2049, -98, 850, 4471 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G12),
      0,
      0,
      { 13244, -5501, -1248, -1508, 9858, 1935, -270, 1083, 4366 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G15),
      0,
      0,
      { 7474, -2301, -567, -4056, 11456, 2975, -222, 716, 4181 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G16),
      0,
      0,
      { 8020, -2687, -682, -3704, 11879, 2052, -965, 1921, 5556 } },
    // From DNG Converter 7.1-rc
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G1X),
      0,
      0,
      { 7378, -1255, -1043, -4088, 12251, 2048, -876, 1946, 5805 } },
    // From DNG Converter 8.7-rc
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G1XMKII),
      0,
      0,
      { 7378, -1255, -1043, -4088, 12251, 2048, -876, 1946, 5805 } },
    // From DNG Converter 10.3
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G1XMKIII),
      0,
      0,
      { 8532, -701, -1167, -4095, 11879, 2508, -797, 2424, 7010 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G3X),
      0,
      0,
      { 9701, -3857, -921, -3149, 11537, 1817, -786, 1817, 5147 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G5X),
      0,
      0,
      { 9602, -3823, -937, -2984, 11495, 1675, -407, 1415, 5049 } },
    // From DNG Converter 10.3
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G7X),
      0,
      0,
      { 9602, -3823, -937, -2984, 11495, 1675, -407, 1415, 5049 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G7XMKII),
      0,
      0,
      { 9602, -3823, -937, -2984, 11495, 1675, -407, 1415, 5049 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G9X),
      0,
      0,
      { 9602, -3823, -937, -2984, 11495, 1675, -407, 1415, 5049 } },
    // From DNG Converter 10.3
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G9XMKII),
      0,
      0,
      { 10056, -4131, -944, -2576, 11143, 1625, -238, 1294, 5179} },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_S90),
      0,
      0,
      { 12374, -5016, -1049, -1677, 9902, 2078, -83, 852, 4683 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_S95),
      0,
      0,
      { 13440, -5896, -1279, -1236, 9598, 1931, -180, 1001, 4651 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_S100),
      0,
      0,
      { 7968, -2565, -636, -2873, 10697, 2513, 180, 667, 4211 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_S100V),
      0,
      0,
      { 7968, -2565, -636, -2873, 10697, 2513, 180, 667, 4211 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_S110),
      0,
      0,
      { 8039, -2643, -654, -3783, 11230, 2930, -206, 690, 4194 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_S120),
      0,
      0,
      { 6961, -1685, -695, -4625, 12945, 1836, -1114, 2152, 5518 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_SX50_HS),
      0,
      0,
      { 12432, -4753, -1247, -2110, 10691, 1629, -412, 1623, 4926 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_SX60_HS),
      0,
      0,
      { 13161, -5451, -1344, -1989, 10654, 1531, -47, 1271, 4955 } },
    { OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_SX1_IS),
      0,
      0,
      { 6578, -259, -502, -5974, 13030, 3309, -308, 1058, 4970 } },

    /*
    { "Canon EOS-1DS", 0, 0xe20,
        { 4374,3631,-1743,-7520,15212,2472,-2892,3632,8161 } },
    { "Canon EOS-1D", 0, 0xe20,
        { 6806,-179,-1020,-8097,16415,1687,-3267,4236,7690 } },
     */
    { 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0 } }
};

const IfdFile::camera_ids_t Cr2File::s_def[] = {
    { "Canon EOS-1D Mark II", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1DMKII) },
    { "Canon EOS-1D Mark III", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1DMKIII) },
    { "Canon EOS-1D Mark IV", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1DMKIV) },
    { "Canon EOS-1Ds Mark II", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1DSMKII) },
    { "Canon EOS-1Ds Mark III",
      OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1DSMKIII) },
    { "Canon EOS-1D X", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1DX) },
    { "Canon EOS-1D X Mark II", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1DXMKII) },
    { "Canon EOS 20D", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_20D) },
    { "Canon EOS 20Da", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_20DA) },
    { "Canon EOS 30D", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_30D) },
    { "Canon EOS 350D DIGITAL", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_350D) },
    { "Canon EOS DIGITAL REBEL XT",
      OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_REBEL_XT) },
    { "Canon EOS Kiss Digital N", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_KISS_DIGITAL_N) },
    { "Canon EOS 40D", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_40D) },
    { "Canon EOS 400D DIGITAL", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_400D) },
    { "Canon EOS 450D", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_450D) },
    { "Canon EOS DIGITAL REBEL XSi", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_REBEL_XSI) },
    { "Canon EOS 50D", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_50D) },
    { "Canon EOS 500D", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_500D) },
    { "Canon EOS 550D", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_550D) },
    { "Canon EOS REBEL T2i", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_REBEL_T2I) },
    { "Canon EOS 600D", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_600D) },
    { "Canon EOS REBEL T3i", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_REBEL_T3I) },
    { "Canon EOS 60D", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_60D) },
    { "Canon EOS 650D", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_650D) },
    { "Canon EOS REBEL T4i", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_REBEL_T4I) },
    { "Canon EOS 70D", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_70D) },
    { "Canon EOS 700D", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_700D) },
    { "Canon EOS 750D", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_750D) },
    { "Canon EOS 760D", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_760D) },
    { "Canon EOS 80D", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_80D) },
    { "Canon EOS 800D", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_800D) },
    { "Canon EOS REBEL T1i", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_REBEL_T1I) },
    { "Canon EOS Rebel T5", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_REBEL_T5) },
    { "Canon EOS REBEL T5i", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_REBEL_T5I) },
    { "Canon EOS Rebel T6i", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_REBEL_T6I) },
    { "Canon EOS Rebel T6s", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_REBEL_T6S) },
    { "Canon EOS Rebel T6", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_REBEL_T6) },
    { "Canon EOS Rebel T7i", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_REBEL_T7I) },
    { "Canon EOS Rebel T7", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_REBEL_T7) },
    { "Canon EOS 1000D", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1000D) },
    { "Canon EOS 2000D", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_2000D) },
    { "Canon EOS DIGITAL REBEL XS",
      OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_REBEL_XS) },
    { "Canon EOS 1100D", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1100D) },
    { "Canon EOS 1200D", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1200D) },
    { "Canon EOS 1300D", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1300D) },
    { "Canon EOS REBEL T3", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_REBEL_T3) },
    { "Canon EOS 100D", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_100D) },
    { "Canon EOS REBEL SL1", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_REBEL_SL1) },
    { "Canon EOS 200D", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_200D) },
    { "Canon EOS Rebel SL2", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_REBEL_SL2) },
    { "Canon EOS 5D", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_5D) },
    { "Canon EOS 5D Mark II", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_5DMKII) },
    { "Canon EOS 5D Mark III", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_5DMKIII) },
    { "Canon EOS 5D Mark IV", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_5DMKIV) },
    { "Canon EOS 5DS", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_5DS) },
    { "Canon EOS 5DS R", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_5DS_R) },
    { "Canon EOS 6D", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_6D) },
    { "Canon EOS 6D Mark II", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_6DMKII) },
    { "Canon EOS 7D", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_7D) },
    { "Canon EOS 7D Mark II", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_7DMKII) },
    { "Canon EOS 77D", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_77D) },
    { "Canon EOS Kiss X3", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_KISS_X3) },
    { "Canon EOS M", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M) },
    { "Canon EOS M10", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M10) },
    { "Canon EOS M100", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M100) },
    { "Canon EOS M2", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M2) },
    { "Canon EOS M3", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M3) },
    { "Canon EOS M5", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M5) },
    { "Canon EOS M6", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M6) },
    { "Canon PowerShot G9", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G9) },
    { "Canon PowerShot G10", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G10) },
    { "Canon PowerShot G11", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G11) },
    { "Canon PowerShot G12", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G12) },
    { "Canon PowerShot G15", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G15) },
    { "Canon PowerShot G16", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G16) },
    { "Canon PowerShot G1 X", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G1X) },
    { "Canon PowerShot G1 X Mark II",
      OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G1XMKII) },
    { "Canon PowerShot G1 X Mark III",
      OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G1XMKIII) },
    { "Canon PowerShot G3 X", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G3X) },
    { "Canon PowerShot G5 X", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G5X) },
    { "Canon PowerShot G7 X", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G7X) },
    { "Canon PowerShot G7 X Mark II", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G7XMKII) },
    { "Canon PowerShot G9 X", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G9X) },
    { "Canon PowerShot G9 X Mark II", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G9XMKII) },
    { "Canon PowerShot S90", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_S90) },
    { "Canon PowerShot S95", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_S95) },
    { "Canon PowerShot S100", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_S100) },
    { "Canon PowerShot S100V", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_S100V) },
    { "Canon PowerShot S110", OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_S110) },
    { "Canon PowerShot SX50 HS",
      OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_SX50_HS) },
    { 0, 0 }
};

RawFile *Cr2File::factory(const IO::Stream::Ptr &s)
{
    return new Cr2File(s);
}

Cr2File::Cr2File(const IO::Stream::Ptr &s) : IfdFile(s, OR_RAWFILE_TYPE_CR2)
{
    _setIdMap(s_def);
    _setMatrices(s_matrices);
}

Cr2File::~Cr2File()
{
}

IfdDir::Ref Cr2File::_locateCfaIfd()
{
    if (!isCr2()) {
        return makerNoteIfd();
    }
    auto ifd = m_container->setDirectory(3);
    if (ifd) {
        ifd->setType(OR_IFD_RAW);
    }
    return ifd;
}

IfdDir::Ref Cr2File::_locateMainIfd()
{
    auto ifd = m_container->setDirectory(0);
    if (ifd) {
        ifd->setType(OR_IFD_MAIN);
    }
    return ifd;
}

::or_error Cr2File::_locateThumbnail(const IfdDir::Ref & dir,
                                     std::vector<uint32_t> &list)
{
    if (isCr2()) {
        return IfdFile::_locateThumbnail(dir, list);
    }
    ::or_error ret = OR_ERROR_NOT_FOUND;
    ::or_data_type _type = OR_DATA_TYPE_NONE;
    uint32_t x = dir->getIntegerValue(IFD::EXIF_TAG_IMAGE_WIDTH).value_or(0);
    uint32_t y = dir->getIntegerValue(IFD::EXIF_TAG_IMAGE_LENGTH).value_or(0);
    uint32_t offset = 0;
    uint32_t byte_count = dir->getValue<uint32_t>(IFD::EXIF_TAG_STRIP_BYTE_COUNTS).value_or(0);

    auto result = dir->getValue<uint32_t>(IFD::EXIF_TAG_STRIP_OFFSETS);
    bool got_it = result.has_value();
    if (got_it) {
        offset = result.value();
    }

    if (x != 0 && y != 0) {
        // See bug 72270 - some CR2 have 16 bpc RGB thumbnails.
        // by default it is RGB8. Unless stated otherwise.
        bool isRGB8 = true;
        IfdEntry::Ref entry = dir->getEntry(IFD::EXIF_TAG_BITS_PER_SAMPLE);
        auto result2 = dir->getEntryArrayValue<uint16_t>(*entry);
        if (result2) {
          std::vector<uint16_t> arr = result2.value();
          for(auto bpc : arr) {
            isRGB8 = (bpc == 8);
            if (!isRGB8) {
              LOGDBG1("bpc != 8, not RGB8 %u\n", bpc);
              break;
            }
          }
        } else {
          LOGDBG1("Error getting BPS\n");
        }
        if (isRGB8) {
          _type = OR_DATA_TYPE_PIXMAP_8RGB;
        }
      }

    if(_type != OR_DATA_TYPE_NONE) {
      uint32_t dim = std::max(x, y);
      offset += dir->container().offset();
      _addThumbnail(dim, ThumbDesc(x, y, _type,
                                   offset, byte_count));
      list.push_back(dim);
      ret = OR_ERROR_NONE;
    }
    return ret;
}

::or_error Cr2File::getRawDataTif(RawData &data, uint32_t options)
{
    const IfdDir::Ref &_cfaIfd = cfaIfd();
    if (!_cfaIfd) {
        LOGDBG1("cfa IFD not found\n");
        return OR_ERROR_NOT_FOUND;
    }

    auto result = _cfaIfd->getValue<uint32_t>(IFD::MNOTE_CANON_RAW_DATA_OFFSET);
    if (result.empty()) {
        LOGDBG1("offset not found\n");
        return OR_ERROR_NOT_FOUND;
    }
    uint32_t offset = result.value();
    uint32_t byte_length = 0;

    result = _cfaIfd->getValue<uint32_t>(IFD::MNOTE_CANON_RAW_DATA_LENGTH);
    if (!result.empty()) {
        byte_length = result.value();
    } else {
        LOGDBG1("byte len not found\n");
        auto len = std::min<off_t>(
            m_container->size(), std::numeric_limits<uint32_t>::max());
        byte_length = len - offset;
    }

    std::vector<uint16_t> slices;

    getRawBytes(data, offset, byte_length, 0, 0, slices, options);
    return OR_ERROR_NONE;
}

void Cr2File::getRawBytes(RawData &data, uint32_t offset, uint32_t byte_length,
                          uint16_t x, uint16_t y,
                          const std::vector<uint16_t>& slices,
                          uint32_t options)
{
    void *p = data.allocData(byte_length);
    size_t real_size = m_container->fetchData(p, offset, byte_length);
    if (real_size < byte_length) {
        LOGWARN("Size mismatch for data: ignoring.\n");
    }
    // they are not all RGGB.
    // but I don't seem to see where this is encoded.
    //
    data.setCfaPatternType(OR_CFA_PATTERN_RGGB);
    data.setDataType(OR_DATA_TYPE_COMPRESSED_RAW);
    data.setDimensions(x, y);

    LOGDBG1("In size is %dx%d\n", data.width(), data.height());
    // decompress if we need
    if ((options & OR_OPTIONS_DONT_DECOMPRESS) == 0) {
      IO::Stream::Ptr s(new IO::MemStream((const uint8_t*)data.data(),
                                          data.size()));
      s->open(); // TODO check success
      auto jfif = std::make_unique<JfifContainer>(s, 0);
      LJpegDecompressor decomp(s.get(), jfif.get());
      // in fact on Canon CR2 files slices either do not exists
      // or is 3.
      if (slices.size() > 1) {
        decomp.setSlices(slices);
      }
      RawDataPtr dData = decomp.decompress();
      if (dData) {
        LOGDBG1("Out size is %dx%d\n", dData->width(), dData->height());
        // must re-set the cfaPattern
        dData->setCfaPatternType(data.mosaicInfo()->patternType());
        data.swap(*dData);
        if (!isCr2()) {
            // In TIF the raw image width needs to be divided by two
            uint16_t w = data.width();
            uint16_t h = data.height();
            data.setDimensions(w / 2, h);
        }
      }
    }
}

::or_error Cr2File::getRawDataCr2(RawData &data, uint32_t options)
{
    const IfdDir::Ref &_cfaIfd = cfaIfd();
    if (!_cfaIfd) {
        LOGDBG1("cfa IFD not found\n");
        return OR_ERROR_NOT_FOUND;
    }

    LOGDBG1("_getRawData()\n");

    auto result = _cfaIfd->getValue<uint32_t>(IFD::EXIF_TAG_STRIP_OFFSETS);
    if (result.empty()) {
        LOGDBG1("offset not found\n");
        return OR_ERROR_NOT_FOUND;
    }
    uint32_t offset = result.value();

    result = _cfaIfd->getValue<uint32_t>(IFD::EXIF_TAG_STRIP_BYTE_COUNTS);
    if (result.empty()) {
        LOGDBG1("byte len not found\n");
        return OR_ERROR_NOT_FOUND;
    }
    uint32_t byte_length = result.value();

    // get the "slicing", tag 0xc640 (3 SHORT)
    std::vector<uint16_t> slices;
    IfdEntry::Ref e = _cfaIfd->getEntry(IFD::CR2_TAG_SLICE);
    if (e) {
        auto result2 = _cfaIfd->getEntryArrayValue<uint16_t>(*e);
        if (result2) {
            slices = result2.value();
            LOGDBG1("Found slice entry count %lu\n", (LSIZE)slices.size());
        }
    }

    const IfdDir::Ref &_exifIfd = exifIfd();
    if (!_exifIfd) {
        LOGERR("unable to find ExifIFD\n");
        return OR_ERROR_NOT_FOUND;
    }

    auto result2 = _exifIfd->getValue<uint16_t>(IFD::EXIF_TAG_PIXEL_X_DIMENSION);
    if (result2.empty()) {
        LOGDBG1("X not found\n");
        return OR_ERROR_NOT_FOUND;
    }
    uint16_t x = result2.value();

    result2 = _exifIfd->getValue<uint16_t>(IFD::EXIF_TAG_PIXEL_Y_DIMENSION);
    if (result2.empty()) {
        LOGDBG1("Y not found\n");
        return OR_ERROR_NOT_FOUND;
    }
    uint16_t y = result2.value();

    getRawBytes(data, offset, byte_length, x, y, slices, options);

    // get the sensor info
    const IfdDir::Ref &_makerNoteIfd = makerNoteIfd();
    auto sensorInfo = canon_get_sensorinfo(_makerNoteIfd);
    if (sensorInfo) {
        data.setActiveArea((*sensorInfo)[0], (*sensorInfo)[1],
                           (*sensorInfo)[2], (*sensorInfo)[3]);
    }

    return OR_ERROR_NONE;
}

::or_error Cr2File::_getRawData(RawData &data, uint32_t options)
{
    if (!isCr2()) {
        return getRawDataTif(data, options);
    }
    return getRawDataCr2(data, options);
}

bool Cr2File::vendorCameraIdLocation(Internals::IfdDir::Ref& ifd, uint16_t& index,
                            const ModelIdMap*& model_map)
{
    auto mn = makerNoteIfd();
    if (mn) {
        // There is a camera model ID in the MakerNote tag 0x0010.
        ifd = mn;
        index = IFD::MNOTE_CANON_MODEL_ID;
        model_map = &canon_modelid_map;
        return true;
    }
    return false;
}

bool Cr2File::isCr2()
{
    return ((OR_GET_FILE_TYPEID_CAMERA(typeId()) != OR_TYPEID_CANON_1D)
            && (OR_GET_FILE_TYPEID_CAMERA(typeId()) != OR_TYPEID_CANON_1DS));
}

/** @} */

}
}

