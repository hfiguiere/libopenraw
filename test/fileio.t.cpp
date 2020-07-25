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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */



#include <stdio.h>
#include <errno.h>
#include "libopenraw/io.h"




int main (int argc, char **argv)
{
	IOFileRef f;
	int retval;
	char buf[128];
	(void)argc;
	(void)argv;

	f = raw_open(get_default_io_methods(), "/etc/hosts", O_RDONLY);

	if (f == NULL) {
		fprintf(stderr, "failed to open /etc/hosts\n");
		return 1;
	}
	fprintf(stderr, "error code is %d\n", raw_get_error(f));

	retval = raw_seek(f, 0, SEEK_SET);
	if (retval == -1) {
		fprintf(stderr, "failed to seek\n");
		return 2;
	}

	fprintf(stderr, "position is %d\n", retval);

	retval = raw_read(f, buf, 10);
	if (retval == -1) {
		fprintf(stderr, "failed to read with error %d\n", raw_get_error(f));
		return 3;
	}

	fprintf(stderr, "read %d bytes\n", retval);

	retval = raw_close(f);
	if (retval == -1) {
		fprintf(stderr, "failed to close\n");
		return 4;
	}

	return 0;
}



