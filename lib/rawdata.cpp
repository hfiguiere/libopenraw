/*
 * libopenraw - rawdata.cpp
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

#include <assert.h>

#include <libopenraw++/rawdata.h>
#include <libopenraw++/rawfile.h>

namespace OpenRaw {

	class RawData::Private {
	public:
		RawData *self;
		CfaPattern cfa_pattern;
		uint32_t compression;
		uint8_t *pos;
		size_t offset;
		size_t row_offset;
		uint8_t slice;
		uint32_t sliceWidth;
		uint32_t sliceOffset;

		std::vector<uint16_t> slices;

		Private(RawData *_self)
			:	self(_self), cfa_pattern(OR_CFA_PATTERN_NONE),
				compression(0),
				pos(NULL), offset(0),
				row_offset(0),
				slice(0), sliceWidth(0),
				sliceOffset(0), slices()
			{
			}
		void advance(size_t s);
		void nextSlice();
		void nextRow();
	private:
		Private(const Private &);
		Private & operator=(const Private &);
	};


	RawData *
	RawData::getAndExtractRawData(const char* filename, uint32_t options,
																or_error & err)
	{
		err = OR_ERROR_NONE;
		RawData *rawdata = NULL;

		RawFile *file = RawFile::newRawFile(filename);
		if (file) {
			rawdata = new RawData();
			err = file->getRawData(*rawdata, options);
			delete file;
		}
		else {
			err = OR_ERROR_CANT_OPEN; // file error
		}
		return rawdata;
	}


	RawData::RawData()
		: BitmapData(),
		  d(new RawData::Private(this))
	{

	}


	RawData::~RawData()
	{
		delete d;
	}


	void RawData::swap(RawData & with)
	{
		BitmapData::swap(with);
		std::swap(this->d, with.d);
	}

 	void * RawData::allocData(const size_t s)
	{
		void * p = BitmapData::allocData(s);
		d->pos = (uint8_t*)p;
		d->offset = 0;
		return p;
	}


	void RawData::setDimensions(uint32_t _x, uint32_t _y)
	{
		BitmapData::setDimensions(_x, _y);
		if(d->slices.size()) {
			d->sliceWidth = d->slices[0];
		}
		else {
			d->sliceWidth = _x;
		}
	}

	void RawData::setSlices(const std::vector<uint16_t> & slices)
	{
		d->slices = slices;
		if(slices.size()) {
			d->sliceWidth = slices[0];
		}
		else {
			d->sliceWidth = x();
		}
	}

	void RawData::setCfaPattern(or_cfa_pattern t)
	{
		d->cfa_pattern = t;
	}

	or_cfa_pattern RawData::cfaPattern()
	{
		return d->cfa_pattern;
	}

	void RawData::setCompression(uint32_t t)
	{
		d->compression = t;
	}

	uint32_t RawData::compression()
	{
		return d->compression;
	}

#if 0
	RawData &RawData::append(uint8_t c)
	{
		assert(d->pos);
		assert(d->offset < d->data_size);
		*(d->pos) = c;
		advance(sizeof(c));
		return *this;
	}
#endif

	RawData &RawData::append(uint16_t c)
	{
		assert(d->pos);
//		assert(d->offset < d->data_size);
		*(d->pos) = c & 0xff;
		*(d->pos + 1) = (c >> 8) & 0xff; 
		d->advance(sizeof(c));
		return *this;
	}
	

	void RawData::nextRow()
	{
		d->nextRow();
	}


	void RawData::Private::nextRow()
	{
		uint32_t w = self->x() * 2;
		uint32_t row = offset / w;
		row++;
		if(row == self->y()) 
		{
			// on the last
			nextSlice();
			row = 0;
		}
		offset = row * w + sliceOffset * 2;
		pos = (uint8_t*)(self->data()) + offset;
		row_offset = offset;
	}

	void RawData::Private::nextSlice()
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
	
	void RawData::Private::advance(size_t s)
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

