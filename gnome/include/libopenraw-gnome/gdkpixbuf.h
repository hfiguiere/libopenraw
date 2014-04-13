/*
 * libopenraw - gdkpixbuf.h
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef LIBOPENRAW_GDKPIXBUF_H_
#define LIBOPENRAW_GDKPIXBUF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libopenraw/thumbnails.h>

GdkPixbuf *or_thumbnail_to_pixbuf(ORThumbnailRef thumbnail);

GdkPixbuf *or_gdkpixbuf_extract_thumbnail(const char *path, 
										  uint32_t preferred_size);
GdkPixbuf *or_gdkpixbuf_extract_rotated_thumbnail(const char *path, 
												  uint32_t preferred_size);

#ifdef __cplusplus
}
#endif

#endif
