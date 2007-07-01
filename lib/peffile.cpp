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
#include "peffile.h"

using namespace Debug;

namespace OpenRaw {


	namespace Internals {

		RawFile *PEFFile::factory(const char* _filename)
		{
			return new PEFFile(_filename);
		}

		PEFFile::PEFFile(const char* _filename)
			: IFDFile(_filename, OR_RAWFILE_TYPE_PEF)
		{
		}


		PEFFile::~PEFFile()
		{
		}

		::or_error PEFFile::_getRawData(RawData & data)
		{
			::or_error err;
			IFDDir::Ref dir = m_container->setDirectory(0);
			err = _getRawDataFromDir(data, dir);
			if(err == OR_ERROR_NONE) {
				uint16_t compression = 0;
				dir->getValue(IFD::EXIF_TAG_COMPRESSION, compression);
				switch(compression) {
				case 1:
					data.setDataType(OR_DATA_TYPE_CFA);
					break;
				case 65535:
					// TODO decompress
					break;
				default:
					break;
				}
			}
			return err;
		}
	}
}

