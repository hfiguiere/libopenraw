/*
 * libopenraw - ifdfile.h
 *
 * Copyright (C) 2006 Hubert Figuiere
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



#ifndef _IFD_FILE_H_
#define _IFD_FILE_H_

#include <vector>
#include <libopenraw/types.h>
#include <libopenraw/consts.h>
#include <libopenraw++/rawfile.h>

#include "ifddir.h"

namespace OpenRaw {
	namespace IO {
		class Stream;
		class File;
	}

	namespace Internals {
		class IFFileContainer;

		/** describe the location of a thumbnail in an IFD file */
		struct IFDThumbDesc
		{
			IFDThumbDesc(uint32_t _x, uint32_t _y, ::or_data_type _type,
									 const IFDDir::Ref & _ifddir)
				: x(_x), y(_y), type(_type), ifddir(_ifddir)
				{
				}
			IFDThumbDesc()
				: x(0), y(0), type(OR_DATA_TYPE_NONE), ifddir((IFDDir*)NULL)
				{
				}
			uint32_t x;    /**< x size. Can be 0 */
			uint32_t y;    /**< y size. Can be 0 */
			::or_data_type type; /**< the data type format */
			IFDDir::Ref ifddir; /**< the IFD directory */
		};


		/** @brief generic IFD based raw file. */
		class IFDFile
			: public OpenRaw::RawFile
		{

		protected:
			IFDFile(const char *_filename, Type _type);
			virtual ~IFDFile();

			/** list the thumbnails in the IFD
			 * @retval list the list of thumbnails
			 * @return the error code. OR_ERROR_NOT_FOUND if no
			 * thumbnail are found. 
			 */
			virtual ::or_error _enumThumbnailSizes(std::vector<uint32_t> &list);

			/** locate the thumnaile in the IFD 
			 * @param dir the IFDDir where to locate the thumbnail
			 * @return the error code. OR_ERROR_NOT_FOUND if the
			 * thumbnail are not found.
			 */
			virtual ::or_error _locateThumbnail(const IFDDir::Ref & dir,
																		std::vector<uint32_t> &list);

			typedef std::map<uint32_t, IFDThumbDesc> ThumbLocations;
			ThumbLocations    m_thumbLocations;
			IO::Stream       *m_io; /**< the IO handle */
			IFDFileContainer *m_container; /**< the real container */

		private:

			IFDFile(const IFDFile&);
			IFDFile & operator=(const IFDFile &);

			virtual ::or_error _getThumbnail(uint32_t size, Thumbnail & thumbnail);
		};

	}
}


#endif
