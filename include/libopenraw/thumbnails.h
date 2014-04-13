/*
 * libopenraw - thumbnails.h
 *
 * Copyright (C) 2005-2006, 2012 Hubert Figuière
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
/**
 * @brief the libopenraw public API header for thumbnails
 * @author Hubert Figuière <hub@figuiere.net>
 */


#ifndef LIBOPENRAW_THUMBNAILS_H_
#define LIBOPENRAW_THUMBNAILS_H_

#include <stdlib.h>

#include <libopenraw/types.h>
#include <libopenraw/consts.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct _Thumbnail *ORThumbnailRef;

	/** Extract thumbnail for raw file
	 *	@param filename the file name to extract from
	 *	@param preferred_size preferred thumnail size
	 *	@param thumb the thumbnail object ref to store it in
	 *	If the ref is NULL, then a new one is allocated. It is
	 *	the responsibility of the caller to release it.
	 *	@return error code
	 */
	or_error or_get_extract_thumbnail(const char* filename,
					 uint32_t preferred_size,
					 ORThumbnailRef *thumb);
	
	/** Allocate a thumbnail
	 */
	extern ORThumbnailRef 
	or_thumbnail_new(void);

	/** Release a thumbnail object
	 */
	extern or_error 
	or_thumbnail_release(ORThumbnailRef thumb);

	/** Return the thumbnail format
	 */
	extern or_data_type 
	or_thumbnail_format(ORThumbnailRef thumb);

	extern void *
	or_thumbnail_data(ORThumbnailRef thumb);

	extern size_t
	or_thumbnail_data_size(ORThumbnailRef thumb);

	extern void
	or_thumbnail_dimensions(ORThumbnailRef thumb, 
													uint32_t *x, uint32_t *y);

#ifdef __cplusplus
}
#endif


#endif
