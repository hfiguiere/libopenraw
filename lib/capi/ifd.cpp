/* -*- mode:c++; indent-tabs-mode:nil; c-basic-offset:4; tab-width:4; -*- */
/*
 * libopenraw - ifd.cpp
 *
 * Copyright (C) 2019-2020 Hubert Figuière
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
    CHECK_PTR(ifd, -1);
    auto wrap = reinterpret_cast<WrappedPointer<OpenRaw::Internals::IfdDir>*>(ifd);
    auto pifd = wrap->ptr();
    return pifd->numTags();
}

API_EXPORT const char*
or_ifd_get_makernote_id(ORIfdDirRef ifd)
{
    CHECK_PTR(ifd, nullptr);
    auto wrap = reinterpret_cast<WrappedPointer<OpenRaw::Internals::IfdDir>*>(ifd);
    auto pifd = wrap->ptr();
    auto maker_note = std::dynamic_pointer_cast<OpenRaw::Internals::MakerNoteDir>(pifd);
    return maker_note->getId().c_str();
}

API_EXPORT const char*
or_ifd_get_tag_name(ORIfdDirRef ifd, uint32_t tag)
{
    CHECK_PTR(ifd, nullptr);
    auto wrap = reinterpret_cast<WrappedPointer<OpenRaw::Internals::IfdDir>*>(ifd);
    auto pifd = wrap->ptr();
    return pifd->getTagName(tag);
}

API_EXPORT or_ifd_dir_type
or_ifd_get_type(ORIfdDirRef ifd)
{
    CHECK_PTR(ifd, OR_IFD_INVALID);
    auto wrap = reinterpret_cast<WrappedPointer<OpenRaw::Internals::IfdDir>*>(ifd);
    auto pifd = wrap->ptr();
    return pifd->type();
}

API_EXPORT void
or_ifd_release(ORIfdDirRef ifd)
{
    if (!ifd) {
        return;
    }
    auto wrap = reinterpret_cast<WrappedPointer<OpenRaw::Internals::IfdDir>*>(ifd);
    delete wrap;
}

}
