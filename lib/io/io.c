/*
 * libopenraw - io.c
 *
 * Copyright (C) 2005-2014 Hubert Figuiere
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

#include <stdlib.h>
#include <errno.h>

#include <libopenraw/io.h>
#include "io_private.h"
#include "posix_io.h"

#ifdef __cplusplus
extern "C" {
#endif

/** check pointer validity */
#define CHECK_PTR(p,r) \
	if(p == NULL) { return r; }

/** get the default io methods instance 

  @fixme currently hardcoded to POSIX
  @return the default io_methods instance, currentlty posix_io_methods
*/
struct io_methods* get_default_io_methods(void)
{
	return &posix_io_methods;
}

/** open a file
  @param methods the io_methods instance to use
  @param path the file path
  @param mode the POSIX file mode
 */
IOFileRef raw_open(struct io_methods * methods, const char *path, int mode)
{
	CHECK_PTR(methods, NULL);
	return methods->open(path, mode);
}

/** close the file

  @param f the file to close

  The implement should free the private data is at will free the file ref. 
  f will be invalid on return

  @return -1 if error.
 */
int raw_close(IOFileRef f)
{
	int retval;
	CHECK_PTR(f,-1);
	retval = f->methods->close(f);
	free(f);
	return retval;
}


/** seek in the file
  @param f the file to seek
  @param offset the offset to seek
  @param whence the the directive for seek. See lseek(2) man page

  @return -1 if error
 */
int raw_seek(IOFileRef f, off_t offset, int whence)
{
	CHECK_PTR(f,-1);
	return f->methods->seek(f, offset, whence);
}


/** read in the file
  @param f the file to readk
  @param buf the buffer to read in
  @param count the number of byte to read

  @return -1 if error
*/
int raw_read(IOFileRef f, void *buf, size_t count)
{
	CHECK_PTR(f,-1);
	return f->methods->read(f, buf, count);
}

off_t raw_filesize(IOFileRef f)
{
	CHECK_PTR(f,0);
	return f->methods->filesize(f);
}

void *raw_mmap(IOFileRef f, size_t l, off_t offset)
{
	CHECK_PTR(f,NULL);
	return f->methods->mmap(f, l, offset);
}


int raw_munmap(IOFileRef f, void *addr, size_t l)
{
	CHECK_PTR(f,-1);
	return f->methods->munmap(f, addr, l);
}


/** get the error for the file

  @param f the file 
  @return the errno code
*/
int raw_get_error(IOFileRef f)
{
	CHECK_PTR(f,EFAULT);
	return f->error;
}


/** get the real path of the file

  This function is needed because libtiff needs a pathname to 
  open TIFF files.

  @param f the file
  @return the pathname
*/
char *raw_get_path(IOFileRef f)
{
	CHECK_PTR(f,NULL);
	return f->path;
}


#ifdef __cplusplus
}
#endif

