/*
 * libopenraw - gdk.c
 *
 * Copyright (C) 2007-2013 Hubert Figuiere
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
    GdkPixbuf *pixbuf = NULL;
    ORRawFileRef raw_file = NULL;
    char *filename = argv[1];
    or_error err;
    ORBitmapDataRef bitmapdata = NULL;

    (void)argc;
    or_debug_set_level(DEBUG2);
#if !GLIB_CHECK_VERSION(2,36,0)
    /* deprecated in 2.36 */
    g_type_init();
#endif

    if(!filename || !*filename) {
        printf("No input file name\n");
        return 1;
    }

    raw_file = or_rawfile_new(filename, OR_RAWFILE_TYPE_UNKNOWN);
    if(!raw_file) {
        printf("error loading file %s\n", filename);
        return 1;
    }

    bitmapdata = or_bitmapdata_new();
    err = or_rawfile_get_rendered_image(raw_file, bitmapdata, 0);
    if(err == OR_ERROR_NONE) {
        uint32_t x,y;
        or_data_type format = or_bitmapdata_format(bitmapdata);
        x = y = 0;
        or_bitmapdata_dimensions(bitmapdata, &x, &y);
        if(format == OR_DATA_TYPE_PIXMAP_8RGB) {
            pixbuf = gdk_pixbuf_new_from_data(or_bitmapdata_data(bitmapdata),
                                              GDK_COLORSPACE_RGB,
                                              FALSE, 8, x , y,
                                              x * 3,
                                              pixbuf_free, bitmapdata);
        }
        else {
            /* Gdk pixbuf still suck ass in 2012 not supporting 16bpp. */
            printf("16 bits isn't supported because Gdkpixbuf still suck\n");
        }
    }
    or_rawfile_release(raw_file);

    if(pixbuf) {
        GError* error = NULL;
        if(!gdk_pixbuf_save (pixbuf, "gdk-demosaic.jpg", "jpeg", &error,
                             "quality", "100", NULL) && error) {
            printf("error saving image: %s\n", error->message);
            g_error_free(error);
        }
        g_object_unref(pixbuf);
    }
    else {
        printf("error creating pixbuf\n");
        return 1;
    }

    return 0;
}
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
