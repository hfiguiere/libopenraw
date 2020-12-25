/* -*- mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil; -*- */
/*
 * libopenraw - canon.hpp
 *
 * Copyright (C) 2018-2020 Hubert Figuière
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

#pragma once

#include <array>

#include <libopenraw/cameraids.h>

#include "rawfile.hpp"
#include "ifddir.hpp"
#include "option.hpp"

namespace OpenRaw {
namespace Internals {

/** @defgroup canon Canon parsing
 * @ingroup internals
 *
 * @brief Canon parsing for all variety or Canon raw files.
 *
 * Despite different format, Canon have somewhat consistent strucures
 * across their product line.
 *
 * @{ */

/** @brief The model ID map for Canon cameras. */
extern const ModelIdMap canon_modelid_map;

/** @brief Map a Canon model ID to a type ID. */
RawFile::TypeId canon_modelid_to_typeid(uint32_t model_id);

/**
 * @brief Parse the sensor info from a buffer of uint16_t
 */
Option<std::array<uint32_t, 4>>
canon_parse_sensorinfo(const std::vector<uint16_t>& sensor_info);
/**
 * @brief Get the sensor info from and IfdDir
 */
Option<std::array<uint32_t, 4>>
canon_get_sensorinfo(const IfdDir::Ref& ifddir);

/** @} */
}
}
