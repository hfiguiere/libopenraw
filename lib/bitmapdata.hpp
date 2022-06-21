/*
 * libopenraw - bitmapdata.h
 *
 * Copyright (C) 2007-2020 Hubert Figuière
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

#pragma once

#include <libopenraw/prefix.h>
#include <libopenraw/rawdata.h>

namespace OpenRaw {

/** @brief Represent some bitmap data. */
class BitmapData
{
public:
	typedef ::or_data_type DataType;

	BitmapData();
	virtual ~BitmapData();

	/** @brief Swap the two objects data. */
	void swap(BitmapData & with);

	/** @brief Get the data type */
	DataType dataType() const;
	/** @brief Set the data type */
	void setDataType(DataType _type);

	virtual void *allocData(const size_t s);
	/** @brief Get the size of the data */
	size_t size() const;
	void *data() const;

	/** @brief Width of the image data */
	uint32_t width() const;
	/** @brief Height of the image data */
	uint32_t height() const;
	/** @brief Bit per channel */
	uint32_t bpc() const;
	/** @brief Set bit per channel */
	void setBpc(uint32_t _bpc);

	/** @brief Set the pixel dimensions of the bitmap */
	virtual void setDimensions(uint32_t x, uint32_t y);

	/** Adjust the size after allocation. If size is bigger
	 *  than allocated size, then it's a no-op.
	 */
	void adjustSize(size_t size);
private:
	class Private;
	BitmapData::Private *d;

	/** private copy constructor to make sure it is not called */
	BitmapData(const BitmapData& f);
	/** private = operator to make sure it is never called */
	BitmapData & operator=(const BitmapData&);
};

}
