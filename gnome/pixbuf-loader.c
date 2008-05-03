/*
 * libopenraw - pixbuf-loader.c
 *
 * Copyright (C) 2008 Hubert Figuiere
 *
 * This library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */


/** @brief gdkpixbuf loader for RAW files */

#include <stdlib.h>

#include <libopenraw/libopenraw.h>

#define GDK_PIXBUF_ENABLE_BACKEND
#include <gdk-pixbuf/gdk-pixbuf-io.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "demosaic.h"

G_MODULE_EXPORT void fill_vtable (GdkPixbufModule *module);
G_MODULE_EXPORT void fill_info (GdkPixbufFormat *info);

static void pixbuf_free(guchar * data, gpointer u)
{
        (void)u;
        free(data);
}

#if 0
static GdkPixbuf * 
gdk_pixbuf__or_image_load(FILE *f, GError **error)
{
	(void)f;
	(void)error;
	return NULL;
}
#endif


typedef struct {
	GdkPixbufModuleSizeFunc     size_func;
	GdkPixbufModulePreparedFunc prepared_func;
	GdkPixbufModuleUpdatedFunc  updated_func;
	gpointer                    user_data;
	GByteArray                 *data;
} OrContext;

static gpointer
gdk_pixbuf__or_image_begin_load (GdkPixbufModuleSizeFunc size_func,
								 GdkPixbufModulePreparedFunc prepared_func,
								 GdkPixbufModuleUpdatedFunc  updated_func,
								 gpointer user_data,
								 GError **error)
{
	OrContext *context = (OrContext*)calloc(1, sizeof(OrContext));
	context->size_func = size_func;
	context->prepared_func = prepared_func;
	context->updated_func = updated_func;
	context->user_data = user_data;
	context->data = g_byte_array_new();

	return (gpointer)context;
}

static gboolean
gdk_pixbuf__or_image_load_increment (gpointer data,
									 const guchar *buf, guint size,
									 GError **error)
{
	OrContext *context = (OrContext*)data;
	g_byte_array_append (context->data, buf, size);
	return TRUE;
}

static gboolean
gdk_pixbuf__or_image_stop_load (gpointer data, GError **error)
{
	OrContext *context = (OrContext*)data;
	gboolean result = FALSE;

	GdkPixbuf *pixbuf = NULL;
	ORRawFileRef raw_file = NULL;
	raw_file = or_rawfile_new_from_memory(context->data->data, context->data->len,
							   OR_DATA_TYPE_NONE);
	
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
											  FALSE, 8, x, y, 
											  (x - 2) * 3, 
											  pixbuf_free, NULL);
		}
		or_rawdata_release(rawdata);
		or_rawfile_release(raw_file);

		if (context->prepared_func != NULL) {
			(*context->prepared_func) (pixbuf, NULL, context->user_data);
		}
		if (context->updated_func != NULL) {
			(*context->updated_func) (pixbuf, 0, 0,
									   gdk_pixbuf_get_width(pixbuf),
									   gdk_pixbuf_get_height(pixbuf),
									   context->user_data);
		}
		result = TRUE;
	}


	g_byte_array_free(context->data, TRUE);
	free(context);
	return result;
}

void
fill_vtable (GdkPixbufModule *module)
{
  module->begin_load     = gdk_pixbuf__or_image_begin_load;
  module->stop_load      = gdk_pixbuf__or_image_stop_load;
  module->load_increment = gdk_pixbuf__or_image_load_increment;

  module->load           = NULL; /*gdk_pixbuf__or_image_load;*/
}


void
fill_info (GdkPixbufFormat *info)
{
	static GdkPixbufModulePattern signature[] = {
		{ "MM \x2a", "  z ", 80 }, /* TIFF */
		{ "II\x2a ", "   z", 80 }, /* TIFF */
		{ "IIRO", "    ", 100 },   /* ORF */
		{ " MRM", "z   ", 100 },   /* MRW */
		{ "II\x1a   HEAPCCDR", "   zzz        ", 100 }, /* CRW */
		{ NULL, NULL, 0 }
	};
	
	static gchar *mime_types[] = {
		"image/x-adobe-dng",
		"image/x-canon-cr2",
		"image/x-canon-crw",
		"image/x-nikon-nef",
		"image/x-olympus-orf",
		"image/x-pentax-pef",
		"image/x-sony-arw",
		"image/x-epson-erf",
		"image/x-minolta-mrw",
		NULL
	};
	
	static gchar *extensions[] = {
		"dng",
		"cr2",
		"crw",
		"nef",
		"orf",
		"pef",
		"arw",
		"erf",
		"mrw",
		NULL
	};
	
	info->name        = "Digital camera RAW";
	info->signature   = signature;
	info->description = "Digital camera RAW images loader.";
	info->mime_types  = mime_types;
	info->extensions  = extensions;
	info->flags       = 0;
	info->license     = "LGPL";
}

