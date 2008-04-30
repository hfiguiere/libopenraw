/*
 * libopenraw - streamclone.h
 *
 * Copyright (C) 2006 Hubert Figuière
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


#ifndef __IO_STREAMCLONE_H__
#define __IO_STREAMCLONE_H__

#include <sys/types.h>
#include <unistd.h>

#include "stream.h"

namespace OpenRaw {
	namespace IO {

		/** @brief cloned stream. Allow reading from a different offset
		 */
		class StreamClone
			: public Stream
		{
		public:
			StreamClone(Stream *clone, off_t offset);
			virtual ~StreamClone();
			
			virtual Error open();
			virtual int close();
			virtual int seek(off_t offset, int whence);
			virtual int read(void *buf, size_t count);
			virtual off_t filesize();


		private:
			/** private copy constructor to make sure it is not called */
			StreamClone(const StreamClone& f);
			/** private = operator to make sure it is never called */
			StreamClone & operator=(const StreamClone&);

			Stream *m_cloned;
			off_t m_offset;
		};

	}
}

#endif
