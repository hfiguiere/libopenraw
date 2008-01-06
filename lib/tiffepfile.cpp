/*
 * libopenraw - tiffepfile.cpp
 *
 * Copyright (C) 2007 Hubert Figuiere
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

#include <vector>

#include "tiffepfile.h"
#include "ifdfilecontainer.h"

namespace OpenRaw {
	namespace Internals {

		TiffEpFile::TiffEpFile(const char* _filename, Type _type)
			: IFDFile(_filename, _type)
		{
		}


		IFDDir::Ref  TiffEpFile::_locateCfaIfd()
		{
			if(!m_mainIfd) {
				m_mainIfd = _locateMainIfd();
			}

			std::vector<IFDDir::Ref> subdirs;
			if (!m_mainIfd || !m_mainIfd->getSubIFDs(subdirs)) {
				// error
				return IFDDir::Ref();
			}
			IFDDir::RefVec::const_iterator i = find_if(subdirs.begin(), 
													   subdirs.end(),
													   IFDDir::isPrimary());
			if (i != subdirs.end()) {
				return *i;
			}
			return IFDDir::Ref();
		}

		IFDDir::Ref  TiffEpFile::_locateMainIfd()
		{
			return m_container->setDirectory(0);
		}

	}
}
