/*
 * libopenraw - iofile.h
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include <libopenraw/libopenraw.h>
#include "iofile.h"


namespace OpenRaw {
	namespace Internals {
	
	IOFile::IOFile(const char *filename)
		: m_fileName(filename),
		  m_methods(::get_default_io_methods()),
		  m_ioRef(NULL)
	{
	}

	IOFile::~IOFile()
	{
	}
	
	IOFile::Error IOFile::open()
	{
		m_ioRef = ::raw_open(m_methods, m_fileName.c_str(), O_RDONLY);
		if (m_ioRef == NULL) {
			return OR_ERROR_CANT_OPEN;
		}
		return OR_ERROR_NONE;
	}

	int IOFile::close()
	{
		return ::raw_close(m_ioRef);
	}

	int IOFile::seek(off_t offset, int whence)
	{
		return ::raw_seek(m_ioRef, offset, whence);
	}

	int IOFile::read(void *buf, size_t count)
	{
		return ::raw_read(m_ioRef, buf, count);
	}

}
}
