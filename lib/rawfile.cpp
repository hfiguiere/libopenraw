/*
 * libopenraw - rawfile.cpp
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


#include <cstring>
#include <string>

#include "rawfile.h"
#include "cr2file.h"

using std::string;

namespace OpenRaw {
	
	RawFile *RawFile::newRawFile(const char*_filename, RawFile::Type _typeHint)
	{
		Type type;
		if (_typeHint == OR_RAWFILE_TYPE_UNKNOWN) {
			type = identify(_filename);
		}
		else {
			type = _typeHint;
		}
		switch(type)
		{
		case OR_RAWFILE_TYPE_CR2:
			return new Internals::CR2File(_filename);
			break;
		default: 
			break;
		}
		return NULL;
	}


	RawFile::Type RawFile::identify(const char*_filename)
	{
		string extension = ::strrchr(_filename, '.') + 1;
		if (extension.size() > 3) {
			return OR_RAWFILE_TYPE_UNKNOWN;
		}

		if (extension == "cr2") {
			return OR_RAWFILE_TYPE_CR2;
		}
		else if (extension == "crw") {
			return OR_RAWFILE_TYPE_CRW;
		}
		else if (extension == "nef") {
			return OR_RAWFILE_TYPE_NEF;
		}
		else if (extension == "mrw") {
			return OR_RAWFILE_TYPE_MRW;
		}
		else if (extension == "dng") {
			return OR_RAWFILE_TYPE_DNG;
		}
		return OR_RAWFILE_TYPE_UNKNOWN;
	}

	RawFile::RawFile(const char * _filename, RawFile::Type _type)
		: m_filename(_filename),
			m_type(_type)
	{
		
	}

	RawFile::~RawFile()
	{
	}

	RawFile::Type RawFile::type() const
	{
		return m_type;
	}


}


