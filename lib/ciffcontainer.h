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
			/** tags for the CIFF records. 
			 * List made by a combination of the CIFF spec and
			 * what exifprobe by Duane H. Hesser has.
			 */
			enum {
				TAG_NULLRECORD =  0x0000,
				TAG_FREEBYTES = 0x0001,
				TAG_COLORINFO1 = 0x0032,
				TAG_FILEDESCRIPTION = 0x0805,
				TAG_RAWMAKEMODEL = 0x080a,
				TAG_FIRMWAREVERSION = 0x080b,
				TAG_COMPONENTVERSION = 0x080c,
				TAG_ROMOPERATIONMODE = 0x080d,
				TAG_OWNERNAME = 0x0810,
				TAG_IMAGETYPE = 0x0815,
				TAG_ORIGINALFILENAME = 0x0816,
				TAG_THUMBNAILFILENAME = 0x0817,

				TAG_TARGETIMAGETYPE = 0x100a,
				TAG_SHUTTERRELEASEMETHOD = 0x1010,
				TAG_SHUTTERRELEASETIMING = 0x1011,
				TAG_RELEASESETTING = 0x1016,
				TAG_BASEISO = 0x101c,
				TAG_FOCALLENGTH = 0x1029,  
				TAG_SHOTINFO = 0x102a,
				TAG_COLORINFO2 = 0x102c,
				TAG_CAMERASETTINGS = 0x102d,
				TAG_SENSORINFO = 0x1031,
				TAG_CUSTOMFUNCTIONS = 0x1033,
				TAG_PICTUREINFO = 0x1038,
				TAG_WHITEBALANCETABLE = 0x10a9,
				TAG_COLORSPACE = 0x10b4,  

				TAG_IMAGESPEC = 0x1803,
				TAG_RECORDID = 0x1804,
				TAG_SELFTIMERTIME = 0x1806,
				TAG_TARGETDISTANCESETTING = 0x1807,
				TAG_SERIALNUMBER = 0x180b,
				TAG_CAPTUREDTIME = 0x180e,
				TAG_IMAGEINFO = 0x1810,
				TAG_FLASHINFO = 0x1813,
				TAG_MEASUREDEV = 0x1814,
				TAG_FILENUMBER = 0x1817,
				TAG_EXPOSUREINFO = 0x1818,
				TAG_DECODERTABLE = 0x1835,

				TAG_RAWIMAGEDATA = 0x2005,
				TAG_JPEGIMAGE = 0x2007,
				TAG_JPEGTHUMBNAIL = 0x2008,

				TAG_IMAGEDESCRIPTION = 0x2804,
				TAG_CAMERAOBJECT = 0x2807,
				TAG_SHOOTINGRECORD = 0x3002,
				TAG_MEASUREDINFO = 0x3003,
				TAG_CAMERASPECIFICATION = 0x3004,
				TAG_IMAGEPROPS = 0x300a,
				TAG_EXIFINFORMATION = 0x300b
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
