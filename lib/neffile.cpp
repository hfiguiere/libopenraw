/*
 * libopenraw - neffile.h
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


#include <iostream>

#include "ifd.h"
#include "ifdfilecontainer.h"
#include "ifddir.h"
#include "ifdentry.h"
#include "iofile.h"
#include "neffile.h"
#include "thumbnail.h"


namespace OpenRaw {
	namespace Internals {

		NEFFile::NEFFile(const char* _filename)
			: RawFile(_filename, OR_RAWFILE_TYPE_NEF),
				m_io(new IOFile(_filename)),
				m_container(new IFDFileContainer(m_io, 0))
		{
		}


		NEFFile::~NEFFile()
		{
			delete m_container;
			delete m_io;
		}


		bool NEFFile::_getSmallThumbnail(Thumbnail & thumbnail)
		{
			int c = m_container->countDirectories();
			if (c < 1) {
				return false;
			}
			IFDDir::Ref dir = m_container->setDirectory(0);
			if (dir == NULL) {
				std::cerr << "dir NULL" << std::endl;
				return false;
			}

			IFDEntry::Ref e = dir->getEntry(IFD::EXIF_TAG_STRIP_OFFSETS);
			off_t offset = e->getLong();
			e = dir->getEntry(IFD::EXIF_TAG_STRIP_BYTE_COUNTS);
			size_t size = e->getLong();
			void *buf = thumbnail.allocData(size);

			int x, y;
			e = dir->getEntry(IFD::EXIF_TAG_IMAGE_WIDTH);
			x = e->getLong();
			e = dir->getEntry(IFD::EXIF_TAG_IMAGE_LENGTH);
			y = e->getLong();

			size_t real_size = m_container->fetchData(buf, offset, size);
			if (real_size != size) {
				std::cerr << "wrong size" << std::endl;
			}
			thumbnail.setDataType(OR_DATA_TYPE_PIXMAP_8RGB);
			thumbnail.setDimensions(x, y);


			return false;
		}
	}
}

