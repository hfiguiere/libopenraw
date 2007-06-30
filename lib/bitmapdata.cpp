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

		uint8_t *pos;
		size_t offset;
		size_t row_offset;
		uint8_t slice;
		uint32_t sliceWidth;
		uint32_t sliceOffset;

		std::vector<uint16_t> slices;

		Private()
			: data(NULL),
				data_size(0),
				data_type(OR_DATA_TYPE_NONE),
				x(0), y(0), bpc(0),
				pos(NULL), offset(0),
				row_offset(0),
				slice(0), sliceWidth(0),
				sliceOffset(0)
			{
			}
		
		~Private()
			{
				if (NULL != data) {
					free(data);
				}
			}
		void advance(size_t s);
		void nextSlice();
		void nextRow();
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
		d->pos = (uint8_t*)d->data;
		d->offset = 0;
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


	void BitmapData::setDimensions(uint32_t x, uint32_t y)
	{
		d->x = x;
		d->y = y;
		if(d->slices.size()) {
			d->sliceWidth = d->slices[0];
		}
		else {
			d->sliceWidth = d->x;
		}
	}

	void BitmapData::setSlices(const std::vector<uint16_t> & slices)
	{
		d->slices = slices;
		if(slices.size()) {
			d->sliceWidth = slices[0];
		}
		else {
			d->sliceWidth = d->x;
		}
	}


	void BitmapData::setBpc(uint32_t _bpc)
	{
		d->bpc = _bpc;
	}

#if 0
	BitmapData &BitmapData::append(uint8_t c)
	{
		assert(d->pos);
		assert(d->offset < d->data_size);
		*(d->pos) = c;
		advance(sizeof(c));
		return *this;
	}
#endif

	BitmapData &BitmapData::append(uint16_t c)
	{
		assert(d->pos);
		assert(d->offset < d->data_size);
		*(uint16_t*)(d->pos) = c;
		d->advance(sizeof(c));
		return *this;
	}
	

	void BitmapData::nextRow()
	{
		d->nextRow();
	}


	void BitmapData::Private::nextRow()
	{
		uint32_t w = x * 2;
		uint32_t row = offset / w;
		row++;
		if(row == y) 
		{
			// on the last
			nextSlice();
			row = 0;
		}
		offset = row * w + sliceOffset * 2;
		pos = (uint8_t*)(data) + offset;
		row_offset = offset;
	}

	void BitmapData::Private::nextSlice()
	{
		if(slices.size()) {
			sliceOffset += slices[slice];
			slice++;
			if(slices.size() > slice) {
				sliceWidth = slices[slice];
			}
			else {
				sliceWidth = 0;
			}
		}
	}
	
	void BitmapData::Private::advance(size_t s)
	{
		if(offset + s - row_offset >= sliceWidth * 2) {
			nextRow();
		}
		else { 
			pos += s;
			offset += s;
		}
	}

}
