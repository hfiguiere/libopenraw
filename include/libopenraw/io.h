/*
 * libopenraw - io.h
 *
 * Copyright (C) 2005 Hubert Figuiere
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

#ifndef __LIBOPENRAW_IO_H
#define __LIBOPENRAW_IO_H

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include <tiffio.h>

#ifdef __cplusplus
extern "C" {
#endif


/*! a file reference. Opaque structure */
typedef struct _RawFile *RawFileRef;
	
	
/*! IO methods for the IO subsystem.*/
struct io_methods {
	/** open method 
	 * @return a descriptor
	 */
	RawFileRef (*open)(const char *path, int mode);
	/** close method */
	int (*close) (RawFileRef f);
	/** seek in the file */
	int (*seek) (RawFileRef f, off_t offset, int whence);
	/** read method */
	int (*read) (RawFileRef f, void *buf, size_t count);

	off_t (*filesize) (RawFileRef f);
	void* (*mmap) (RawFileRef f, size_t l, off_t offset);
	int   (*munmap) (RawFileRef f, void *addr, size_t l);
};

extern struct io_methods* get_default_io_methods(void);

extern RawFileRef raw_open(struct io_methods * methods, const char *path, 
			      int mode);
extern int raw_close(RawFileRef f);
extern int raw_seek(RawFileRef f, off_t offset, int whence);
extern int raw_read(RawFileRef f, void *buf, size_t count);
extern off_t raw_filesize(RawFileRef f);
extern void *raw_mmap(RawFileRef f, size_t l, off_t offset);
extern int raw_munmap(RawFileRef f, void *addr, size_t l);

extern int raw_get_error(RawFileRef f);
extern char *raw_get_path(RawFileRef f);

extern TIFF *raw_tiff_open(RawFileRef f);

#ifdef __cplusplus
};
#endif



#endif
