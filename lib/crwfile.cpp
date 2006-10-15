/*
 * libopenraw - crwfile.cpp
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


#include <libopenraw/libopenraw.h>

#include "debug.h"
#include "iofile.h"
#include "thumbnail.h"
#include "crwfile.h"
#include "ciffcontainer.h"

#include "rawfilefactory.h"

using namespace Debug;

namespace OpenRaw {

	namespace Internals {

		using namespace CIFF;

		RawFile *CRWFile::factory(const char* _filename)
		{
			return new CRWFile(_filename);
		}

		CRWFile::CRWFile(const char* _filename)
			: RawFile(_filename, OR_RAWFILE_TYPE_CRW),
				m_io(new IOFile(_filename)),
				m_container(new CIFFContainer(m_io))
		{

		}

		CRWFile::~CRWFile()
		{
			delete m_container;
			delete m_io;
		}

		bool CRWFile::_getSmallThumbnail(Thumbnail & thumbnail)
		{
			return false;
		}


		bool CRWFile::_getLargeThumbnail(Thumbnail & thumbnail)
		{
			return false;
		}

		bool CRWFile::_getPreview(Thumbnail & thumbnail)
		{
			Heap::Ref heap = m_container->heap();

			RecordEntry::List & records = heap->records();

			RecordEntry::List::iterator iter;
			for(iter = records.begin(); iter != records.end(); ++iter) {
				if ((*iter).typeCode == (TAGCODE_MASK & TAG_JPEGIMAGE)) {
					Trace(DEBUG2) << "JPEG @" << (*iter).offset << "\n";
					size_t size = (*iter).length;
					void *buf = thumbnail.allocData(size);
					size_t real_size = (*iter).fetchData(heap.get(), buf, size);
					if (real_size != size) {
						Trace(WARNING) << "wrong size\n";
					}
					thumbnail.setDataType(OR_DATA_TYPE_JPEG);
				}
			}

			return true;
		}
	}
}
