/*
 * libopenraw - thumbnail.cpp
 *
 * Copyright (C) 2005-2007 Hubert Figuiere
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

#include <cstdlib>
#include <cstring>
#include <iostream>

#include "trace.h"

#include <libopenraw/libopenraw.h>
#include <libopenraw++/rawfile.h>
#include <libopenraw++/thumbnail.h>

using namespace Debug;

namespace OpenRaw {

	/** Private data for the thumbnail class */
	class Thumbnail::Private {
	public:
		Private()
			{
			}
		
		~Private()
			{
			}
	private:
		Private(const Private &);
		Private & operator=(const Private &);
	};

	Thumbnail::Thumbnail()
		: BitmapData(),
			d(new Thumbnail::Private())
	{
	}

	Thumbnail::~Thumbnail()
	{
		delete d;
	}

	Thumbnail *
	Thumbnail::getAndExtractThumbnail(const char* _filename,
																		uint32_t preferred_size, 
																		or_error & err)
	{
		err = OR_ERROR_NONE;
		Thumbnail *thumb = NULL;

		RawFile *file = RawFile::newRawFile(_filename);
		if (file) {
			thumb = new Thumbnail();
			err = file->getThumbnail(preferred_size, *thumb);
			delete file;
		}
		else {
			err = OR_ERROR_CANT_OPEN; // file error
		}
		return thumb;
	}


}
