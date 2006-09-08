/*
 * libopenraw - rawfile.h
 *
 * Copyright (C) 2005-2006 Hubert Figuiere
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



#ifndef __RAWFILE_H
#define __RAWFILE_H

#include <string>

#include <libopenraw/libopenraw.h>

namespace OpenRaw {

	class Thumbnail;

	class RawFile
	{
	public:
		typedef ::or_rawfile_type Type;

		/** factory method to create the proper RawFile instance.
		 * @param _filename the name of the file to load
		 * @param _typeHint a hint on the type. Use UNKNOWN_TYPE
		 * if you want to let the library detect it for you.
		 */
		static RawFile *newRawFile(const char*_filename, 
															 Type _typeHint = OR_RAWFILE_TYPE_UNKNOWN);

		/** Destructor */
		virtual ~RawFile();
		/** Accessor for the type */
		Type type() const;

		// standard api, like get thumbnail
		// and get exif.

		/** Get the thumbnail from the raw file 
				@param thumbnail the thumbnail to extract into
		 */
		bool getThumbnail(Thumbnail & thumbnail);
	protected:
		/** 
		 * Construct a raw file
		 * @param _filename the RAW file name 
		 * @param _type the type
		 */
		RawFile(const char *_filename, Type _type);

		/** get the small size thumbnail */
		virtual bool _getSmallThumbnail(Thumbnail & thumbnail) = 0;
		/** get the large thumbnail */
		virtual bool _getLargeThumbnail(Thumbnail & thumbnail) = 0;
	private:
		static Type identify(const char*_filename);

		/** the name of the file */
		std::string m_filename;
		/** the real type of the raw file */
		Type m_type;
	};
}


#endif
