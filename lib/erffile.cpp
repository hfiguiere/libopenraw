/*
 * libopenraw - erffile.cpp
 *
 * Copyright (C) 2006-2008 Hubert Figuiere
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

#include "trace.h"
#include "ifd.h"
#include "ifdfilecontainer.h"
#include "ifddir.h"
#include "ifdentry.h"
#include "io/file.h"
#include "erffile.h"

using namespace Debug;

namespace OpenRaw {


	namespace Internals {

		const IFDFile::camera_ids_t ERFFile::s_def[] = {
			{ "R-D1", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_EPSON,
										   OR_TYPEID_EPSON_RD1) },
			{ 0, 0 }
		};


		RawFile *ERFFile::factory(IO::Stream *s)
		{
			return new ERFFile(s);
		}

		ERFFile::ERFFile(IO::Stream *s)
			: TiffEpFile(s, OR_RAWFILE_TYPE_ERF)
		{
			_setIdMap(s_def);
		}


		ERFFile::~ERFFile()
		{
		}

		::or_error ERFFile::_getRawData(RawData & data, uint32_t /*options*/)
		{
			::or_error err;
			m_cfaIfd = _locateCfaIfd();
			if(m_cfaIfd) {
				err = _getRawDataFromDir(data, m_cfaIfd);
			}
			else {
				err = OR_ERROR_NOT_FOUND;
			}
			return err;
		}
	}
}

