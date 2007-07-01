/*
 * libopenraw - bitmapdata.cpp
 *
 * Copyright (C) 2007 Hubert Figuiere
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <assert.h>

#include "debug.h"

#include <libopenraw/libopenraw.h>
#include <libopenraw++/rawfile.h>
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
		/** x dimension in pixels of thumbnail data */
		uint32_t x;
		/** y dimension in pixels of thumbnail data */
		uint32_t y;
		/** bpc bit per channel. 0 is not a valid value */
		uint32_t bpc;


		Private()
			: data(NULL),
				data_size(0),
				data_type(OR_DATA_TYPE_NONE),
				x(0), y(0), bpc(0)
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
		Private & operator=(const Private &);
	};


	BitmapData::BitmapData()
		: d(new BitmapData::Private())
	{
	}

	BitmapData::~BitmapData()
	{
		delete d;
	}

	void BitmapData::swap(BitmapData & with)
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
		if(d->bpc == 0) {
			switch(_type) {
			case OR_DATA_TYPE_NONE:
				d->bpc = 0;
				break;
			case OR_DATA_TYPE_COMPRESSED_CFA:
			case OR_DATA_TYPE_CFA:
				d->bpc = 16;
				break;
			case OR_DATA_TYPE_PIXMAP_8RGB:
			case OR_DATA_TYPE_JPEG:
			default:
				d->bpc = 8;
			}
		}
	}

 	void * BitmapData::allocData(const size_t s)
	{
		Trace(DEBUG1) << "allocate s=" << s << " data =" 
							<< d->data << "\n";
		d->data = calloc(s, 1);
		Trace(DEBUG1) << " data =" << d->data << "\n";
		d->data_size = s;
		return d->data;
	}

	size_t BitmapData::size() const
	{
		return d->data_size;
	}

	void * BitmapData::data() const
	{
		return d->data;
	}

	uint32_t BitmapData::x() const
	{
		return d->x;
	}

	uint32_t BitmapData::y() const
	{
		return d->y;
	}

	uint32_t BitmapData::bpc() const
	{
		return d->bpc;
	}


	void BitmapData::setDimensions(uint32_t _x, uint32_t _y)
	{
		d->x = _x;
		d->y = _y;
	}

	void BitmapData::setBpc(uint32_t _bpc)
	{
		d->bpc = _bpc;
	}


}
