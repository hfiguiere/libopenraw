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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
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

		RawFile *ORFFile::factory(const char* _filename)
		{
			return new ORFFile(_filename);
		}


		ORFFile::ORFFile(const char* _filename)
			: IFDFile(_filename, OR_RAWFILE_TYPE_ORF, false)
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

		::or_error ORFFile::_getRawData(RawData & data, uint32_t /*options*/)
		{
			if(!m_cfaIfd) {
				m_cfaIfd = _locateCfaIfd();
			}
			return _getRawDataFromDir(data, m_cfaIfd);
		}

	}
}

