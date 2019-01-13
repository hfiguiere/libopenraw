/* -*- mode:c++; indent-tabs-mode:nil; c-basic-offset:4; tab-width:4; -*- */
/*
 * libopenraw - cfapattern.cpp
 *
 * Copyright (C) 2016-2019 Hubert Figuière
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

#include <libopenraw/cfapattern.h>

#include "capi.h"
#include "cfapattern.hpp"

extern "C" {

API_EXPORT or_cfa_pattern
or_cfapattern_get_type(ORCfaPatternRef pattern)
{
    return reinterpret_cast<const OpenRaw::CfaPattern*>(pattern)->patternType();
}

API_EXPORT const uint8_t *
or_cfapattern_get_pattern(ORCfaPatternRef pattern, uint16_t *count)
{
    // TODO check parameters.
    auto pat = reinterpret_cast<const OpenRaw::CfaPattern*>(pattern);
    return pat->patternPattern(*count);
}

}

