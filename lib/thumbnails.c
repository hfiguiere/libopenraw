/*
 * libopenraw - thumbnails.c
 *
 * Copyright (C) 2005 Hubert Figuiere
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

#include <stdlib.h>
#include <string.h>

#include "cr2.h"
#include "thumbnails.h"

#include <libopenraw/libopenraw.h>


ORThumbnailRef 
or_thumbnail_new(void)
{
    ORThumbnailRef obj;

    obj = (ORThumbnailRef)malloc(sizeof(struct _ORThumbnail));
    if (obj) {
	    memset(obj, 0, sizeof(struct _ORThumbnail));
    }
    return obj;
}


or_error 
or_thumbnail_release(ORThumbnailRef thumb)
{
    if (thumb == NULL) {
        return OR_ERROR_NOTAREF;
    }
    if (thumb->data) {
        free(thumb->data);
    }
    free(thumb);
    return OR_ERROR_NONE;
}

or_error 
or_get_extract_thumbnail(const char* filename,
						 or_thumb_size preferred_size,
						 ORThumbnailRef *thumbnail)
{
    or_error err = OR_ERROR_NONE;
    RawFileRef raw_file;

    if (*thumbnail == NULL) {
        *thumbnail = or_thumbnail_new();
    }

    raw_file = raw_open(get_default_io_methods(), filename, O_RDONLY);

    cr2_get_thumbnail(raw_file, *thumbnail);

    raw_close(raw_file);
    raw_file = NULL;
    
    return err;
}


or_data_type
or_thumbnail_format(ORThumbnailRef thumb)
{
	or_data_type format = OR_DATA_TYPE_NONE;
	if (thumb) {
		format = thumb->data_type;
	}
	else {
		/* FIXME handle the error */
	}

	return format;
}


int
or_thumbnail_size(ORThumbnailRef thumb)
{
	if (thumb == NULL) {
		return 0;
	}
	return thumb->data_size;
}

void *
or_thumbnail_data(ORThumbnailRef thumb)
{
	if (thumb == NULL) {
		return NULL;
	}
	return thumb->data;
}
