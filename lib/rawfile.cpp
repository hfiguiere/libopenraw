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
#include "neffile.h"
#include "orffile.h"
#include "arwfile.h"
#include "peffile.h"
#include "thumbnail.h"
#include "dngfile.h"

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
		case OR_RAWFILE_TYPE_NEF:
			return new Internals::NEFFile(_filename);
			break;
		case OR_RAWFILE_TYPE_ARW:
			return new Internals::ARWFile(_filename);
			break;
		case OR_RAWFILE_TYPE_ORF:
			return new Internals::ORFFile(_filename);
			break;
		case OR_RAWFILE_TYPE_DNG:
			return new Internals::DNGFile(_filename);
			break;
		case OR_RAWFILE_TYPE_PEF:
			return new Internals::PEFFile(_filename);
			break;
		default:
			break;
		}
		return NULL;
	}


	RawFile::Type RawFile::identify(const char*_filename)
	{
		const char * extension = ::strrchr(_filename, '.') + 1;
		if (::strlen(extension) > 3) {
			return OR_RAWFILE_TYPE_UNKNOWN;
		}

		if (::strcasecmp(extension, "cr2") == 0) {
			return OR_RAWFILE_TYPE_CR2;
		}
		else if (::strcasecmp(extension, "crw") == 0) {
			return OR_RAWFILE_TYPE_CRW;
		}
		else if (::strcasecmp(extension, "nef") == 0) {
			return OR_RAWFILE_TYPE_NEF;
		}
		else if (::strcasecmp(extension, "mrw") == 0) {
			return OR_RAWFILE_TYPE_MRW;
		}
		else if (::strcasecmp(extension, "arw") == 0) {
			return OR_RAWFILE_TYPE_ARW;
		}
		else if (::strcasecmp(extension, "dng") == 0) {
			return OR_RAWFILE_TYPE_DNG;
		}
		else if (::strcasecmp(extension, "orf") == 0) {
			return OR_RAWFILE_TYPE_ORF;
		}
		else if (::strcasecmp(extension, "pef") == 0) {
			return OR_RAWFILE_TYPE_PEF;
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


	bool RawFile::getThumbnail(Thumbnail & thumbnail)
	{
		bool ret = false;
		Thumbnail::Size tsize = thumbnail.thumbSize();
		switch (tsize)
		{
		case OR_THUMB_SIZE_SMALL:
			ret = _getSmallThumbnail(thumbnail);
			break;
		case OR_THUMB_SIZE_LARGE:
			ret = _getLargeThumbnail(thumbnail);
			break;
		case OR_THUMB_SIZE_PREVIEW:
			ret = _getPreview(thumbnail);
			break;
		default:
			break;
		}
		return ret;
	}

}


