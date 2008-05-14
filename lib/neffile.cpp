/*
 * libopenraw - neffile.cpp
 *
 * Copyright (C) 2006-2008 Hubert Figuiere
 * Copyright (C) 2008 Novell, Inc.
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
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
		const IFDFile::camera_ids_t NEFFile::s_def[] = {
			{ "NIKON D1 ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NIKON, 
											   OR_TYPEID_NIKON_D1) },
			{ "NIKON D100 ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NIKON, 
											   OR_TYPEID_NIKON_D100) },
			{ "NIKON D1X", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NIKON, 
											   OR_TYPEID_NIKON_D1X) },
			{ "NIKON D200", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NIKON, 
												OR_TYPEID_NIKON_D200) },
			{ "NIKON D2H", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NIKON, 
											   OR_TYPEID_NIKON_D2H ) },
			{ "NIKON D2X", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NIKON, 
											   OR_TYPEID_NIKON_D2X ) },
			{ "NIKON D3", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NIKON, 
											  OR_TYPEID_NIKON_D3) },
			{ "NIKON D300", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NIKON, 
												OR_TYPEID_NIKON_D300) },
			{ "NIKON D40", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NIKON, 
											   OR_TYPEID_NIKON_D40) },
			{ "NIKON D40X", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NIKON, 
											   OR_TYPEID_NIKON_D40X) },
			{ "NIKON D50", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NIKON, 
											   OR_TYPEID_NIKON_D50) },
			{ "NIKON D70", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NIKON, 
											   OR_TYPEID_NIKON_D70) },
			{ "NIKON D70s", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NIKON, 
											   OR_TYPEID_NIKON_D70S) },
			{ "NIKON D80", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_NIKON, 
											   OR_TYPEID_NIKON_D80) },
			{ 0, 0 }
		};

		RawFile *NEFFile::factory(IO::Stream* _filename)
		{
			return new NEFFile(_filename);
		}

		NEFFile::NEFFile(IO::Stream* _filename)
			: TiffEpFile(_filename, OR_RAWFILE_TYPE_NEF)
		{
			_setIdMap(s_def);
		}


		NEFFile::~NEFFile()
		{
		}

		bool NEFFile::isCompressed(RawContainer & container, uint32_t offset)
		{
			int i;
			uint8_t buf[256];
			size_t real_size = container.fetchData(buf, offset, 
												   256);
			if(real_size != 256) {
				return true;
			}
			for(i = 15; i < 256; i+= 16) {
				if(buf[i]) {
					Trace(DEBUG1) << "isCompressed: true\n";
					return true;
				}
			}
			Trace(DEBUG1) << "isCompressed: false\n";
			return false;
		}

		::or_error NEFFile::_getRawData(RawData & data, uint32_t /*options*/)
		{
			::or_error ret = OR_ERROR_NONE;
			m_cfaIfd = _locateCfaIfd();
			Trace(DEBUG1) << "_getRawData()\n";

			if(m_cfaIfd) {
				ret = _getRawDataFromDir(data, m_cfaIfd);
			}
			else {
				ret = OR_ERROR_NOT_FOUND;
			}
			return ret;
		}

	}
}

