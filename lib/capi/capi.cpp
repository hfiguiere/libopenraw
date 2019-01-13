/* -*- mode:c++; indent-tabs-mode:nil; c-basic-offset:4; tab-width:4; -*- */
/*
 * libopenraw - capi.cpp
 *
 * Copyright (C) 2005-2019 Hubert Figuière
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
/**
 * @brief the libopenraw public C API
 * @author Hubert Figuiere <hub@figuiere.net>
 */

#include <stddef.h>
#include <stdint.h>

#include <libopenraw/consts.h>
#include <libopenraw/thumbnails.h>

#include "capi.h"
#include "thumbnail.hpp"

using OpenRaw::Thumbnail;

extern "C" {

API_EXPORT
or_error or_get_extract_thumbnail(const char* _filename,
                                  uint32_t _preferred_size,
                                  ORThumbnailRef *_thumb)
{
    or_error ret = OR_ERROR_NONE;

    Thumbnail ** pThumbnail = reinterpret_cast<Thumbnail **>(_thumb);
    *pThumbnail = Thumbnail::getAndExtractThumbnail(_filename,
                                                    _preferred_size, ret);
    return ret;
}

API_EXPORT
ORThumbnailRef or_thumbnail_new(void)
{
    Thumbnail *thumb = new Thumbnail();
    return reinterpret_cast<ORThumbnailRef>(thumb);
}

API_EXPORT or_error
or_thumbnail_release(ORThumbnailRef thumb)
{
    if (thumb == nullptr) {
        return OR_ERROR_NOTAREF;
    }
    delete reinterpret_cast<Thumbnail *>(thumb);
    return OR_ERROR_NONE;
}

API_EXPORT or_data_type
or_thumbnail_format(ORThumbnailRef thumb)
{
    return reinterpret_cast<Thumbnail *>(thumb)->dataType();
}

API_EXPORT void *
or_thumbnail_data(ORThumbnailRef thumb)
{
    return reinterpret_cast<Thumbnail *>(thumb)->data();
}

API_EXPORT size_t
or_thumbnail_data_size(ORThumbnailRef thumb)
{
    return reinterpret_cast<Thumbnail *>(thumb)->size();
}

API_EXPORT void
or_thumbnail_dimensions(ORThumbnailRef thumb, uint32_t *width, uint32_t *height)
{
    Thumbnail* t = reinterpret_cast<Thumbnail *>(thumb);
    if (width != nullptr) {
        *width = t->width();
    }
    if (height != nullptr) {
        *height = t->height();
    }
}


}

