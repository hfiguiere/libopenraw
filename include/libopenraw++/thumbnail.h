/*
 * libopenraw - thumbnail.h
 *
 * Copyright (C) 2005-2007 Hubert Figuiere
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


#ifndef __OPENRAW_THUMBNAIL_H__
#define __OPENRAW_THUMBNAIL_H__


#include <libopenraw/libopenraw.h>
#include <libopenraw++/bitmapdata.h>

namespace OpenRaw {

/** real thumbnail extracted */
	class Thumbnail 
		: public BitmapData
	{
	public:
		Thumbnail();
		virtual ~Thumbnail();

		/** quick and dirty "get this thumbnail" 
		 * @param _filename the filename
		 * @param preferred_size the size of the thumbnail
		 * @retval err the error code
		 * @return a Thumbnail object. Callers own it and must delete it.
		 */
		static Thumbnail *
		getAndExtractThumbnail(const char *_filename,
													 uint32_t preferred_size,
													 ::or_error & err);

	private:

		Thumbnail(const Thumbnail&);
		Thumbnail & operator=(const Thumbnail &);

		class Private;
		Thumbnail::Private *d;
	};

}

#endif

