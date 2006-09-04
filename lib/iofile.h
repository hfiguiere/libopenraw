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



#ifndef __IO_FILE_H__
#define __IO_FILE_H__

#include <sys/types.h>
#include <unistd.h>

#include <string>

#include <libopenraw/libopenraw.h>

namespace OpenRaw {
	namespace Internals {


/** Abstract file IO */
class IOFile
{
public:
	/** Contruct the file 
	 * @param filename the full pathname for the file
	 */
	IOFile(const char *filename);
	virtual ~IOFile();

	/** Error type.
	 * @see or_error
	 */
	typedef ::or_error Error;
	
// file APIs
	/** open the file */
	Error open();
	/** close the file */
	int close();
	/** seek in the file. Semantics are similar to POSIX */
 	int seek(off_t offset, int whence);
	/** read in the file. Semantics are similar to POSIX */
	int read(void *buf, size_t count);
	off_t filesize();
	void *mmap(size_t l, off_t offset);
	int munmap(void *addr, size_t l);

	int get_error();
	/** get the path of the file */
	const std::string &get_path() const
		{
			return m_fileName;
		}

private:
	/** private copy constructor to make sure it is not called */
	IOFile(const IOFile& f);
  /** private = operator to make sure it is never called */
	IOFile & operator=(const IOFile&);

	/** the file name (full path) */
	std::string m_fileName;
	/** the interface to the C io */
	::io_methods *m_methods;
	/** the C io file handle */
	::IOFileRef m_ioRef;
};


}
}

#endif
