/*
 * libopenraw - gdkpixbuf.c
 *
 * Copyright (C) 2006-2007 Hubert Figuiere
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

/** @brief gdkpixbuf support */

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libopenraw/thumbnails.h>
#include <libopenraw/rawfile.h>
#include <libopenraw-gnome/gdkpixbuf.h>


static void pixbuf_free(guchar * data, gpointer u)
{
	(void)u;
	free(data);
}


static GdkPixbuf *rotate_pixbuf(GdkPixbuf *tmp, int32_t orientation)
{
	GdkPixbuf *pixbuf = NULL;
	switch(orientation) {
	case 0:
	case 1:
		pixbuf = tmp;
		break;
	case 2:
		pixbuf = gdk_pixbuf_flip(tmp, TRUE);
		gdk_pixbuf_unref(tmp);
		break;
	case 3:
		pixbuf = gdk_pixbuf_rotate_simple(tmp, GDK_PIXBUF_ROTATE_UPSIDEDOWN);
		gdk_pixbuf_unref(tmp);		
		break;
	case 4:
		pixbuf = gdk_pixbuf_rotate_simple(tmp, GDK_PIXBUF_ROTATE_UPSIDEDOWN);
		gdk_pixbuf_unref(tmp);
		tmp = pixbuf;
		pixbuf = gdk_pixbuf_flip(tmp, TRUE);
		gdk_pixbuf_unref(tmp);		
		break;
	case 5:
		pixbuf =  gdk_pixbuf_rotate_simple(tmp, GDK_PIXBUF_ROTATE_CLOCKWISE);
		gdk_pixbuf_unref(tmp);		
		tmp = pixbuf;
		pixbuf = gdk_pixbuf_flip(tmp, FALSE);
		gdk_pixbuf_unref(tmp);		
		break;
	case 6:
		pixbuf =  gdk_pixbuf_rotate_simple(tmp, GDK_PIXBUF_ROTATE_CLOCKWISE);
		gdk_pixbuf_unref(tmp);		
		break;
	case 7:
		pixbuf =  gdk_pixbuf_rotate_simple(tmp, GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
		gdk_pixbuf_unref(tmp);		
		tmp = pixbuf;
		pixbuf = gdk_pixbuf_flip(tmp, FALSE);
		gdk_pixbuf_unref(tmp);		
		break;		
	case 8:
		pixbuf =  gdk_pixbuf_rotate_simple(tmp, GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
		gdk_pixbuf_unref(tmp);		
		break;		
	default:
		break;
	}
	return pixbuf;
}



static GdkPixbuf *_or_thumbnail_to_pixbuf(ORThumbnailRef thumbnail, 
										  int32_t orientation)
{
	GdkPixbuf *tmp = NULL;
	
	const guchar * buf;
	or_data_type format = or_thumbnail_format(thumbnail);
	buf = (const guchar *)or_thumbnail_data(thumbnail);
	
	switch (format)
	{
	case OR_DATA_TYPE_PIXMAP_8RGB:
	{
		uint32_t x, y;
		size_t buf_size;
		guchar * data;

		buf_size = or_thumbnail_data_size(thumbnail);
		data = (guchar*)malloc(buf_size);
		memcpy(data, buf, buf_size);
		or_thumbnail_dimensions(thumbnail, &x, &y);
		
		tmp = gdk_pixbuf_new_from_data(data, 
									   GDK_COLORSPACE_RGB,
									   FALSE, 8, x, y, x * 3, 
									   pixbuf_free, NULL);
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
			tmp = gdk_pixbuf_loader_get_pixbuf(loader);
		}
		break;
	}
	default: 
		break;
	}
	return rotate_pixbuf(tmp, orientation);
}




GdkPixbuf *or_thumbnail_to_pixbuf(ORThumbnailRef thumbnail)
{
	return _or_thumbnail_to_pixbuf(thumbnail, 0); 
}


static GdkPixbuf *_or_gdkpixbuf_extract_thumbnail(const char *path, 
												  uint32_t preferred_size, 
												  gboolean rotate)
{
	ORRawFileRef rf;
	int32_t orientation = 0;
	GdkPixbuf *pixbuf = NULL;
	or_error err = OR_ERROR_NONE;
	ORThumbnailRef thumbnail = NULL;

	rf = or_rawfile_new(path, OR_RAWFILE_TYPE_UNKNOWN);
	if(rf) {
		if(rotate) {
			orientation = or_rawfile_get_orientation(rf);
		}
		thumbnail = or_thumbnail_new();
		err = or_rawfile_get_thumbnail(rf, preferred_size,
									   thumbnail);
		if (err == OR_ERROR_NONE)	{
			pixbuf = _or_thumbnail_to_pixbuf(thumbnail, orientation);
		}
		else {
			g_debug("or_get_extract_thumbnail() failed with %d.", err);
		}
		err = or_thumbnail_release(thumbnail);
		if (err != OR_ERROR_NONE) {
			g_warning("or_thumbnail_release() failed with %d", err);
		}
		or_rawfile_release(rf);
	}

	return pixbuf;
}



GdkPixbuf *or_gdkpixbuf_extract_thumbnail(const char *path, uint32_t preferred_size)
{
	return _or_gdkpixbuf_extract_thumbnail(path, preferred_size, FALSE);
}

GdkPixbuf *or_gdkpixbuf_extract_rotated_thumbnail(const char *path, uint32_t preferred_size)
{
	return _or_gdkpixbuf_extract_thumbnail(path, preferred_size, TRUE);
}



