/*
 * Copyright (C) 2005 Hubert Figuiere
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */



#ifndef __LIBCWK_IO_H
#define __LIBCWK_IO_H

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>


#ifdef __cplusplus
extern "C" {
#endif


/*! a file reference. Opaque structure */
typedef struct cwk_file *cwk_file_ref;
	
	
/*! IO methods for the IO subsystem.*/
struct io_methods {
	cwk_file_ref (*open)(const char *path, int mode);
	int (*close) (cwk_file_ref f);
	int (*seek) (cwk_file_ref f, off_t offset, int whence);
	int (*read) (cwk_file_ref f, void *buf, size_t count);
};

extern struct io_methods* get_default_io_methods(void);

extern cwk_file_ref cwk_open(struct io_methods * methods, const char *path, 
			      int mode);
extern int cwk_close(cwk_file_ref f);
extern int cwk_seek(cwk_file_ref f, off_t offset, int whence);
extern int cwk_read(cwk_file_ref f, void *buf, size_t count);

extern int cwk_get_error(cwk_file_ref f);


#ifdef __cplusplus
};
#endif



#endif
