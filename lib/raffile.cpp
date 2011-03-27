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


#include <boost/scoped_array.hpp>
#include <libopenraw++/thumbnail.h>
#include <libopenraw++/rawdata.h>

#include "raffile.h"
#include "rafcontainer.h"
#include "rafmetacontainer.h"
#include "jfifcontainer.h"
#include "unpack.h"
#include "trace.h"


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
	{ "FinePix S6500fd", OR_MAKE_FILE_TYPEID(OR_TYPEID_VENDOR_FUJIFILM,
											 OR_TYPEID_FUJIFILM_S6500FD) },
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

	RafMetaContainer * meta = m_container->getMetaContainer();

	RafMetaValue::Ref value = meta->getValue(RAF_TAG_SENSOR_DIMENSION);
	uint32_t dims = value->get().getInteger();
	uint16_t h = (dims & 0xFFFF0000) >> 16;
	uint16_t w = (dims & 0x0000FFFF);

	value = meta->getValue(RAF_TAG_RAW_INFO);
	uint32_t rawProps = value->get().getInteger();
	uint8_t layout = (rawProps & 0xFF000000) >> 24 >> 7; // MSBit in byte.
	uint8_t compressed = ((rawProps & 0xFF0000) >> 16) & 8; // 8 == compressed
	
	//printf("layout %x - compressed %x\n", layout, compressed);
	
	data.setDataType(OR_DATA_TYPE_CFA);
	data.setDimensions(w,h);
	// TODO get the right pattern.
	data.setCfaPattern(OR_CFA_PATTERN_GBRG);
	// TODO actually read the 2048.
	// TODO make sure this work for the other file formats...
	size_t byte_size = m_container->getCfaLength() - 2048;
	size_t fetched = 0;
	off_t offset = m_container->getCfaOffset() + 2048;
	
	bool is_compressed = (compressed == 8);
	uint32_t finaldatalen = 2 * h * w;
	uint32_t datalen =	(is_compressed ? byte_size : finaldatalen);
	void *buf = data.allocData(finaldatalen);

	if(is_compressed)
	{
		Unpack unpack(w, IFD::COMPRESS_NONE);
		size_t blocksize = unpack.block_size();
		boost::scoped_array<uint8_t> block(new uint8_t[blocksize]);
		uint8_t * outdata = (uint8_t*)data.data();
		size_t outsize = finaldatalen;
		size_t got;
		do {
			Debug::Trace(DEBUG2) << "fatchData @offset " << offset << "\n";
			got = m_container->fetchData (block.get(), 
										  offset, blocksize);
			fetched += got;
			offset += got;
			Debug::Trace(DEBUG2) << "got " << got << "\n";
			if(got) {
				size_t out;
				or_error err = unpack.unpack_be12to16(outdata, outsize,
													  block.get(), got, out);
				outdata += out;
				outsize -= out;
				Debug::Trace(DEBUG2) << "unpacked " << out
				<< " bytes from " << got << "\n";
				if(err != OR_ERROR_NONE) {
					ret = err;
					break;
				}
			}
		} while((got != 0) && (fetched < datalen));
	}
	else
	{
		m_container->fetchData (buf, offset, datalen);
	}

	ret = OR_ERROR_NONE;

	return ret;
}

MetaValue *RafFile::_getMetaValue(int32_t meta_index)
{
	if(META_INDEX_MASKOUT(meta_index) == META_NS_EXIF
	   || META_INDEX_MASKOUT(meta_index) == META_NS_TIFF) {
		
		JFIFContainer * jpegPreview = m_container->getJpegPreview();
		IfdDir::Ref dir = jpegPreview->exifIfd();
		IfdEntry::Ref e = dir->getEntry(META_NS_MASKOUT(meta_index));
		if(e) {
			return new MetaValue(e);
		}
	}
	
	return NULL;
}

void RafFile::_identifyId()
{
	_setTypeId(_typeIdFromModel(m_container->getModel()));
}
	
}
}
