/*
 * libopenraw - neffile.cpp
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

#include "debug.h"
#include "ifd.h"
#include "ifdfilecontainer.h"
#include "ifddir.h"
#include "ifdentry.h"
#include "iofile.h"
#include "neffile.h"
#include "thumbnail.h"

using namespace Debug;

namespace OpenRaw {


	namespace Internals {

		RawFile *NEFFile::factory(const char* _filename)
		{
			return new NEFFile(_filename);
		}

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
				Trace(WARNING) << "dir NULL\n";
				return false;
			}

			bool success;
			long offset = 0; 
			long size = 0;
			success = dir->getLongValue(IFD::EXIF_TAG_STRIP_OFFSETS, offset);
			success = dir->getLongValue(IFD::EXIF_TAG_STRIP_BYTE_COUNTS, size);

			void *buf = thumbnail.allocData(size);

			long x = 0;
			long y = 0;

			success = dir->getLongValue(IFD::EXIF_TAG_IMAGE_WIDTH, x);
			success = dir->getLongValue(IFD::EXIF_TAG_IMAGE_LENGTH, y);

			size_t real_size = m_container->fetchData(buf, offset, size);
			if (real_size != size) {
				Trace(WARNING) << "wrong size\n";
			}
			thumbnail.setDataType(OR_DATA_TYPE_PIXMAP_8RGB);
			thumbnail.setDimensions(x, y);
			return true;
		}

		bool NEFFile::_getLargeThumbnail(Thumbnail & thumbnail)
		{
			return false;
		}

		bool NEFFile::_getPreview(Thumbnail & thumbnail)
		{
			return false;
		}

	}
}

