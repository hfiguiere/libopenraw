/*
 * libopenraw - stream.h
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


#ifndef __IO_STREAM_H__
#define __IO_STREAM_H__

#include <sys/types.h>
#include <unistd.h>

#include <string>

#include <libopenraw/libopenraw.h>

#include "exception.h"

namespace OpenRaw {
	namespace IO {
		
		/** 
		 * @brief base virtual class for IO
		 */
		class Stream
		{
		public:
			/** Construct the file 
			 * @param filename the full uri for the file
			 */
			Stream(const char *filename);
			virtual ~Stream();
			
			/** Error type.
			 * @see or_error
			 */
			typedef ::or_error Error;
			
// file APIs
			/** open the file */
			virtual Error open() = 0;
			/** close the file */
			virtual int close() = 0;
			/** seek in the file. Semantics are similar to POSIX lseek() */
			virtual int seek(off_t offset, int whence) = 0;
			/** read in the file. Semantics are similar to POSIX read() */
			virtual int read(void *buf, size_t count) = 0;
			virtual off_t filesize() = 0;
//			virtual void *mmap(size_t l, off_t offset) = 0;
//			virtual int munmap(void *addr, size_t l) = 0;
			
			Error get_error()
				{
					return m_error;
				}

			/** get the uri path of the file */
			const std::string &get_path() const
				{
					return m_fileName;
				}

			uint8_t readByte() throw(Internals::IOException);
		protected:
			void set_error(Error error)
				{
					m_error = error;
				}

		private:
			/** private copy constructor to make sure it is not called */
			Stream(const Stream& f);
			/** private = operator to make sure it is never called */
			Stream & operator=(const Stream&);
			
			/** the file name (full path) */
			std::string m_fileName;
			Error m_error;
		};

	}
}


#endif
