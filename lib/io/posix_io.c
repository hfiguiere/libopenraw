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


#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "io_private.h"
#include "posix_io.h"


/*! private data to be store in the _RawFile */
struct io_data_posix {
	int fd;
};

static RawFileRef posix_open(const char *path, int mode);
static int posix_close(RawFileRef f);
static int posix_seek(RawFileRef f, off_t offset, int whence);
static int posix_read(RawFileRef f, void *buf, size_t count);

/*! posix io methods instance. Constant. */
struct io_methods posix_io_methods = {
	&posix_open,
	&posix_close,
	&posix_seek,
	&posix_read
};


/*! posix implementation for open() */
static RawFileRef posix_open(const char *path, int mode)
{
	struct io_data_posix *data = malloc(sizeof(struct io_data_posix));
	RawFileRef f = (RawFileRef)malloc(sizeof(struct _RawFile));

	memset(f, 0, sizeof(struct _RawFile));
	memset(data, 0, sizeof(struct io_data_posix));
	
	f->methods = &posix_io_methods;
	f->private = data;
	data->fd = open(path, mode);
	if (data->fd == -1) {
		free(data);
		free(f);
		f = NULL;
	}


	return f;
}



/*! posix implementation for close() */
static int posix_close(RawFileRef f)
{
	int retval = 0;
	struct io_data_posix *data = (struct io_data_posix*)f->private;

	retval = close(data->fd);
	free(data);
	return retval;
}


/*! posix implementation for seek() */
static int posix_seek(RawFileRef f, off_t offset, int whence)
{
	int retval = 0;
	struct io_data_posix *data = (struct io_data_posix*)f->private;

	retval = lseek(data->fd, offset, whence);
	if (retval == -1) {
		f->error = errno;
	}
	else {
		f->error = 0;
	}
	return retval;
}


/*! posix implementation for read() */
static int posix_read(RawFileRef f, void *buf, size_t count)
{
	int retval = 0;
	struct io_data_posix *data = (struct io_data_posix*)f->private;

	retval = read(data->fd, buf, count);
	if (retval == -1) {
		f->error = errno;
	}
	else {
		f->error = 0;
	}
	return retval;
}


