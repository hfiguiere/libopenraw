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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */


#include "debug.h"
#include "orfcontainer.h"


using namespace Debug;

namespace OpenRaw {

	namespace Internals {


		ORFContainer::ORFContainer(IO::Stream *_file, off_t offset)
			: IFDFileContainer(_file, offset)
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

				Trace(DEBUG1) << "Identified ORF file\n";

				return ENDIAN_LITTLE;
			}

			Trace(DEBUG1) << "Unidentified ORF file\n";

			return ENDIAN_NULL;
		}

	}
}

