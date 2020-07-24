/*
 * libopenraw - exif_tags.hpp
 *
 * Copyright (C) 2020 Hubert Figuière
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

#include <stdint.h>
#include <map>

#pragma once

extern std::map<uint32_t, const char*> exif_tag_names;

extern std::map<uint32_t, const char*> mnote_canon_tag_names;
extern std::map<uint32_t, const char*> mnote_fujifilm_tag_names;
extern std::map<uint32_t, const char*> mnote_nikon_tag_names;
extern std::map<uint32_t, const char*> mnote_olympus_tag_names;
extern std::map<uint32_t, const char*> mnote_panasonic_tag_names;
extern std::map<uint32_t, const char*> mnote_pentax_tag_names;
extern std::map<uint32_t, const char*> mnote_sony_tag_names;

extern std::map<uint32_t, const char*> raw_panasonic_tag_names;
