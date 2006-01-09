/*
 * libopenraw - or_exif.c
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


#include <libexif/exif-loader.h>

#include "or_exif.h"

or_error 
exif_get_thumbnail(RawFileRef raw_file, ORThumbnailRef thumbnail)
{
	or_error err = OR_ERROR_NONE;	
	ExifLoader *loader;
	ExifData   *exifData;

	loader = exif_loader_new();
	exif_loader_write_file(loader, raw_get_path(raw_file));
	exifData = exif_loader_get_data(loader);
	exif_loader_unref(loader);

	thumbnail->data = malloc(exifData->size);
	memcpy(thumbnail->data, exifData->data, exifData->size);
	thumbnail->data_size = exifData->size;
	thumbnail->data_type = OR_DATA_TYPE_JPEG;
	thumbnail->thumb_size = OR_THUMB_SIZE_SMALL;

	exif_data_unref(exifData);

	return err;
}
