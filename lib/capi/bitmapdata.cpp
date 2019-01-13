/*
 * libopenraw - bitmapdata.cpp
 *
 * Copyright (C) 2007-2019 Hubert Figuiere
 * Copyright (C) 2008 Novell Inc.
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
/* @brief C api for bitmapdata
 */

#include <stddef.h>
#include <stdint.h>

#include <libopenraw/types.h>
#include <libopenraw/consts.h>

#include "capi.h"
#include "bitmapdata.hpp"

using OpenRaw::BitmapData;

extern "C" {

API_EXPORT
ORBitmapDataRef or_bitmapdata_new(void)
{
    BitmapData *bitmapdata = new BitmapData();
    return reinterpret_cast<ORBitmapDataRef>(bitmapdata);
}

API_EXPORT
or_error or_bitmapdata_release(ORBitmapDataRef bitmapdata)
{
    if (bitmapdata == NULL) {
        return OR_ERROR_NOTAREF;
    }
    delete reinterpret_cast<BitmapData *>(bitmapdata);
    return OR_ERROR_NONE;
}

API_EXPORT
or_data_type or_bitmapdata_format(ORBitmapDataRef bitmapdata)
{
    return reinterpret_cast<BitmapData *>(bitmapdata)->dataType();
}

API_EXPORT
void *or_bitmapdata_data(ORBitmapDataRef bitmapdata)
{
    return reinterpret_cast<BitmapData *>(bitmapdata)->data();
}

API_EXPORT
size_t or_bitmapdata_data_size(ORBitmapDataRef bitmapdata)
{
    return reinterpret_cast<BitmapData *>(bitmapdata)->size();
}

API_EXPORT
void or_bitmapdata_dimensions(ORBitmapDataRef bitmapdata, uint32_t *x,
                              uint32_t *y)
{
    BitmapData *t = reinterpret_cast<BitmapData *>(bitmapdata);
    if (x != NULL) {
        *x = t->width();
    }
    if (y != NULL) {
        *y = t->height();
    }
}

API_EXPORT
uint32_t or_bitmapdata_bpc(ORBitmapDataRef bitmapdata)
{
    return reinterpret_cast<BitmapData *>(bitmapdata)->bpc();
}
}
