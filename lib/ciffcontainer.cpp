/*
 * libopenraw - ciffcontainer.cpp
 *
 * Copyright (C) 2006-2008 Hubert Figuiere
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
#include <iostream>
#include <boost/shared_ptr.hpp>

#include <libopenraw/types.h>

#include "io/file.h"
#include "ciffcontainer.h"
#include "debug.h"

using namespace Debug;

namespace OpenRaw {
	namespace Internals {

		namespace CIFF {


			bool ImageSpec::readFrom(off_t offset, CIFFContainer *container)
			{
				bool ret;
				IO::Stream *file = container->file();
				file->seek(offset, SEEK_SET);
				ret = container->readUInt32(file, imageWidth);
				ret = container->readUInt32(file, imageHeight);
				ret = container->readUInt32(file, pixelAspectRatio);
				ret = container->readInt32(file, rotationAngle);
				ret = container->readUInt32(file, componentBitDepth);
				ret = container->readUInt32(file, colorBitDepth);
				ret = container->readUInt32(file, colorBW);
				return ret;
			}

			int32_t ImageSpec::exifOrientation() const
			{
				int32_t orientation = 0;
				switch(rotationAngle) {
				case 0:
					orientation = 1;
					break;
				case 90:
					orientation = 6;
					break;
				case 180:
					orientation = 3;
					break;
				case 270:
					orientation = 8;
					break;
				}
				return orientation;
			}

			RecordEntry::RecordEntry()
				: typeCode(0), length(0), offset(0)
			{
			}

			bool RecordEntry::readFrom(CIFFContainer *container)
			{
				bool ret;
				IO::Stream *file = container->file();
				ret = container->readUInt16(file, typeCode);
				ret = container->readUInt32(file, length);
				ret = container->readUInt32(file, offset);
				return ret;
			}

			size_t RecordEntry::fetchData(Heap* heap, void* buf, size_t size) const
			{
				return heap->container()->fetchData(buf, 
													offset + heap->offset(), size);
			}


			Heap::Heap(off_t start, off_t length, CIFFContainer * _container)
				: m_start(start),
					m_length(length),
					m_container(_container),
					m_records()
			{
				Debug::Trace(DEBUG2) << "Heap @ " << start << " length = "
														 << m_length << "\n";
			}

			std::vector<RecordEntry> & Heap::records()
			{
				if (m_records.size() == 0) {
					_loadRecords();
				}
				return m_records;
			}


			bool Heap::_loadRecords()
			{
				IO::Stream *file = m_container->file();
				file->seek(m_start + m_length - 4, SEEK_SET);
				int32_t record_offset;
				bool ret = m_container->readInt32(file, record_offset);
					
				if (ret) {
					int16_t numRecords;

					m_records.clear();
					file->seek(m_start + record_offset, SEEK_SET);
					ret = m_container->readInt16(file, numRecords);
					if (!ret) 
					{
						Trace(DEBUG1) << "read failed: " << ret << "\n";
					}
					Trace(DEBUG2) << "numRecords " << numRecords << "\n";
					int16_t i;
					m_records.reserve(numRecords);
					for (i = 0; i < numRecords; i++) {
						m_records.push_back(RecordEntry());
						m_records.back().readFrom(m_container);
					}
				}
				return ret;
			}


#if 0
			class OffsetTable {
				uint16_t numRecords;/* the number tblArray elements */
				RecordEntry tblArray[1];/* Array of the record entries */
			};
