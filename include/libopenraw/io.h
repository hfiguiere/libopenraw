/*
 * libopenraw - io.h
 *
 * Copyright (C) 2005 Hubert Figuiere
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

#ifndef LIBOPENRAW_IO_H_
#define LIBOPENRAW_IO_H_

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef __cplusplus
extern "C" {
#endif


/*! a file reference. Opaque structure */
typedef struct _IOFile *IOFileRef;
	
	
/*! IO methods for the IO subsystem.*/
struct io_methods {
	/** open method 
	 * @return a descriptor
	 */
	IOFileRef (*open)(const char *path, int mode);
	/** close method */
	int (*close) (IOFileRef f);
	/** seek in the file */
	int (*seek) (IOFileRef f, off_t offset, int whence);
	/** read method */
	int (*read) (IOFileRef f, void *buf, size_t count);

	off_t (*filesize) (IOFileRef f);
	void* (*mmap) (IOFileRef f, size_t l, off_t offset);
	int   (*munmap) (IOFileRef f, void *addr, size_t l);
};

extern struct io_methods* get_default_io_methods(void);

extern IOFileRef raw_open(struct io_methods * methods, const char *path, 
			      int mode);
extern int raw_close(IOFileRef f);
extern int raw_seek(IOFileRef f, off_t offset, int whence);
extern int raw_read(IOFileRef f, void *buf, size_t count);
extern off_t raw_filesize(IOFileRef f);
extern void *raw_mmap(IOFileRef f, size_t l, off_t offset);
extern int raw_munmap(IOFileRef f, void *addr, size_t l);

extern int raw_get_error(IOFileRef f);
extern char *raw_get_path(IOFileRef f);


#ifdef __cplusplus
}
#endif



#endif
