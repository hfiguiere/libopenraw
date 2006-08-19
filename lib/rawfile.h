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


namespace OpenRaw {

	class RawFile
	{
	public:
		/** factory method */
		static RawFile *newRawFile(const char*_filename);

		/** destructor */
		virtual ~RawFile();
		
		// standard api, like get thumbnail
		// and get exif.
	protected:
		/** @param _filename the RAW file name */
		RawFile(const char *_filename);

	private:
		/** the name of the file */
		std::string m_filename;
		
	};
}


#endif
