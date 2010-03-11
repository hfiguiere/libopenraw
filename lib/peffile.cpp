/*
 * libopenraw - peffile.cpp
 *
 * Copyright (C) 2006-2008, 2010 Hubert Figuiere
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
#include "peffile.h"

using namespace Debug;

namespace OpenRaw {


	namespace Internals {
		const struct IFDFile::camera_ids_t PEFFile::s_def[] = {
			{ "PENTAX *ist D      ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX, 
														 OR_TYPEID_PENTAX_IST_D) },
			{ "PENTAX *ist DL     ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX, 
														 OR_TYPEID_PENTAX_IST_DL) },
			{ "PENTAX K10D        ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX, 
														 OR_TYPEID_PENTAX_K10D_PEF) },
			{ "PENTAX K100D       ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX, 
														 OR_TYPEID_PENTAX_K100D_PEF) },
			{ "PENTAX K100D Super ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX, 
														 OR_TYPEID_PENTAX_K100D_PEF) },
			{ "PENTAX K20D        ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_PENTAX, 
														 OR_TYPEID_PENTAX_K20D_PEF) },
			{ 0, 0 }
		};


		RawFile *PEFFile::factory(IO::Stream *s)
		{
			return new PEFFile(s);
		}

		PEFFile::PEFFile(IO::Stream *s)
			: IFDFile(s, OR_RAWFILE_TYPE_PEF)
		{
			_setIdMap(s_def);
		}


		PEFFile::~PEFFile()
		{
		}

		IFDDir::Ref  PEFFile::_locateCfaIfd()
		{
			// in PEF the CFA IFD is the main IFD
			if(!m_mainIfd) {
				m_mainIfd = _locateMainIfd();
			}
			return m_mainIfd;
		}


		IFDDir::Ref  PEFFile::_locateMainIfd()
		{
			return m_container->setDirectory(0);
		}

		::or_error PEFFile::_getRawData(RawData & data, uint32_t options)
		{
			::or_error err;
			if(!m_cfaIfd) {
				m_cfaIfd = _locateCfaIfd();
			}
			err = _getRawDataFromDir(data, m_cfaIfd);
			if(err == OR_ERROR_NONE) {
                uint16_t compression = data.compression();
                switch(compression) {
                case 65535:
                    if((options & OR_OPTIONS_DONT_DECOMPRESS) == 0) {
                        // TODO decompress
                    }
					break;
				default:
					break;
				}
			}
			return err;
		}
	}
}

