/*
 * libopenraw - cr2file.h
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




#ifndef __CR2FILE_H_
#define __CR2FILE_H_

#include "rawfile.h"

namespace OpenRaw {

	class Thumbnail;

	namespace Internals {
		class IOFile;
		class IFDFileContainer;

		class CR2File
			: public OpenRaw::RawFile
		{
		public:
			CR2File(const char* _filename);
			virtual ~CR2File();

			virtual bool getThumbnail(Thumbnail & thumbnail);

		private:

			CR2File(const CR2File&);
			CR2File & operator=(const CR2File&);

			/** get the small size thumbnail */
			bool _getSmallThumbnail(Thumbnail & thumbnail);

			IOFile *m_io; /**< the IO handle */
			IFDFileContainer *m_container; /**< the real container */
		};
	}

}

#endif
