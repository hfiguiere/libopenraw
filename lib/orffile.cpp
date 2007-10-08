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
			: IFDFile(_filename, OR_RAWFILE_TYPE_ORF)
		{
			// FIXME this is hackish... find a way to override that better.
			delete m_container;
			m_container = new ORFContainer(m_io, 0);
		}
		
		ORFFile::~ORFFile()
		{
		}

		::or_error ORFFile::_getRawData(RawData & data, uint32_t /*options*/)
		{
			IFDDir::Ref dir = m_container->setDirectory(0);
			return _getRawDataFromDir(data, dir);
		}

	}
}

