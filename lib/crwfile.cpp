/*
 * libopenraw - crwfile.cpp
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

#include <algorithm>
#include <boost/bind.hpp>

#include <libopenraw/libopenraw.h>
#include <libopenraw++/thumbnail.h>
#include <libopenraw++/rawdata.h>

#include "debug.h"
#include "io/file.h"
#include "io/streamclone.h"
#include "crwfile.h"
#include "ciffcontainer.h"
#include "jfifcontainer.h"

#include "rawfilefactory.h"

using namespace Debug;

namespace OpenRaw {

	namespace Internals {

		using namespace CIFF;

		RawFile *CRWFile::factory(const char* _filename)
		{
			return new CRWFile(_filename);
		}

		CRWFile::CRWFile(const char* _filename)
			: RawFile(_filename, OR_RAWFILE_TYPE_CRW),
				m_io(new IO::File(_filename)),
				m_container(new CIFFContainer(m_io)),
				m_x(0), m_y(0)
		{

		}

		CRWFile::~CRWFile()
		{
			delete m_container;
			delete m_io;
		}

		::or_error CRWFile::_enumThumbnailSizes(std::vector<uint32_t> &list)
		{
			::or_error err = OR_ERROR_NOT_FOUND;

			Heap::Ref heap = m_container->heap();

			const RecordEntry::List & records = heap->records();
			RecordEntry::List::const_iterator iter;
			iter = std::find_if(records.begin(), records.end(), boost::bind(
														&RecordEntry::isA, _1, 
														static_cast<uint16_t>(TAG_JPEGIMAGE)));
			if (iter != records.end()) {
				Trace(DEBUG2) << "JPEG @" << (*iter).offset << "\n";
				m_x = m_y = 0;
				
				IO::StreamClone *s = new IO::StreamClone(m_io, heap->offset()
																								 + (*iter).offset);
				JFIFContainer *jfif = new JFIFContainer(s, 0);
				jfif->getDimensions(m_x, m_y);
				delete jfif;
				delete s;
				Trace(DEBUG1) << "JPEG dimensions x=" << m_x 
											<< " y=" << m_y << "\n";
				list.push_back(std::max(m_x,m_y));
				err = OR_ERROR_NONE;
			}

			return err;
		}

		::or_error CRWFile::_getThumbnail(uint32_t size, Thumbnail & thumbnail)
		{
			::or_error err = OR_ERROR_NOT_FOUND;
			Heap::Ref heap = m_container->heap();

			const RecordEntry::List & records = heap->records();
			RecordEntry::List::const_iterator iter;
			iter = std::find_if(records.begin(), records.end(), boost::bind(
														&RecordEntry::isA, _1, 
														static_cast<uint16_t>(TAG_JPEGIMAGE)));
			if (iter != records.end()) {
				Trace(DEBUG2) << "JPEG @" << (*iter).offset << "\n";
				size_t byte_size = (*iter).length;
				void *buf = thumbnail.allocData(byte_size);
				size_t real_size = (*iter).fetchData(heap.get(), buf, byte_size);
				if (real_size != byte_size) {
					Trace(WARNING) << "wrong size\n";
				}
				thumbnail.setDimensions(m_x, m_y);
				thumbnail.setDataType(OR_DATA_TYPE_JPEG);
				err = OR_ERROR_NONE;
			}

			return err;
		}

		::or_error CRWFile::_getRawData(RawData & data)
		{
			::or_error err = OR_ERROR_NOT_FOUND;
			Heap::Ref heap = m_container->heap();

			const RecordEntry::List & records = heap->records();
			RecordEntry::List::const_iterator iter;

			// locate the properties
			iter = std::find_if(records.begin(), records.end(), boost::bind(
														&RecordEntry::isA, _1, 
														static_cast<uint16_t>(TAG_IMAGEPROPS)));
			if (iter == records.end()) {
				Trace(ERROR) << "Couldn't find the image properties.\n";
				return err;
			}
			
			Heap props(iter->offset + heap->offset(), iter->length, m_container);
			const RecordEntry::List & propsRecs = props.records();
			iter = std::find_if(propsRecs.begin(), propsRecs.end(), boost::bind(
														&RecordEntry::isA, _1, 
														static_cast<uint16_t>(TAG_IMAGEINFO)));
			if (iter == propsRecs.end()) {
				Trace(ERROR) << "Couldn't find the image info.\n";
				return err;
			}
			ImageSpec img_spec;
			img_spec.readFrom(iter->offset + props.offset(), m_container);
			uint32_t x, y;
			x = img_spec.imageWidth;
			y = img_spec.imageHeight;

			// locate the RAW data
			iter = std::find_if(records.begin(), records.end(), boost::bind(
														&RecordEntry::isA, _1, 
														static_cast<uint16_t>(TAG_RAWIMAGEDATA)));
			if (iter != records.end()) {
				Trace(DEBUG2) << "RAW @" << (*iter).offset << "\n";
				size_t byte_size = (*iter).length;
				void *buf = data.allocData(byte_size);
				size_t real_size = (*iter).fetchData(heap.get(), buf, byte_size);
				if (real_size != byte_size) {
					Trace(WARNING) << "wrong size\n";
				}
				data.setDimensions(x, y);
				data.setDataType(OR_DATA_TYPE_COMPRESSED_CFA);
				err = OR_ERROR_NONE;
			}
			return err;
		}

	}
}
