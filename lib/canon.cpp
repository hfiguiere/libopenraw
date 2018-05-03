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

Option<std::array<uint32_t, 4>> canon_get_sensorinfo(const IfdDir::Ref& ifddir)
{
    auto e = ifddir->getEntry(IFD::MNOTE_CANON_SENSORINFO);
    if (!e) {
        return OptionNone();
    }
    auto result3 = e->getArray<uint16_t>();
    if (result3) {
        std::vector<uint16_t> sensorInfo = result3.value();
        if (sensorInfo.size() > 8) {
            std::array<uint32_t, 4> result;
            result[0] = sensorInfo[5];
            result[1] = sensorInfo[6];
            result[2] = sensorInfo[7] - sensorInfo[5];
            result[3] = sensorInfo[8] - sensorInfo[6];
            return option_some(std::move(result));
        }
        else {
            LOGWARN("sensorInfo is too small: %lu - skipping.\n",
                    sensorInfo.size());
        }
    }
    return OptionNone();
}

}
}
