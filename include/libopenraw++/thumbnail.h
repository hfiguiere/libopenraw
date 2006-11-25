/*
 * libopenraw - thumbnail.h
 *
 * Copyright (C) 2005 Hubert Figuiere
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#ifndef __OPENRAW_THUMBNAIL_H__
#define __OPENRAW_THUMBNAIL_H__


#include <libopenraw/libopenraw.h>


namespace OpenRaw {

/** real thumbnail extracted */
	class Thumbnail 
	{
	public:
		typedef ::or_data_type DataType;

		Thumbnail();
		~Thumbnail();

		/** quick and dirty "get this thumbnail" */
		static Thumbnail *
		getAndExtractThumbnail(const char *_filename,
													 uint32_t preferred_size);

		/** return the data type */
		DataType dataType() const;
		/** set the data type */
		void setDataType(Thumbnail::DataType _type);
		void *allocData(const size_t s);
		/** return the size of the data */
		size_t size() const;
		void *data() const;
		
		uint32_t x();
		uint32_t y();
		/** set the pixel dimensions of the thumbnail */
		void setDimensions(uint32_t x, uint32_t y);
	private:

		Thumbnail(const Thumbnail&);
		Thumbnail & operator=(const Thumbnail &);

		class Private;
		Thumbnail::Private *d;
	};

}

#endif

