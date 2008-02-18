/*
 * libopenraw - crwfile.h
 *
 * Copyright (C) 2006-2008 Hubert Figuiere
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




#ifndef __CRWFILE_H_
#define __CRWFILE_H_

#include <libopenraw++/rawfile.h>

#include "rawfilefactory.h"

namespace OpenRaw {

	class Thumbnail;

	namespace IO {
		class File;
	}

	namespace Internals {
		class CIFFContainer;

		class CRWFile
			: public OpenRaw::RawFile
		{
		public:
			static RawFile *factory(const char* _filename);
			CRWFile(const char* _filename);
			virtual ~CRWFile();

		protected:


			virtual ::or_error _enumThumbnailSizes(std::vector<uint32_t> &list);

			virtual ::or_error _getThumbnail(uint32_t size, Thumbnail & thumbnail);

			virtual ::or_error _getRawData(RawData & data, uint32_t options);
			virtual MetaValue *_getMetaValue(int32_t meta_index);
		private:
			CRWFile(const CRWFile&);
			CRWFile & operator=(const CRWFile&);

			IO::File *m_io; /**< the IO handle */
			CIFFContainer *m_container; /**< the real container */
			uint32_t m_x;
			uint32_t m_y;
		};
	}

}

#endif
