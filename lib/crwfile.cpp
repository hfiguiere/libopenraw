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
#include "io/file.h"
#include "io/streamclone.h"
#include "thumbnail.h"
#include "crwfile.h"
#include "ciffcontainer.h"
#include "jfifcontainer.h"

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
				m_io(new IO::File(_filename)),
				m_container(new CIFFContainer(m_io)),
				m_x(0), m_y(0)
		{

		}

		CRWFile::~CRWFile()
		{
			delete m_container;
			delete m_io;
		}

		bool CRWFile::_enumThumbnailSizes(std::vector<uint32_t> &list)
		{
			Heap::Ref heap = m_container->heap();

			RecordEntry::List & records = heap->records();
			RecordEntry::List::iterator iter;
			for(iter = records.begin(); iter != records.end(); ++iter) {
				if ((*iter).typeCode == (TAGCODE_MASK & TAG_JPEGIMAGE)) {
					Trace(DEBUG2) << "JPEG @" << (*iter).offset << "\n";
					m_x = m_y = 0;

					IO::StreamClone *s = new IO::StreamClone(m_io, heap->offset()
																									 + (*iter).offset);
					JFIFContainer *jfif = new JFIFContainer(s, 0);
					jfif->getDimensions(m_x, m_y);
					delete jfif;
					delete s;
					Trace(DEBUG1) << "JPEG dimensions x=" << m_x 
															<< " y=" << m_y << "\n";
					list.push_back(std::max(m_x,m_y));
				}
			}

			return true;
		}

		bool CRWFile::_getThumbnail(uint32_t size, Thumbnail & thumbnail)
		{
			Heap::Ref heap = m_container->heap();

			RecordEntry::List & records = heap->records();

			RecordEntry::List::iterator iter;
			for(iter = records.begin(); iter != records.end(); ++iter) {
				if ((*iter).typeCode == (TAGCODE_MASK & TAG_JPEGIMAGE)) {
					Trace(DEBUG2) << "JPEG @" << (*iter).offset << "\n";
					size_t byte_size = (*iter).length;
					void *buf = thumbnail.allocData(byte_size);
					size_t real_size = (*iter).fetchData(heap.get(), buf, byte_size);
					if (real_size != byte_size) {
						Trace(WARNING) << "wrong size\n";
					}
					thumbnail.setDimensions(m_x, m_y);
					thumbnail.setDataType(OR_DATA_TYPE_JPEG);
				}
			}

			return true;
		}
	}
}
