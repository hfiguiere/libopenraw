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
#include <stdlib.h>

#include <libopenraw/libopenraw.h>
#include <libopenraw-gnome/gdkpixbuf.h>

static void pixbuf_free(guchar * data, gpointer u)
{
    ORBitmapDataRef b = (ORBitmapDataRef)u;
    (void)data;
    or_bitmapdata_release(b);
}

int
main(int argc, char **argv)
{
	char *filename = argv[1];

	(void)argc;
	or_debug_set_level(DEBUG2);
	g_type_init();

	if(filename && *filename)
	{
		GdkPixbuf *pixbuf = NULL;
		ORRawFileRef raw_file = or_rawfile_new(filename, OR_DATA_TYPE_NONE);
		
		if(raw_file) {
		    or_error err;
		    ORBitmapDataRef bitmapdata = or_bitmapdata_new();
		    err = or_rawfile_get_rendered_image(raw_file, bitmapdata, 0);
		    if(err == OR_ERROR_NONE) {
			uint32_t x,y;
			x = y = 0;
			or_bitmapdata_dimensions(bitmapdata, &x, &y);
			pixbuf = gdk_pixbuf_new_from_data(or_bitmapdata_data(bitmapdata), 
							  GDK_COLORSPACE_RGB,
							  FALSE, 8, x , y , 
							  ( x - 2 )* 3, 
							  pixbuf_free, bitmapdata);
		    }
		    or_rawfile_release(raw_file);
		}
		
		
		if(pixbuf) {
			gdk_pixbuf_save (pixbuf, "gdk-demosaic.jpg", "jpeg", NULL,
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
