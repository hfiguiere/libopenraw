/*
 * libopenraw - file.h
 *
 * Copyright (C) 2006 Hubert Figuière
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


#include <libopenraw/libopenraw.h>
#include "file.h"


namespace OpenRaw {
	namespace IO {
	
		File::File(const char *filename)
			: OpenRaw::IO::Stream(filename),
				m_methods(::get_default_io_methods()),
				m_ioRef(NULL)
		{
		}

		File::~File()
		{
		}
	
		File::Error File::open()
		{
			m_ioRef = ::raw_open(m_methods, get_path().c_str(), O_RDONLY);
			if (m_ioRef == NULL) {
				return OR_ERROR_CANT_OPEN;
			}
			return OR_ERROR_NONE;
		}

		int File::close()
		{
			return ::raw_close(m_ioRef);
		}

		int File::seek(off_t offset, int whence)
		{
			return ::raw_seek(m_ioRef, offset, whence);
		}

		int File::read(void *buf, size_t count)
		{
			return ::raw_read(m_ioRef, buf, count);
		}

		off_t File::filesize()
		{
			return ::raw_filesize(m_ioRef);
		}

	}
}
