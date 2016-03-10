/*
 * libopenraw - bitmapdata.h
 *
 * Copyright (C) 2007 Hubert Figuiere
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


#ifndef LIBOPENRAWPP_BITMAPDATA_H_
#define LIBOPENRAWPP_BITMAPDATA_H_

#include <libopenraw/prefix.h>
#include <libopenraw/rawdata.h>


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
	
	/** width of the image data */
	OR_DEPRECATED uint32_t x() const;
	uint32_t width() const;
	/** height of the image data */
	OR_DEPRECATED uint32_t y() const;
	uint32_t height() const;
	/** bit per channel */
	uint32_t bpc() const;
	/** set bit per channel */
	void setBpc(uint32_t _bpc);

	/** set the pixel dimensions of the bitmap */
	virtual void setDimensions(uint32_t x, uint32_t y);

	/** retrieve the region of interest within the data 
	 *  the only guarantee is that if the width or height is 0 
	 *  when setting the dimensions the first time
	 *  they'll be set to width() and height()
	 */
	uint32_t roi_x() const;
	uint32_t roi_y() const;
	uint32_t roi_width() const;
	uint32_t roi_height() const;
	void setRoi(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
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
