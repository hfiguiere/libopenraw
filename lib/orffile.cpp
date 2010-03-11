/*
 * libopenraw - orffile.cpp
 *
 * Copyright (C) 2006, 2008, 2010 Hubert Figuiere
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

#include <libopenraw++/thumbnail.h>
#include <libopenraw++/rawdata.h>

#include "trace.h"
#include "orffile.h"
#include "ifd.h"
#include "ifddir.h"
#include "ifdentry.h"
#include "orfcontainer.h"
#include "io/file.h"

using namespace Debug;

namespace OpenRaw {
namespace Internals {

		const struct IFDFile::camera_ids_t OrfFile::s_def[] = {
			{ "E-1             ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_OLYMPUS, 
													   OR_TYPEID_OLYMPUS_E1) },
			{ "E-10        "    , OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_OLYMPUS, 
												  OR_TYPEID_OLYMPUS_E10) },
			{ "E-3             ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_OLYMPUS, 
													  OR_TYPEID_OLYMPUS_E3) },
			{ "E-300           ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_OLYMPUS, 
													  OR_TYPEID_OLYMPUS_E300) },
			{ "E-330           ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_OLYMPUS, 
													  OR_TYPEID_OLYMPUS_E330) },
			{ "E-400           ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_OLYMPUS, 
													  OR_TYPEID_OLYMPUS_E400) },
			{ "E-410           ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_OLYMPUS, 
													  OR_TYPEID_OLYMPUS_E410) },
			{ "E-500           ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_OLYMPUS, 
													  OR_TYPEID_OLYMPUS_E500) },
			{ "E-510           ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_OLYMPUS, 
													  OR_TYPEID_OLYMPUS_E510) },
			{ "SP350"           , OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_OLYMPUS, 
													  OR_TYPEID_OLYMPUS_SP350) },
			{ "SP500UZ"         , OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_OLYMPUS, 
													  OR_TYPEID_OLYMPUS_SP500) },
			{ "SP510UZ"         , OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_OLYMPUS, 
													  OR_TYPEID_OLYMPUS_SP510) },
			{ "SP550UZ                ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_OLYMPUS, 
													  OR_TYPEID_OLYMPUS_SP550) },
		    { "E-P1            ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_OLYMPUS, 
													  OR_TYPEID_OLYMPUS_EP1) },
		    { "E-620           ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_OLYMPUS, 
													  OR_TYPEID_OLYMPUS_E620) },
			{ 0, 0 }
		};

		RawFile *OrfFile::factory(IO::Stream *s)
		{
			return new OrfFile(s);
		}


		OrfFile::OrfFile(IO::Stream *s)
			: IFDFile(s, OR_RAWFILE_TYPE_ORF, false)
		{
			_setIdMap(s_def);
			m_container = new OrfContainer(m_io, 0);
		}
		
		OrfFile::~OrfFile()
		{
		}

		IFDDir::Ref  OrfFile::_locateCfaIfd()
		{
			// in ORF the CFA IFD is the main IFD
			if(!m_mainIfd) {
				m_mainIfd = _locateMainIfd();
			}
			return m_mainIfd;
		}


		IFDDir::Ref  OrfFile::_locateMainIfd()
		{
			return m_container->setDirectory(0);
		}


		
		::or_error OrfFile::_getRawData(RawData & data, uint32_t options)
		{
			::or_error err;
			if(!m_cfaIfd) {
				m_cfaIfd = _locateCfaIfd();
			}
			err = _getRawDataFromDir(data, m_cfaIfd);
			if(err == OR_ERROR_NONE) {
				// ORF files seems to be marked as uncompressed even if they are.
				uint32_t x = data.x();
				uint32_t y = data.y();
				uint16_t compression = 0;
				if(data.size() < x * y * 2) {
                    compression = 65535;
                    data.setCompression(65535);
					data.setDataType(OR_DATA_TYPE_COMPRESSED_CFA);
				}
                else {
                    compression = data.compression();
                }
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

