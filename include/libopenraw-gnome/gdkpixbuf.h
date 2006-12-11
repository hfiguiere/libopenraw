





#ifndef __LIBOPENRAW_GDKPIXBUF_H_
#define __LIBOPENRAW_GDKPIXBUF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libopenraw/thumbnails.h>

GdkPixbuf *or_thumbnail_to_pixbuf(ORThumbnailRef thumbnail);

GdkPixbuf *or_gdkpixbuf_extract_thumbnail(const char *path, 
																					uint32_t preferred_size);

#ifdef __cplusplus
}
#endif

#endif
