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


#include "io/io.h"
#include "io_private.h"
#include "posix_io.h"



/*! get the default io methods instance 

  \fixme currently hardcoded to POSIX
  \return the default io_methods instance, currentlty posix_io_methods
*/
struct io_methods* get_default_io_methods(void)
{
	return &posix_io_methods;
}

/*! open a file
  \param methods the io_methods instance to use
  \param path the file path
  \param mode the POSIX file mode
 */
cwk_file_ref cwk_open(struct io_methods * methods, const char *path, int mode)
{
	return methods->open(path, mode);
}

/*! close the file

  \param f the file to close

  The implement should free the private data is at will free the file ref. 
  f will be invalid on return

  \return -1 if error.
 */
int cwk_close(cwk_file_ref f)
{
	int retval = f->methods->close(f);
	free(f);
	return retval;
}


/*! seek in the file
  \param f the file to seek
  \param offset the offset to seek
  \param whence the the directive for seek. See lseek(2) man page

  \return -1 if error
 */
int cwk_seek(cwk_file_ref f, off_t offset, int whence)
{
	return f->methods->seek(f, offset, whence);
}


/*! read in the file
  \param f the file to readk
  \param buf the buffer to read in
  \param count the number of byte to read

  \return -1 if error
*/
int cwk_read(cwk_file_ref f, void *buf, size_t count)
{
	return f->methods->read(f, buf, count);
}


/*! get the error for the file

  \param f the file 
  \return the errno code
*/
int cwk_get_error(cwk_file_ref f)
{
	return f->error;
}
