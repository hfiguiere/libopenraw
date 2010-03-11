/*
 * libopenraw - orfcontainer.cpp
 *
 * Copyright (C) 2006, 2010 Hubert Figuiere
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


#include "trace.h"
#include "orfcontainer.h"


using namespace Debug;

namespace OpenRaw {

namespace Internals {


OrfContainer::OrfContainer(IO::Stream *_file, off_t offset)
	: IFDFileContainer(_file, offset)
	, subtype_(0)
{
}


OrfContainer::~OrfContainer()
{
}


IFDFileContainer::EndianType 
OrfContainer::isMagicHeader(const char *p, int len)
{			
	if (len < 4){
		// we need at least 4 bytes to check
		return ENDIAN_NULL;
	}
	if ((p[0] == 'I') && (p[1] == 'I')) {
		if((p[2] == 'R') && ((p[3] == 'O') || (p[3] == 'S'))) {

			Trace(DEBUG1) << "Identified EL ORF file. Subtype = " << p[3] << "\n";
			subtype_ = p[3];	
			return ENDIAN_LITTLE;
		}
	}
	else if((p[0] == 'M') && (p[1] == 'M')) {
		if((p[3] == 'R') && ((p[2] == 'O') || (p[2] == 'S'))) {

			Trace(DEBUG1) << "Identified BE ORF file. Subtype = " << p[2] << "\n";
			subtype_ = p[2];
			return ENDIAN_BIG;
		}
	}

	Trace(ERROR) << "Unidentified ORF file\n";

	return ENDIAN_NULL;
}

}
}

