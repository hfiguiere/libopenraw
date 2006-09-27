/*
 * libopenraw - orffile.cpp
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

#include "debug.h"
#include "orffile.h"
#include "thumbnail.h"
#include "ifd.h"
#include "ifddir.h"
#include "ifdentry.h"
#include "orfcontainer.h"
#include "iofile.h"

using namespace Debug;

namespace OpenRaw {

	namespace Internals {

		RawFile *ORFFile::factory(const char* _filename)
		{
			return new ORFFile(_filename);
		}


		ORFFile::ORFFile(const char* _filename)
			: RawFile(_filename, OR_RAWFILE_TYPE_ORF),
			  m_io(new IOFile(_filename)),
				m_container(new ORFContainer(m_io, 0))
		{
		}
		
		ORFFile::~ORFFile()
		{
			delete m_container;
			delete m_io;
		}

		bool ORFFile::_getSmallThumbnail(Thumbnail & thumbnail)
		{
			int c = m_container->countDirectories();
			if (c < 2) {
				return false;
			}
			IFDDir::Ref dir = m_container->setDirectory(1);

			bool success;

			long offset = 0;
			long size = 0;
			success = dir->getLongValue(IFD::EXIF_TAG_JPEG_INTERCHANGE_FORMAT, 
																	offset);
			success = dir->getLongValue(IFD::EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LENGTH,
																	size);

			void *buf = thumbnail.allocData(size);

			size_t real_size = m_container->fetchData(buf, offset, size);
			if (real_size != size) {
				Trace(WARNING) << "wrong size\n";
			}
			thumbnail.setDataType(OR_DATA_TYPE_JPEG);
			/* size has to be hardcoded, it does not seems to be 
			 * in the tags */
			thumbnail.setDimensions(160, 120);
			return true;
		}

		bool ORFFile::_getLargeThumbnail(Thumbnail & thumbnail)
		{
			return false;
		}

		bool ORFFile::_getPreview(Thumbnail & thumbnail)
		{
			return false;
		}
	}
}

