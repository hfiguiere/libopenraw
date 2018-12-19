/* -*- mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil; -*- */
/*
 * libopenraw - canon.cpp
 *
 * Copyright (C) 2018 Hubert Figuiere
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

#include <array>

#include "canon.hpp"
#include "ifddir.hpp"
#include "option.hpp"
#include "trace.hpp"

namespace OpenRaw {
namespace Internals {

namespace {

#define OR_MAKE_CANON_TYPEID(camid)                     \
    OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_CANON, camid)

const std::map<uint32_t, RawFile::TypeId> type_map = {
    // TIF
    { 0x80000001, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1D) },
    { 0x80000167, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1DS) },
    // CRW and CR2
    { 0x80000174, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1DMKII) },
    { 0x80000175, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_20D) },
    { 0x80000188, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1DSMKII) },
    { 0x80000189, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_350D) },
    { 0x80000213, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_5D) },
    { 0x80000232, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1DMKIIN) },
    { 0x80000234, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_30D) },
    { 0x80000236, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_400D) },
    { 0x80000169, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1DMKIII) },
    { 0x80000190, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_40D) },
    { 0x80000215, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1DSMKIII) },
    { 0x02230000, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G9) },
    { 0x80000176, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_450D) },
    { 0x80000254, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1000D) },
    { 0x80000261, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_50D) },
    { 0x02490000, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G10) },
    { 0x80000218, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_5DMKII) },
    { 0x02460000, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_SX1_IS) },
    { 0x80000252, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_500D) },
    { 0x02700000, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G11) },
    { 0x02720000, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_S90) },
    { 0x80000250, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_7D) },
    { 0x80000281, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1DMKIV) },
    { 0x80000270, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_550D) },
    { 0x02950000, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_S95) },
    { 0x80000287, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_60D) },
    { 0x02920000, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G12) },
    { 0x80000286, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_600D) },
    { 0x80000288, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1100D) },
    { 0x03110000, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_S100) },
    { 0x80000269, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1DX) },
    { 0x03080000, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G1X) },
    { 0x80000285, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_5DMKIII) },
    { 0x80000301, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_650D) },
    { 0x80000331, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M) },
    { 0x03360000, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_S110) },
    { 0x03330000, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G15) },
    { 0x03340000, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_SX50_HS) },
    { 0x80000302, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_6D) },
    { 0x80000326, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_700D) },
    { 0x80000346, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_100D) },
    { 0x80000325, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_70D) },
    { 0x03540000, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G16) },
    { 0x03550000, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_S120) },
//    { 0x80000355, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M2) },
    { 0x80000327, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1200D) },
    { 0x03640000, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G1XMKII) },
    { 0x80000289, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_7DMKII) },
    { 0x03780000, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G7X) },
    { 0x03750000, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_SX60_HS) },
    { 0x80000382, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_5DS) },
    { 0x80000401, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_5DS_R) },
    { 0x80000393, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_750D) },
    { 0x80000347, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_760D) },
    { 0x03740000, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M3) },
    { 0x03850000, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G3X) },
    { 0x03950000, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G5X) },
    { 0x03930000, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G9X) },
    { 0x03840000, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M10) },
    { 0x80000328, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1DXMKII) },
    { 0x80000350, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_80D) },
    { 0x03970000, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G7XMKII) },
    { 0x80000404, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_1300D) },
    { 0x80000349, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_5DMKIV) },
    { 0x03940000, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M5) },
    { 0x04100000, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G9XMKII) },
    { 0x80000405, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_800D) },
    { 0x80000408, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_77D) },
    { 0x04070000, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M6) },
    { 0x80000417, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_200D) },
    { 0x80000406, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_6DMKII) },
    { 0x03980000, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M100) },
    { 0x04180000, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_G1XMKIII) },
    { 0x80000432, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_2000D) },
    { 0x80000422, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_3000D) },
    // CR3
    { 0x00000412, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_M50) },
    { 0x80000424, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_EOS_R) },
    { 0x00000805, OR_MAKE_CANON_TYPEID(OR_TYPEID_CANON_SX70_HS) },
};

}

RawFile::TypeId canon_modelid_to_typeid(uint32_t model_id)
{
    auto iter = type_map.find(model_id);
    if (iter != type_map.end()) {
        return iter->second;
    }
    return 0;
}

Option<std::array<uint32_t, 4>>
canon_parse_sensorinfo(const std::vector<uint16_t>& sensor_info)
{
    if (sensor_info.size() > 8) {
        std::array<uint32_t, 4> result;
        result[0] = sensor_info[5];
        result[1] = sensor_info[6];
        if (sensor_info[7] <= sensor_info[5]) {
            LOGWARN("sensor_info: bottom %u <= top %u\n",
                    sensor_info[7], sensor_info[5]);
            return OptionNone();
        }
        uint32_t w = sensor_info[7] - sensor_info[5];
        // it seems that this could lead to an odd number. Make it even.
        if (w % 2) {
            w++;
        }
        result[2] = w;
        if (sensor_info[8] <= sensor_info[6]) {
            LOGWARN("sensor_info: right %u <= left %u\n",
                    sensor_info[8], sensor_info[6]);
            return OptionNone();
        }
        uint32_t h = sensor_info[8] - sensor_info[6];
        // same as for width
        if (h % 2) {
            h++;
        }
        result[3] = h;
        return option_some(std::move(result));
    }
    else {
        LOGWARN("SensorInfo is too small: %lu - skipping.\n",
                sensor_info.size());
    }
    return OptionNone();
}

Option<std::array<uint32_t, 4>> canon_get_sensorinfo(const IfdDir::Ref& ifddir)
{
    auto e = ifddir->getEntry(IFD::MNOTE_CANON_SENSORINFO);
    if (!e) {
        return OptionNone();
    }
    auto result = e->getArray<uint16_t>();
    if (result) {
        return canon_parse_sensorinfo(result.value());
    }
    return OptionNone();
}

}
}
