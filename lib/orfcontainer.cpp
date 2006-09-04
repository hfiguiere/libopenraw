/*
 * libopenraw - orfcontainer.cpp
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <iostream>

#include "orfcontainer.h"


namespace OpenRaw {
	namespace Internals {


		ORFContainer::ORFContainer(IOFile *file, off_t offset)
			: IFDFileContainer(file, offset)
		{
		}


		ORFContainer::~ORFContainer()
		{
		}


		IFDFileContainer::EndianType 
		ORFContainer::isMagicHeader(const char *p, int len)
		{
			if (len < 4){
				// we need at least 4 bytes to check
				return ENDIAN_NULL;
			}
			if ((p[0] == 0x49) && (p[1] == 0x49)
					&& (p[2] == 0x52) && (p[3] == 0x4f)) {
				std::cerr << "Identified ORF file" << std::endl;
				return ENDIAN_LITTLE;
			}
			std::cerr << "Unidentified ORF file" << std::endl;
			return ENDIAN_NULL;
		}

	}
}