#endif


			bool HeapFileHeader::readFrom(CIFFContainer *container)
			{
				endian = RawContainer::ENDIAN_NULL;
				bool ret = false;
				IO::Stream *file = container->file();
				int s = file->read(byteOrder, 2);
				if (s == 2) {
					if((byteOrder[0] == 'I') && (byteOrder[1] == 'I')) {
						endian = RawContainer::ENDIAN_LITTLE;
					}
					else if((byteOrder[0] == 'M') && (byteOrder[1] == 'M')) {
						endian = RawContainer::ENDIAN_BIG;
					}
					container->setEndian(endian);
					ret = container->readUInt32(file, headerLength);
					if (ret) {
						ret = (file->read(type, 4) == 4);
					}
					if (ret) {
						ret = (file->read(subType, 4) == 4);
					}
					if (ret) {
						ret = container->readUInt32(file, version);
					}
				}
				return ret;
			}
		}
		
		CIFFContainer::CIFFContainer(IO::Stream *_file)
			: RawContainer(_file, 0),
			  m_hdr(),
			  m_heap((CIFF::Heap*)NULL),
			  m_hasImageSpec(false)
		{
			m_endian = _readHeader();
		}

		CIFFContainer::~CIFFContainer()
		{
		}

		CIFF::Heap::Ref CIFFContainer::heap()
		{
			if (m_heap == NULL) {
				_loadHeap();
			}
			return m_heap;
		}

		bool CIFFContainer::_loadHeap()
		{
			bool ret = false;
			if (m_heap == NULL) {
				if(m_endian != ENDIAN_NULL) {
					off_t heapLength = m_file->filesize() - m_hdr.headerLength;

					Trace(DEBUG1) << "heap len " << heapLength << "\n";
					m_heap = CIFF::Heap::Ref(new CIFF::Heap(m_hdr.headerLength, 
																									heapLength, this));
					
					ret = true;
				}
				else {
					Trace(DEBUG1) << "Unknown endian\n";
				}
			}
			return ret;
		}


		RawContainer::EndianType CIFFContainer::_readHeader()
		{
			EndianType _endian = ENDIAN_NULL;
			m_hdr.readFrom(this);
			if ((::strncmp(m_hdr.type, "HEAP", 4) == 0)
					&& (::strncmp(m_hdr.subType, "CCDR", 4) == 0)) {
				_endian = m_hdr.endian;
			}
			return _endian;
		}

		CIFF::Heap::Ref CIFFContainer::getImageProps()
		{
			if(!m_imageprops) {
				if(!heap()) {
					return CIFF::Heap::Ref();
				}
				
				const CIFF::RecordEntry::List & records = m_heap->records();
				CIFF::RecordEntry::List::const_iterator iter;
				
				// locate the properties
				iter = std::find_if(records.begin(), records.end(), boost::bind(
										&CIFF::RecordEntry::isA, _1, 
										static_cast<uint16_t>(CIFF::TAG_IMAGEPROPS)));
				if (iter == records.end()) {
					Trace(ERROR) << "Couldn't find the image properties.\n";
					return CIFF::Heap::Ref();
				}
				
				m_imageprops = CIFF::Heap::Ref(new CIFF::Heap(iter->offset + m_heap->offset(), iter->length, this));
			}
			return m_imageprops;
		}
		
		const CIFF::ImageSpec * CIFFContainer::getImageSpec()
		{
			if(!m_hasImageSpec) {
				CIFF::Heap::Ref props = getImageProps();

				const CIFF::RecordEntry::List & propsRecs = props->records();
				CIFF::RecordEntry::List::const_iterator iter;
				iter = std::find_if(propsRecs.begin(), propsRecs.end(), 
									boost::bind(
										&CIFF::RecordEntry::isA, _1, 
										static_cast<uint16_t>(CIFF::TAG_IMAGEINFO)));
				if (iter == propsRecs.end()) {
					Trace(ERROR) << "Couldn't find the image info.\n";
					return NULL;
				}
				m_imagespec.readFrom(iter->offset + props->offset(), this);
			}
			return &m_imagespec;
		}


		const CIFF::RecordEntry * CIFFContainer::getRawDataRecord() const
		{
			if(!m_heap) {
				return NULL;
			}
			const CIFF::RecordEntry::List & records = m_heap->records();
			CIFF::RecordEntry::List::const_iterator iter;
			// locate the RAW data
			iter = std::find_if(records.begin(), records.end(), boost::bind(
									&CIFF::RecordEntry::isA, _1, 
									static_cast<uint16_t>(CIFF::TAG_RAWIMAGEDATA)));
			
			if (iter != records.end()) {
				return &(*iter);
			}
			return NULL;
		}
	}
}
