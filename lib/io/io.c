/*
 * libopenraw - io.c
 *
 * Copyright (C) 2005-2006 Hubert Figuiere
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

#include <stdlib.h>

#include "libopenraw/io.h"
#include "io_private.h"
#include "posix_io.h"
#include "or_debug.h"


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
RawFileRef raw_open(struct io_methods * methods, const char *path, int mode)
{
	return methods->open(path, mode);
}

/** close the file

  @param f the file to close

  The implement should free the private data is at will free the file ref. 
  f will be invalid on return

  @return -1 if error.
 */
int raw_close(RawFileRef f)
{
	int retval = f->methods->close(f);
	free(f);
	return retval;
}


/** seek in the file
  @param f the file to seek
  @param offset the offset to seek
  @param whence the the directive for seek. See lseek(2) man page

  @return -1 if error
 */
int raw_seek(RawFileRef f, off_t offset, int whence)
{
	return f->methods->seek(f, offset, whence);
}


/** read in the file
  @param f the file to readk
  @param buf the buffer to read in
  @param count the number of byte to read

  @return -1 if error
*/
int raw_read(RawFileRef f, void *buf, size_t count)
{
	return f->methods->read(f, buf, count);
}

off_t raw_filesize(RawFileRef f)
{
	return f->methods->filesize(f);
}

void *raw_mmap(RawFileRef f, size_t l, off_t offset)
{
	return f->methods->mmap(f, l, offset);
}


int raw_munmap(RawFileRef f, void *addr, size_t l)
{
	return f->methods->munmap(f, addr, l);
}


/** get the error for the file

  @param f the file 
  @return the errno code
*/
int raw_get_error(RawFileRef f)
{
	return f->error;
}


/** get the real path of the file

  This function is needed because libtiff needs a pathname to 
  open TIFF files.

  @param f the file
  @return the pathname
*/
char *raw_get_path(RawFileRef f)
{
	return f->path;
}



#include <tiffio.h>

/* These are the glue routing for TIFFIO */
static tsize_t 
_TIFFRead(thandle_t fileRef, tdata_t data, tsize_t size)
{	
	or_debug("_TIFFRead()");
	return raw_read((RawFileRef)fileRef, data, size);
}

static tsize_t 
_TIFFWrite(thandle_t fileRef, tdata_t data, tsize_t size)
{	
	or_debug("_TIFFWrite()");
	return -1;
}


static toff_t 
_TIFFSeek(thandle_t fileRef, toff_t off, int offmode)
{
	or_debug("_TIFFSeek()");
	return raw_seek((RawFileRef)fileRef, off, offmode);
}


static int 
_TIFFClose(thandle_t fileRef)
{
	or_debug("_TIFFClose()");
	/* this is a no op*/
	return 0;
}


static toff_t 
_TIFFSize(thandle_t fileRef)
{
	or_debug("_TIFFSize()");
	
	return raw_filesize((RawFileRef)fileRef);
}


static int 
_TIFFMapFile(thandle_t fileRef, tdata_t* d, toff_t* s)
{
	or_debug("_TIFFMapFile()");
	toff_t size = _TIFFSize(fileRef);

	if (size != -1) {
		*d = raw_mmap((RawFileRef)fileRef, size, 0);
		if (*d != (tdata_t) -1) {
			*s = size;
			return 1;
		}
	}

	return 0;
}


static void 
_TIFFUnmapFile(thandle_t fileRef, tdata_t d, toff_t o)
{
	or_debug("_TIFFUnmapFile()");
	raw_munmap((RawFileRef)fileRef, d, o);
}


TIFF *raw_tiff_open(RawFileRef f)
{
	return TIFFClientOpen("", "r", (thandle_t)f, 
						  &_TIFFRead, &_TIFFWrite, &_TIFFSeek, &_TIFFClose,
						  &_TIFFSize, &_TIFFMapFile, &_TIFFUnmapFile);
}
