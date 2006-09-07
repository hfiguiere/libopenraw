/*
 * libopenraw - cr2file.cpp
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
#include "ifdfilecontainer.h"
#include "ifd.h"
#include "thumbnail.h"
#include "cr2file.h"


namespace OpenRaw {

	using Debug::Trace;

	namespace Internals {

		CR2File::CR2File(const char* _filename)
			: RawFile(_filename, OR_RAWFILE_TYPE_CR2),
				m_io(new IOFile(_filename)),
				m_container(new IFDFileContainer(m_io, 0))
		{

		}

		CR2File::~CR2File()
		{
			delete m_container;
			delete m_io;
		}

		bool CR2File::_getSmallThumbnail(Thumbnail & thumbnail)
		{
			int c = m_container->countDirectories();
			if (c < 2) {
				return false;
			}
			IFDDir::Ref dir = m_container->setDirectory(1);
			if (dir == NULL) {
				Trace(Debug::WARNING) << "dir NULL\n";
				return false;
			}
			IFDEntry::Ref e = dir->getEntry(IFD::EXIF_TAG_JPEG_INTERCHANGE_FORMAT);
			if (e == NULL) {
				Trace(Debug::WARNING) << "EXIF_TAG_JPEG_INTERCHANGE_FORMAT NULL\n";
				return false;
			}
			off_t offset = e->getLong();
			e = dir->getEntry(IFD::EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LENGTH);
			if (e == NULL) {

				Trace(Debug::WARNING) << "EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LENGTH NULL\n";

				return false;
			}
			size_t size = e->getLong();
			void *buf = thumbnail.allocData(size);

			size_t real_size = m_container->fetchData(buf, offset, size);
			if (real_size != size) {
				Trace(Debug::WARNING) << "wrong size\n";
			}
			thumbnail.setDataType(OR_DATA_TYPE_JPEG);
			thumbnail.setDimensions(160, 120);
			return true;
		}
	}
}
