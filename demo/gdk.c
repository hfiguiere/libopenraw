/*
 * libopenraw - gdk.c
 *
 * Copyright (C) 2007 Hubert Figuiere
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */



#include <stdio.h>

#include <libopenraw/libopenraw.h>
#include <libopenraw-gnome/gdkpixbuf.h>

int
main(int argc, char **argv)
{
	char *filename = argv[1];

	(void)argc;
	or_debug_set_level(DEBUG2);
	g_type_init();

	if(filename && *filename)
	{
		GdkPixbuf *pixbuf;

		pixbuf = or_gdkpixbuf_extract_rotated_thumbnail(filename, 160);
		if(pixbuf) {
			gdk_pixbuf_save (pixbuf, "gdk.jpg", "jpeg", NULL,
							 "quality", "100", NULL);
			gdk_pixbuf_unref(pixbuf);
		}
		else {
			printf("error\n");
		}
	}
	else {
		printf("No input file name\n");
	}

	return 0;
}
