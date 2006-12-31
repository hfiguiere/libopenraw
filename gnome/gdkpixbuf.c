/*
 * libopenraw - gdkpixbuf.c
 *
 * Copyright (C) 2006 Hubert Figuiere
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

/** @brief gdkpixbuf support */

#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libopenraw/thumbnails.h>
#include <libopenraw-gnome/gdkpixbuf.h>

GdkPixbuf *or_thumbnail_to_pixbuf(ORThumbnailRef thumbnail)
{
	GdkPixbuf *pixbuf = NULL;
	
	const guchar * buf;
	or_data_type format = or_thumbnail_format(thumbnail);
	buf = (const guchar *)or_thumbnail_data(thumbnail);
	
	switch (format)
	{
	case OR_DATA_TYPE_PIXMAP_8RGB:
	{
		uint32_t x, y;
		or_thumbnail_dimensions(thumbnail, &x, &y);
		pixbuf = gdk_pixbuf_new_from_data(buf, 
																			GDK_COLORSPACE_RGB,
																			FALSE, 24, x, y, 0, 
																			NULL, NULL);
		break;
	}
	case OR_DATA_TYPE_JPEG:
	case OR_DATA_TYPE_TIFF:
	case OR_DATA_TYPE_PNG:
	{
		GdkPixbufLoader *loader = NULL;
		size_t count = or_thumbnail_data_size(thumbnail);
		loader = gdk_pixbuf_loader_new();
		if (loader != NULL) {
			gdk_pixbuf_loader_write(loader, buf, count, NULL);
			gdk_pixbuf_loader_close(loader, NULL);
			pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);
			g_object_unref(loader);
		}
		break;
	}
	default: 
		break;
	}
	return pixbuf;
}


GdkPixbuf *or_gdkpixbuf_extract_thumbnail(const char *path, uint32_t preferred_size)
{
	GdkPixbuf *pixbuf = NULL;
	or_error err = OR_ERROR_NONE;
	ORThumbnailRef thumbnail = NULL;
	g_debug("file %s is raw", path);

	err = or_get_extract_thumbnail(path, preferred_size,
																 &thumbnail);
	if (err == OR_ERROR_NONE)	{
		pixbuf = or_thumbnail_to_pixbuf(thumbnail);
		err = or_thumbnail_release(thumbnail);
		if (err != OR_ERROR_NONE) {
			g_warning("or_thumbnail_release() failed with %d", err);
		}
	}
	else {
		g_debug("or_get_extract_thumbnail() failed with %d.", err);
	}
	return pixbuf;
}


