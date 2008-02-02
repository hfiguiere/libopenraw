/*
 * libopenraw - rawfile.cpp
 *
 * Copyright (C) 2006-2007 Hubert Figuiere
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


#include <cstring>
#include <cassert>
#include <map>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <boost/checked_delete.hpp>

#include "debug.h"

#include <libopenraw/metadata.h>
#include <libopenraw++/rawfile.h>
#include <libopenraw++/thumbnail.h>

#include "cr2file.h"
#include "neffile.h"
#include "orffile.h"
#include "arwfile.h"
#include "peffile.h"
#include "crwfile.h"
#include "erffile.h"
#include "dngfile.h"
#include "mrwfile.h"
#include "metavalue.h"
#include "exception.h"

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
		static RawFileFactory fcterf(OR_RAWFILE_TYPE_ERF,
									 &Internals::ERFFile::factory,
									 "erf");
		static RawFileFactory fctmrw(OR_RAWFILE_TYPE_MRW,
									 &Internals::MRWFile::factory,
									 "mrw");
	}	

	class RawFile::Private 
	{
	public:
		Private(std::string f, Type t)
			: m_filename(f),
				m_type(t),
				m_sizes()
			{
			}
		~Private()
			{
				std::map<int32_t, MetaValue*>::iterator iter;
				for(iter = m_metadata.begin();
					iter != m_metadata.end(); ++iter)
				{
					if(iter->second) {
						delete iter->second;
					}
				}
			}
		/** the name of the file */
		std::string m_filename;
		/** the real type of the raw file */
		Type m_type;
		/** list of thumbnail sizes */
		std::vector<uint32_t> m_sizes;
		std::map<int32_t, MetaValue*> m_metadata;
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
		const char *e = ::strrchr(_filename, '.');
		if (e == NULL) {
			Trace(DEBUG1) << "Extension not found\n";
			return OR_RAWFILE_TYPE_UNKNOWN;
		}
		std::string extension(e + 1);
		if (extension.length() > 3) {
 			return OR_RAWFILE_TYPE_UNKNOWN;
		}

		boost::to_lower(extension);

		RawFileFactory::Extensions & extensions = RawFileFactory::extensions();
		RawFileFactory::Extensions::iterator iter = extensions.find(extension);
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

	const std::vector<uint32_t> & RawFile::listThumbnailSizes(void)
	{
		if (d->m_sizes.size() == 0) {
			Trace(DEBUG1) << "_enumThumbnailSizes init\n";
			bool ret = _enumThumbnailSizes(d->m_sizes);
			if (!ret) {
				Trace(DEBUG1) << "_enumThumbnailSizes failed\n";
			}
		}
		return d->m_sizes;
	}


	::or_error RawFile::getThumbnail(uint32_t tsize, Thumbnail & thumbnail)
	{
		::or_error ret = OR_ERROR_NOT_FOUND;
		uint32_t smallest_bigger = 0xffffffff;
		uint32_t biggest_smaller = 0;
		uint32_t found_size = 0;

		Trace(DEBUG1) << "requested size " << tsize << "\n";

		const std::vector<uint32_t> & sizes(listThumbnailSizes());

		std::vector<uint32_t>::const_iterator iter;

		for (iter = sizes.begin(); iter != sizes.end(); ++iter) {
			Trace(DEBUG1) << "current iter is " << *iter << "\n";
			if (*iter < tsize) {
				if (*iter > biggest_smaller) {
					biggest_smaller = *iter;
				}
			}
			else if(*iter > tsize) {
				if(*iter < smallest_bigger) {
					smallest_bigger = *iter;
				}
			}
			else { // *iter == tsize
				found_size = tsize;
				break;
			}
		}

		if (found_size == 0) {
			found_size = (smallest_bigger != 0xffffffff ? 
										smallest_bigger : biggest_smaller);
		}

		if (found_size != 0) {
			Trace(DEBUG1) << "size " << found_size << " found\n";
			ret = _getThumbnail(found_size, thumbnail);
		}
		else {
			// no size found, let's fail gracefuly
			Trace(DEBUG1) << "no size found\n";
 			ret = OR_ERROR_NOT_FOUND;
		}

		return ret;
	}


	::or_error RawFile::getRawData(RawData & rawdata, uint32_t options)
	{
		Trace(DEBUG1) << "getRawData()\n";
		::or_error ret = _getRawData(rawdata, options);
		return ret;
	}	


	int32_t RawFile::getOrientation()
	{
		int32_t idx = 0;
		const MetaValue * value = getMetaValue(META_NS_TIFF 
											   | EXIF_TAG_ORIENTATION);
		if(value == NULL) {
			return 0;
		}
		try {
			idx = value->getInteger();
		}
		catch(const Internals::BadTypeException & e)	{
			Trace(DEBUG1) << "wrong type - " << e.what() << "\n";
		}
		return idx;
	}
	
	const MetaValue *RawFile::getMetaValue(int32_t meta_index)
	{
		MetaValue *val = NULL;
		std::map<int32_t, MetaValue*>::iterator iter = d->m_metadata.find(meta_index);
		if(iter == d->m_metadata.end()) {
			val = _getMetaValue(meta_index);
			if(val != NULL) {
				d->m_metadata[meta_index] = val;
			}
		}
		else {
			val = iter->second;
		}
		return val;
	}

}


