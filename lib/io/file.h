/*
 * libopenraw - file.h
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



#ifndef __IO_FILE_H__
#define __IO_FILE_H__

#include <sys/types.h>
#include <unistd.h>

#include <libopenraw/libopenraw.h>

#include "stream.h"


namespace OpenRaw {
	namespace IO {


/** file IO stream */
		class File
			: public Stream
		{
		public:
			/** Contruct the file 
			 * @param filename the full pathname for the file
			 */
			File(const char *filename);
			virtual ~File();
			
// file APIs
			/** open the file */
			virtual Error open();
			/** close the file */
			virtual int close();
			/** seek in the file. Semantics are similar to POSIX */
			virtual int seek(off_t offset, int whence);
			/** read in the file. Semantics are similar to POSIX */
			virtual int read(void *buf, size_t count);
			virtual off_t filesize();
//			virtual void *mmap(size_t l, off_t offset);
//			virtual int munmap(void *addr, size_t l);
			
		private:
			/** private copy constructor to make sure it is not called */
			File(const File& f);
			/** private = operator to make sure it is never called */
			File & operator=(const File&);
			
			/** the interface to the C io */
			::io_methods *m_methods;
			/** the C io file handle */
			::IOFileRef m_ioRef;
		};


	}
}

#endif
