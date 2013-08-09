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

#include <libopenraw/libopenraw.h>
#include <libopenraw-gnome/gdkpixbuf.h>

int
main(int argc, char **argv)
{
    char *filename = argv[1];

    (void)argc;
    or_debug_set_level(DEBUG2);
#if !GLIB_CHECK_VERSION(2,36,0)
    /* deprecated in 2.36 */
    g_type_init();
#endif

    if(filename && *filename)
    {
        GdkPixbuf *pixbuf;

        pixbuf = or_gdkpixbuf_extract_rotated_thumbnail(filename, 160);
        if(pixbuf) {
            gdk_pixbuf_save (pixbuf, "gdk.jpg", "jpeg", NULL,
                             "quality", "100", NULL);
            g_object_unref(pixbuf);
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
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
