/* -*- mode:c++; indent-tabs-mode:nil; c-basic-offset:4; tab-width:4; -*- */
/*
 * libopenraw - ifd.cpp
 *
 * Copyright (C) 2019 Hubert Figuière
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

#include <libopenraw/types.h>

#include "capi.h"

#include "makernotedir.hpp"

extern "C" {
/** check pointer validity */
#define CHECK_PTR(p, r)                                                        \
    if (p == nullptr) {                                                           \
        return r;                                                              \
    }

API_EXPORT int32_t
or_ifd_count_tags(ORIfdDirRef ifd)
{
    auto pifd = reinterpret_cast<OpenRaw::Internals::IfdDir*>(ifd);
    CHECK_PTR(ifd, -1);
    return pifd->numTags();
}

API_EXPORT const char*
or_ifd_get_makernote_id(ORIfdDirRef ifd)
{
    auto pifd = reinterpret_cast<OpenRaw::Internals::IfdDir*>(ifd);
    CHECK_PTR(ifd, nullptr);
    auto maker_note = dynamic_cast<OpenRaw::Internals::MakerNoteDir*>(pifd);
    return maker_note->getId().c_str();
}

}
