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
        (void)u;
        free(data);
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
			ORRawDataRef rawdata = or_rawdata_new();
			int32_t orientation = or_rawfile_get_orientation(raw_file);
			
			err = or_rawfile_get_rawdata(raw_file, rawdata, 0);
			if(err == OR_ERROR_NONE) {
				or_cfa_pattern pattern;
				uint32_t x,y;
				uint16_t *src;
				uint8_t *dst;
				pattern = or_rawdata_get_cfa_pattern(rawdata);
				x = y = 0;
				or_rawdata_dimensions(rawdata, &x, &y);
				dst = (uint8_t*)malloc(sizeof(uint8_t) * 3 * x * y);
				src = (uint16_t*)or_rawdata_data(rawdata);
				/* check the size of the data*/
				demosaic(src , x, y, pattern, dst);
				pixbuf = gdk_pixbuf_new_from_data(dst, GDK_COLORSPACE_RGB,
												  FALSE, 8, x , y , 
												  ( x - 2 )* 3, 
												  pixbuf_free, NULL);
			}
			or_rawdata_release(rawdata);
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
