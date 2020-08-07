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

#pragma once

#include <stdint.h>
#include <map>

typedef std::map<uint32_t, const char*> TagTable;

extern const TagTable exif_tag_names;

extern const TagTable mnote_canon_tag_names;
extern const TagTable mnote_fujifilm_tag_names;
extern const TagTable mnote_leica2_tag_names;
extern const TagTable mnote_leica4_tag_names;
extern const TagTable mnote_leica5_tag_names;
extern const TagTable mnote_leica6_tag_names;
extern const TagTable mnote_leica9_tag_names;
extern const TagTable mnote_minolta_tag_names;
extern const TagTable mnote_nikon_tag_names;
extern const TagTable mnote_nikon2_tag_names;
extern const TagTable mnote_olympus_tag_names;
extern const TagTable mnote_panasonic_tag_names;
extern const TagTable mnote_pentax_tag_names;
extern const TagTable mnote_ricoh_tag_names;
extern const TagTable mnote_sony_tag_names;

extern const TagTable raw_panasonic_tag_names;
