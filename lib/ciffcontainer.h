/*
 * libopenraw - ciffcontainer.h
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

/**
 * @brief CIFF is the container for CRW files. It is an attempt from Canon to
 * make this a standard. I guess it failed.
 */


#ifndef _CIFFCONTAINER_H_
#define _CIFFCONTAINER_H_

#include <vector>
#include <boost/shared_ptr.hpp>

#include "rawcontainer.h"

namespace OpenRaw {
	namespace Internals {

		class IOFile;
		class CIFFContainer;

		namespace CIFF {
			
			/** mask for the typeCode */
			enum {
				STORAGELOC_MASK = 0xc000, /**< storage location bit mask */
				FORMAT_MASK = 0x3800,     /**< format of the data */
				TAGCODE_MASK = 0x3fff, /**< include the format, because the last
																* part is non significant */
			};
			/** tags for the CIFF records. */
			enum {
				TAG_JPEGIMAGE = 0x2007
			};
			
			class Heap;

			class RecordEntry 
			{
			public:
				typedef std::vector<RecordEntry> List;

				RecordEntry();

				/** load record from container 
				 * @param container the container
				 * @return true if success
				 */
				bool readFrom(CIFFContainer *container);
        /** fetch data define by the record from the heap
				 * @param heap the heap to load from
				 * @param buf the allocated buffer to load into
				 * @param size the size of the allocated buffer
				 * @return the size actually fetched. MIN(size, this->length);
				 */
				size_t fetchData(Heap* heap, void* buf, size_t size);

				uint16_t   typeCode;/* type code of the record */
				uint32_t   length;/* record length */
				uint32_t   offset;/* offset of the record in the heap*/
			};

			/** a CIFF Heap */
			class Heap
			{
			public:
				typedef boost::shared_ptr<Heap> Ref;

				Heap(off_t start, off_t length, CIFFContainer * container);

				RecordEntry::List & records();
				CIFFContainer *container()
					{
						return m_container;
					}
				off_t offset()
					{
						return m_start;
					}
			private:
				bool _loadRecords();

				off_t m_start;
				off_t m_length;
				CIFFContainer *m_container;
				RecordEntry::List m_records;
			};


			/** Heap Header of CIFF file*/
			class HeapFileHeader 
			{
			public:
				HeapFileHeader();
				

				bool readFrom(CIFFContainer *);
				char       byteOrder[2];/* 'MM' for Motorola,'II' for Intel */
				uint32_t   headerLength;/* length of header (in bytes) */
				char       type[4];
				char       subType[4];
				uint32_t   version; /* higher word: 0x0001, Lower word: 0x0002 */
				//uint32_t   reserved1;
				//uint32_t   reserved2;
				RawContainer::EndianType endian;				
			};
		}

		/** CIFF container
		 * as described by the CIFF documentation
		 */
		class CIFFContainer 
			: public RawContainer
		{
		public:
			CIFFContainer(IOFile *file);
			virtual ~CIFFContainer();

			CIFF::Heap::Ref heap();

			const CIFF::HeapFileHeader & header() const
				{
					return m_hdr;
				}
		private:
			bool _loadHeap();
			EndianType _readHeader();

			CIFFContainer(const CIFFContainer &);
			CIFFContainer & operator=(const CIFFContainer &);

			friend class CIFF::HeapFileHeader;
			CIFF::HeapFileHeader m_hdr;
			CIFF::Heap::Ref m_heap;
		};


	}
}



#endif
