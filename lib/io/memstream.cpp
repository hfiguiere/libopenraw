/*
 * libopenraw - memstream.cpp
 *
 * Copyright (C) 2007 Hubert Figuière
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


#include <string.h>

#include <libopenraw/libopenraw.h>

#include "memstream.h"
#include "debug.h"

using namespace Debug;

namespace OpenRaw {
	namespace IO {
		
		MemStream::MemStream(void *ptr, size_t s)
			: Stream(""),
				m_ptr(ptr),
				m_size(s),
				m_current(NULL)
		{
		}

		or_error MemStream::open()
		{
			m_current = (unsigned char *)m_ptr;
			return OR_ERROR_NONE;
		}


		int MemStream::close()
		{
			m_current = NULL;
			return 0;
		}

		int MemStream::seek(off_t offset, int whence)
		{
//			Trace(DEBUG1) << "MemStream::seek " << offset 
//										<< " bytes - whence = " 
//										<< whence <<  "\n";
			// TODO check bounds
			if (m_current == NULL) {
				// TODO set error code
				return -1;
			}
			switch(whence)
			{
			case SEEK_SET:
				m_current = (unsigned char*)m_ptr + offset;
				break;
			case SEEK_END:
				m_current = (unsigned char*)m_ptr + m_size + offset;
				break;
			case SEEK_CUR:
				m_current += offset;
				break;
			default:
				return -1;
				break;
			}
			return 0;
		}


		int MemStream::read(void *buf, size_t count)
		{
			if((m_current == NULL) || (m_ptr == NULL)) {
				Trace(DEBUG1) << "MemStream::failed\n";
				return -1;
			}
			// TODO check the bounds
			memcpy(buf, m_current, count);
			m_current += count;
			return count;
		}


		off_t MemStream::filesize()
		{
			return m_size;
		}

	}
}
