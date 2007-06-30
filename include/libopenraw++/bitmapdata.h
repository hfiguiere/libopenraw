/*
 * libopenraw - bitmapdata.h
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


#ifndef __OPENRAW_BITMAPDATA_H__
#define __OPENRAW_BITMAPDATA_H__

#include <vector>

#include <libopenraw/libopenraw.h>


namespace OpenRaw {

	class BitmapData
	{
	public:
		typedef ::or_data_type DataType;

		BitmapData();
		virtual ~BitmapData();

		/** swap the two objects data. */
		void swap(BitmapData & with);
		
		/** return the data type */
		DataType dataType() const;
		/** set the data type */
		void setDataType(DataType _type);

		void *allocData(const size_t s);
		/** return the size of the data */
		size_t size() const;
		void *data() const;
		
		uint32_t x() const;
		uint32_t y() const;
		/** bit per channel */
		uint32_t bpc() const;
		/** set bit per channel */
		void setBpc(uint32_t _bpc);

		/** set the pixel dimensions of the thumbnail */
		void setDimensions(uint32_t x, uint32_t y);

		void setSlices(const std::vector<uint16_t> & slices);

		/** append a uint8_t at the current position */
//		BitmapData &append(uint8_t c);
		/** append a uint18_t at the current position */
		BitmapData &append(uint16_t c);
		/** Jump to next row. Take slicing into account. */
		void nextRow();
	private:
		class Private;
		BitmapData::Private *d;
	};
	
}



#endif
