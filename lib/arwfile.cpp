/*
 * libopenraw - arwfile.cpp
 *
 * Copyright (C) 2006 Hubert Figuiere
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


#include <libopenraw/libopenraw.h>
#include <libopenraw++/thumbnail.h>

#include "debug.h"
#include "io/file.h"
#include "ifdfilecontainer.h"
#include "ifd.h"
#include "arwfile.h"

using namespace Debug;

namespace OpenRaw {


	namespace Internals {

		RawFile *ARWFile::factory(IO::Stream * s)
		{
			return new ARWFile(s);
		}

		ARWFile::ARWFile(IO::Stream *s)
			: IFDFile(s, OR_RAWFILE_TYPE_ARW)
		{

		}

		ARWFile::~ARWFile()
		{
		}

		IFDDir::Ref  ARWFile::_locateCfaIfd()
		{
			// in ARW the CFA IFD is the main IFD
			if(!m_mainIfd) {
				m_mainIfd = _locateMainIfd();
			}
			return m_mainIfd;
		}


		IFDDir::Ref  ARWFile::_locateMainIfd()
		{
			return m_container->setDirectory(0);
		}

		::or_error ARWFile::_getRawData(RawData & /*data*/, uint32_t /*options*/) 
		{ 
			return OR_ERROR_NOT_FOUND; 
		}

	}
}
