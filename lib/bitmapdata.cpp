/*
 * libopenraw - bitmapdata.cpp
 *
 * Copyright (C) 2007-2015 Hubert Figuiere
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

#include <stddef.h>
#include <stdint.h>
#include <algorithm>
#include <cstdlib>

#include "trace.h"

#include <libopenraw/consts.h>
#include <libopenraw/debug.h>
#include <libopenraw++/bitmapdata.h>

using namespace Debug;

namespace OpenRaw {

class BitmapData::Private {
public:
    /** raw data */
    void *data;
    /** size in bytes of raw data */
    size_t data_size;
    /** type of thumbnail data */
    DataType data_type;
    /** width dimension in pixels of image data */
    uint32_t width;
    /** height dimension in pixels of image data */
    uint32_t height;
    /** bpc bit per channel. 0 is not a valid value */
    uint32_t bpc;
    /** region of interest */
    uint32_t roi_x;
    uint32_t roi_y;
    uint32_t roi_w;
    uint32_t roi_h;

    Private()
        : data(NULL)
        , data_size(0)
        , data_type(OR_DATA_TYPE_NONE)
        , width(0)
        , height(0)
        , bpc(0)
        , roi_x(0)
        , roi_y(0)
        , roi_w(0)
        , roi_h(0)
    {
    }

    ~Private()
    {
        if (NULL != data) {
            free(data);
        }
    }

private:
    Private(const Private &);
    Private &operator=(const Private &);
};

BitmapData::BitmapData() : d(new BitmapData::Private())
{
}

BitmapData::~BitmapData()
{
    delete d;
}

void BitmapData::swap(BitmapData &with)
{
    std::swap(this->d, with.d);
}

BitmapData::DataType BitmapData::dataType() const
{
    return d->data_type;
}

void BitmapData::setDataType(BitmapData::DataType _type)
{
    d->data_type = _type;
    if (d->bpc == 0) {
        switch (_type) {
        case OR_DATA_TYPE_NONE:
            d->bpc = 0;
            break;
        case OR_DATA_TYPE_COMPRESSED_RAW:
        case OR_DATA_TYPE_RAW:
            d->bpc = 16;
            break;
        case OR_DATA_TYPE_PIXMAP_8RGB:
        case OR_DATA_TYPE_JPEG:
        default:
            d->bpc = 8;
        }
    }
}

void *BitmapData::allocData(const size_t s)
{
    Trace(DEBUG1) << "allocate s=" << s << " data =" << d->data << "\n";
    d->data = calloc(s, 1);
    Trace(DEBUG1) << " data =" << d->data << "\n";
    d->data_size = s;
    return d->data;
}

size_t BitmapData::size() const
{
    return d->data_size;
}

void *BitmapData::data() const
{
    return d->data;
}

uint32_t BitmapData::x() const
{
    return d->width;
}

uint32_t BitmapData::width() const
{
    return d->width;
}

uint32_t BitmapData::y() const
{
    return d->height;
}

uint32_t BitmapData::height() const
{
    return d->height;
}

uint32_t BitmapData::bpc() const
{
    return d->bpc;
}

void BitmapData::setBpc(uint32_t _bpc)
{
    d->bpc = _bpc;
}

void BitmapData::setDimensions(uint32_t _width, uint32_t _height)
{
    d->width = _width;
    d->height = _height;
    if (d->roi_w == 0) {
        d->roi_w = _width;
    }
    if (d->roi_h == 0) {
        d->roi_h = _height;
    }
}

uint32_t BitmapData::roi_x() const
{
    return d->roi_x;
}

uint32_t BitmapData::roi_y() const
{
    return d->roi_y;
}

uint32_t BitmapData::roi_width() const
{
    return d->roi_w;
}

uint32_t BitmapData::roi_height() const
{
    return d->roi_h;
}

void BitmapData::setRoi(uint32_t _x, uint32_t _y, uint32_t w, uint32_t h)
{
    d->roi_x = _x;
    d->roi_y = _y;
    d->roi_w = w;
    d->roi_h = h;
}
}
