/*
 * libopenraw - iostream.h
 *
 * Copyright (C) 2006-2007 Hubert Figuière
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


#include "stream.h"
#include "exception.h"

namespace OpenRaw {
	namespace IO {
		
		Stream::Stream(const char *filename)
			: m_fileName(filename),
				m_error(OR_ERROR_NONE)
		{
		}

		Stream::~Stream()
		{
		}

		uint8_t Stream::readByte() throw(Internals::IOException)
		{
			uint8_t theByte;
			int r = read(&theByte, 1);
			if (r != 1) {
				// TODO add the error code
				throw Internals::IOException("Stream::readByte() failed.");
			}
			return theByte;
		}
	}
}


