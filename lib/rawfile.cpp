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
#include <cassert>
#include <map>
#include <string>

#include "debug.h"

#include "rawfile.h"
#include "cr2file.h"
#include "neffile.h"
#include "orffile.h"
#include "arwfile.h"
#include "peffile.h"
#include "crwfile.h"
#include "thumbnail.h"
#include "dngfile.h"

#include "rawfilefactory.h"

using std::string;
using namespace Debug;

namespace OpenRaw {

	using Internals::RawFileFactory;

	void init(void)
	{
 		static RawFileFactory fctcr2(OR_RAWFILE_TYPE_CR2, 
																 &Internals::CR2File::factory,
																 "cr2");
		static RawFileFactory fctnef(OR_RAWFILE_TYPE_NEF, 
																 &Internals::NEFFile::factory,
																 "nef");
		static RawFileFactory fctarw(OR_RAWFILE_TYPE_ARW, 
																 &Internals::ARWFile::factory,
																 "arw");
		static RawFileFactory fctorf(OR_RAWFILE_TYPE_ORF, 
																 &Internals::ORFFile::factory,
																 "orf");
		static RawFileFactory fctdng(OR_RAWFILE_TYPE_DNG, 
																 &Internals::DNGFile::factory,
																 "dng");
		static RawFileFactory fctpef(OR_RAWFILE_TYPE_PEF, 
																 &Internals::PEFFile::factory,
																 "pef");
		static RawFileFactory fctcrw(OR_RAWFILE_TYPE_CRW,
																 &Internals::CRWFile::factory,
																 "crw");																 
	}	

	class RawFile::Private 
	{
	public:
		Private(std::string f, Type t)
			: m_filename(f),
			m_type(t)
			{
			}
		
		/** the name of the file */
		std::string m_filename;
		/** the real type of the raw file */
		Type m_type;
	};



	RawFile *RawFile::newRawFile(const char*_filename, RawFile::Type _typeHint)
	{
		init();

		Type type;
		if (_typeHint == OR_RAWFILE_TYPE_UNKNOWN) {
			type = identify(_filename);
		}
		else {
			type = _typeHint;
		}
		Trace(DEBUG1) << "factory size " << RawFileFactory::table().size() << "\n";
		RawFileFactory::Table::iterator iter = RawFileFactory::table().find(type);
		if (iter == RawFileFactory::table().end()) {
			Trace(WARNING) << "factory not found\n";
			return NULL;
		}
		if (iter->second == NULL) {
			Trace(WARNING) << "factory is NULL\n";
			return NULL;
		}
		return (*(iter->second))(_filename);
	}


	RawFile::Type RawFile::identify(const char*_filename)
	{
		const char * extension = ::strrchr(_filename, '.') + 1;
		if (::strlen(extension) > 3) {
			return OR_RAWFILE_TYPE_UNKNOWN;
		}

		RawFileFactory::Extensions & extensions = RawFileFactory::extensions();
		RawFileFactory::Extensions::iterator iter 
			= extensions.find(string(extension));
		if (iter == extensions.end())
		{
			return OR_RAWFILE_TYPE_UNKNOWN;
		}
		return iter->second;
	}


	RawFile::RawFile(const char * _filename, RawFile::Type _type)
		: d(new Private(_filename, _type))
	{
		
	}


	RawFile::~RawFile()
	{
		delete d;
	}


	RawFile::Type RawFile::type() const
	{
		return d->m_type;
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


