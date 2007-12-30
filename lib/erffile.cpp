/*
 * libopenraw - peffile.cpp
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
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
#include "erffile.h"

using namespace Debug;

namespace OpenRaw {


	namespace Internals {

		RawFile *ERFFile::factory(const char* _filename)
		{
			return new ERFFile(_filename);
		}

		ERFFile::ERFFile(const char* _filename)
			: IFDFile(_filename, OR_RAWFILE_TYPE_ERF)
		{
		}


		ERFFile::~ERFFile()
		{
		}

		::or_error ERFFile::_getRawData(RawData & data, uint32_t /*options*/)
		{
			::or_error err;
			IFDDir::Ref dir = m_container->setDirectory(0);

			std::vector<IFDDir::Ref> subdirs;
			if (!dir->getSubIFDs(subdirs)) {
				// error
				return OR_ERROR_NOT_FOUND;
			}
			IFDDir::RefVec::const_iterator i = find_if(subdirs.begin(), 
													   subdirs.end(),
													   IFDDir::isPrimary());
			if (i != subdirs.end()) {
				IFDDir::Ref subdir(*i);
				err = _getRawDataFromDir(data, subdir);
				if(err == OR_ERROR_NONE) {
					uint16_t compression = 0;
					subdir->getValue(IFD::EXIF_TAG_COMPRESSION, compression);
					switch(compression) {
					case 1:
						data.setDataType(OR_DATA_TYPE_CFA);
						break;
					case 32769:
						// TODO decompress. see nikon_load_raw() in dcraw
						Trace(DEBUG1) << "Epson compressed\n";
						break;
					default:
						break;
					}
				}
			}
			else {
				err = OR_ERROR_NOT_FOUND;
			}
			return err;
		}
	}
}

