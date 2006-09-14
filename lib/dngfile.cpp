/*
 * libopenraw - dngfile.cpp
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
#include "dngfile.h"

using namespace Debug;

namespace OpenRaw {


	namespace Internals {

		DNGFile::DNGFile(const char* _filename)
			: RawFile(_filename, OR_RAWFILE_TYPE_DNG),
				m_io(new IOFile(_filename)),
				m_container(new IFDFileContainer(m_io, 0))
		{

		}

		DNGFile::~DNGFile()
		{
			delete m_container;
			delete m_io;
		}

		/** does not have a small thumbnail */
		bool DNGFile::_getSmallThumbnail(Thumbnail & thumbnail)
		{
			return false;
		}


		/** DNG has only one RGB thumbnail in IFD 0 */
		bool DNGFile::_getLargeThumbnail(Thumbnail & thumbnail)
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
			success = dir->getLongValue(IFD::EXIF_TAG_STRIP_OFFSETS, offset);
			long size = 0;
			success = dir->getLongValue(IFD::EXIF_TAG_STRIP_BYTE_COUNTS, size);

			short x = 0;
			short y = 0;
			success = dir->getShortValue(IFD::EXIF_TAG_IMAGE_WIDTH, x);
			success = dir->getShortValue(IFD::EXIF_TAG_IMAGE_LENGTH, y);
			
			Trace(DEBUG1) << "x, y " << x << " " << y << "\n";
			void *buf = thumbnail.allocData(size);

			size_t real_size = m_container->fetchData(buf, offset, size);
			if (real_size != size) {
				Trace(WARNING) << "wrong size\n";
			}
			thumbnail.setDataType(OR_DATA_TYPE_PIXMAP_8RGB);
			thumbnail.setDimensions(x, y);
			return true;
		}
	}
}
