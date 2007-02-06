/*
 * libopenraw - neffile.cpp
 *
 * Copyright (C) 2006-2007 Hubert Figuiere
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
#include <libopenraw++/thumbnail.h>
#include <libopenraw++/rawdata.h>

#include "debug.h"
#include "ifd.h"
#include "ifdfilecontainer.h"
#include "ifddir.h"
#include "ifdentry.h"
#include "io/file.h"
#include "neffile.h"

using namespace Debug;

namespace OpenRaw {


	namespace Internals {

		RawFile *NEFFile::factory(const char* _filename)
		{
			return new NEFFile(_filename);
		}

		NEFFile::NEFFile(const char* _filename)
			: IFDFile(_filename, OR_RAWFILE_TYPE_NEF)
		{
		}


		NEFFile::~NEFFile()
		{
		}

		::or_error NEFFile::_getRawData(RawData & data)
		{
			::or_error ret = OR_ERROR_NONE;
			IFDDir::Ref dir = m_container->setDirectory(0);

			Trace(DEBUG1) << "_getRawData()\n";

			std::vector<IFDDir::Ref> subdirs;
			if (!dir->getSubIFDs(subdirs)) {
				// error
				return OR_ERROR_NOT_FOUND;
			}
			
			IFDDir::RefVec::const_iterator i = find_if(subdirs.begin(), 
																								 subdirs.end(),
																								 IFDDir::isPrimary());
				
			if (i != subdirs.end()) {
				uint32_t offset = 0;
				uint32_t byte_length = 0;
				bool got_it;
				uint32_t x, y;
				x = 0;
				y = 0;
				IFDDir::Ref subdir = *i;
				got_it = subdir->getValue(IFD::EXIF_TAG_STRIP_OFFSETS, offset);
				if(!got_it) {
					Trace(DEBUG1) << "offset not found\n";
					return OR_ERROR_NOT_FOUND;
				}
				got_it = subdir->getValue(IFD::EXIF_TAG_STRIP_BYTE_COUNTS, byte_length);
				if(!got_it) {
					Trace(DEBUG1) << "byte len not found\n";
					return OR_ERROR_NOT_FOUND;
				}

				got_it = subdir->getValue(IFD::EXIF_TAG_IMAGE_WIDTH, x);
				if(!got_it) {
					Trace(DEBUG1) << "X not found\n";
					return OR_ERROR_NOT_FOUND;
				}
				got_it = subdir->getValue(IFD::EXIF_TAG_IMAGE_LENGTH, y);
				if(!got_it) {
					Trace(DEBUG1) << "Y not found\n";
					return OR_ERROR_NOT_FOUND;
				}
				
				void *p = data.allocData(byte_length);
				size_t real_size = m_container->fetchData(p, offset, 
																									byte_length);
				if (real_size < byte_length) {
					Trace(WARNING) << "Size mismatch for data: ignoring.\n";
				}
				data.setDataType(OR_DATA_TYPE_COMPRESSED_CFA);
				data.setDimensions(x, y);
			}
			else {
				ret = OR_ERROR_NOT_FOUND;
			}
			return ret;
		}

	}
}

