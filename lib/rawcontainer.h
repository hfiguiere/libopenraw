/*
 * libopenraw - rawcontainer.h
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




#ifndef _RAWCONTAINER_H_
#define _RAWCONTAINER_H_

#include <sys/types.h>

#include <libopenraw/io.h>
#include <libopenraw/types.h>


namespace OpenRaw {
	namespace IO {
		class Stream;
	}

	namespace Internals {

		
/**
   Generic interface for the RAW file container
 */
		class RawContainer
		{
		public:
			/** define the endian of the container */
			typedef enum {
				ENDIAN_NULL = 0, /** no endian found: means invalid file */
				ENDIAN_BIG,      /** big endian found */
				ENDIAN_LITTLE    /** little endian found */
			} EndianType;

			/** 
					@param file the stream to read from
					@param offset the offset since starting the 
					beginning of the file for the container
			*/
			RawContainer(IO::Stream *_file, off_t offset);
			/** destructor */
			virtual ~RawContainer();
			
			IO::Stream *file()
				{
					return m_file;
				}
			EndianType endian() const
				{
					return m_endian;
				}

			bool readInt8(IO::Stream *f, int8_t & v);
			bool readUInt8(IO::Stream *f, uint8_t & v);
			/** Read an int16 following the m_endian set */
			bool readInt16(IO::Stream *f, int16_t & v);
			/** Read an int32 following the m_endian set */
			bool readInt32(IO::Stream *f, int32_t & v);
			/** Read an uint16 following the m_endian set */
			bool readUInt16(IO::Stream *f, uint16_t & v);
			/** Read an uint32 following the m_endian set */
			bool readUInt32(IO::Stream *f, uint32_t & v);
			/** 
			 * Fetch the data chunk from the file
			 * @param buf the buffer to load into
			 * @param offset the offset
			 * @param buf_size the size of the data to fetch
			 * @return the size retrieved, <= buf_size likely equal
			 */
			size_t fetchData(void *buf, const off_t offset, const size_t buf_size);

		protected:

			RawContainer(const RawContainer&);
			RawContainer & operator=(const RawContainer &);

			void setEndian(EndianType _endian)
				{
					m_endian = _endian;
				}

			/** the file handle */
			IO::Stream *m_file;
			/** the offset from the beginning of the file */
			off_t m_offset;
			EndianType m_endian;
		};
		
	}
}


#endif
