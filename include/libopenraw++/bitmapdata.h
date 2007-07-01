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

		virtual void *allocData(const size_t s);
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
		virtual void setDimensions(uint32_t x, uint32_t y);

	private:
		class Private;
		BitmapData::Private *d;

		/** private copy constructor to make sure it is not called */
		BitmapData(const BitmapData& f);
		/** private = operator to make sure it is never called */
		BitmapData & operator=(const BitmapData&);
	};
	
}



#endif
