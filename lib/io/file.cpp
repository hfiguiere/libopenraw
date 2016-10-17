/*
 * libopenraw - file.cpp
 *
 * Copyright (C) 2006-2016 Hubert Figuière
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

#include <fcntl.h>
#include <string>

#include "libopenraw/consts.h"
#include "libopenraw/io.h"

#include "io/stream.hpp"
#include "file.hpp"

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
			if (m_ioRef) {
				close();
			}
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
			int result = ::raw_close(m_ioRef);
			m_ioRef = NULL;
			return result;
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
