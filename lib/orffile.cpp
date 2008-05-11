/*
 * libopenraw - orffile.cpp
 *
 * Copyright (C) 2006, 2008 Hubert Figuiere
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

#include "debug.h"
#include "orffile.h"
#include "ifd.h"
#include "ifddir.h"
#include "ifdentry.h"
#include "orfcontainer.h"
#include "io/file.h"

using namespace Debug;

namespace OpenRaw {

	namespace Internals {

		RawFile *ORFFile::factory(IO::Stream *s)
		{
			return new ORFFile(s);
		}


		ORFFile::ORFFile(IO::Stream *s)
			: IFDFile(s, OR_RAWFILE_TYPE_ORF, false)
		{
			 m_container = new ORFContainer(m_io, 0);
		}
		
		ORFFile::~ORFFile()
		{
		}

		IFDDir::Ref  ORFFile::_locateCfaIfd()
		{
			// in PEF the CFA IFD is the main IFD
			if(!m_mainIfd) {
				m_mainIfd = _locateMainIfd();
			}
			return m_mainIfd;
		}


		IFDDir::Ref  ORFFile::_locateMainIfd()
		{
			return m_container->setDirectory(0);
		}

		static const struct camera_definition_t {
			const char * model;
			const uint32_t type_id;
		} s_def[] = {
			{ "E-1             ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_OLYMPUS, 
													   OR_TYPEID_OLYMPUS_E1) },
			{ "E-10        ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_OLYMPUS, 
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
			
			{ 0, 0 }
		};

		
		RawFile::TypeId ORFFile::_typeIdFromModel(const std::string & model)
		{
			// TODO optimise this as we can predict
			const struct camera_definition_t * p = s_def;
			while(p->model) {
				if(model == p->model) {
					break;
				}
				p++;
			}
			return p->type_id;
		}

		void ORFFile::_identifyId()
		{
			if(!m_mainIfd) {
				m_mainIfd = _locateMainIfd();
			}
			std::string model;
			if(m_mainIfd->getValue(IFD::EXIF_TAG_MODEL, model)) {
				_setTypeId(_typeIdFromModel(model));
			}
		}

		::or_error ORFFile::_getRawData(RawData & data, uint32_t /*options*/)
		{
			if(!m_cfaIfd) {
				m_cfaIfd = _locateCfaIfd();
			}
			return _getRawDataFromDir(data, m_cfaIfd);
		}

	}
}

