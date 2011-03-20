/* -*- tab-width:4; c-basic-offset:4 -*- */

/*
 * libopenraw - raffile.cpp
 *
 * Copyright (C) 2011 Hubert Figuiere
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


#include <libopenraw++/thumbnail.h>

#include "raffile.h"
#include "rafcontainer.h"
#include "jfifcontainer.h"

namespace OpenRaw {
namespace Internals {

const RawFile::camera_ids_t RafFile::s_def[] = {
	{ "FinePix X100" , OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_FUJIFILM,
											OR_TYPEID_FUJIFILM_X100) },
	{ "FinePix F700  ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_FUJIFILM,
										   OR_TYPEID_FUJIFILM_F700) }, 
	{ "FinePix F810   ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_FUJIFILM,
											OR_TYPEID_FUJIFILM_F810) }, 
	{ "FinePix E900   ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_FUJIFILM,
											 OR_TYPEID_FUJIFILM_E900) },
	{ "FinePixS2Pro", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_FUJIFILM,
											 OR_TYPEID_FUJIFILM_S2PRO) },
	{ "FinePix S3Pro  ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_FUJIFILM,
											 OR_TYPEID_FUJIFILM_S3PRO) },
	{ "FinePix S5Pro  ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_FUJIFILM,
											 OR_TYPEID_FUJIFILM_S5PRO) },
	{ "FinePix S5600  ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_FUJIFILM,
											 OR_TYPEID_FUJIFILM_S5600) },
	{ "FinePix S9500  ", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_FUJIFILM,
											 OR_TYPEID_FUJIFILM_S9500) },
	{ NULL, 0 }
};

RawFile *RafFile::factory(IO::Stream * s)
{
	return new RafFile(s);
}

RafFile::RafFile(IO::Stream * s)
	: RawFile(s, OR_RAWFILE_TYPE_RAF)
	, m_io(s)
	, m_container(new RafContainer(s))
{
	_setIdMap(s_def);
}

RafFile::~RafFile()
{
	delete m_container;
}

::or_error RafFile::_enumThumbnailSizes(std::vector<uint32_t> &list)
{
	or_error ret = OR_ERROR_NOT_FOUND;
	
	JFIFContainer * jpegPreview = m_container->getJpegPreview();
	uint32_t x, y;
	if(jpegPreview && jpegPreview->getDimensions(x, y)) {
		list.push_back(std::max(x, y));
		ret = OR_ERROR_NONE;
	}
	
	return ret;
}

::or_error RafFile::_getThumbnail(uint32_t /*size*/, Thumbnail & thumbnail)
{
	::or_error ret = OR_ERROR_NOT_FOUND;
	JFIFContainer * jpegPreview = m_container->getJpegPreview();
	uint32_t x, y;
	if(jpegPreview && jpegPreview->getDimensions(x, y)) {
		thumbnail.setDataType(OR_DATA_TYPE_JPEG);
		thumbnail.setDimensions(x, y);
		size_t byte_size = m_container->getJpegLength();
		void *buf = thumbnail.allocData(byte_size);
		m_container->fetchData(buf, m_container->getJpegOffset(), byte_size);
		ret = OR_ERROR_NONE;
	}
	return ret;
}

::or_error RafFile::_getRawData(RawData & data, uint32_t options)
{
	::or_error ret = OR_ERROR_NOT_FOUND;

#if 0
	IfdFileContainer * rawContainer = m_container->getCfaContainer();
	
	data.setDataType(OR_DATA_TYPE_CFA);
	deta.setDimensions();
	size_t byte_size = m_container->getCfaLength();
	void *buf = thumbnail.allocData(byte_size);
	m_container->fetchData(buf, m_container->getCfaOffset(), byte_size);	
	ret = OR_ERROR_NONE;
#endif
	
	return ret;
}

MetaValue *RafFile::_getMetaValue(int32_t /*meta_index*/)
{
	return NULL;
}

void RafFile::_identifyId()
{
	// get magic
	_setTypeId(_typeIdFromModel(m_container->getModel()));
}
	
}
}
