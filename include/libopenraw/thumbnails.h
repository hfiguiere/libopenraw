/*
 * libopenraw - thumbnails.h
 *
 * Copyright (C) 2005-2020 Hubert Figuière
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
 * @file the libopenraw public API header for thumbnails
 * @author Hubert Figuière <hub@figuiere.net>
 */


#ifndef LIBOPENRAW_THUMBNAILS_H_
#define LIBOPENRAW_THUMBNAILS_H_

#include <stdlib.h>

#include <libopenraw/types.h>
#include <libopenraw/consts.h>

/** @defgroup thumbnails_api Thumbnail API
 * @ingroup public_api
 * @{
 */
#ifdef __cplusplus
extern "C" {
#endif

/** @brief Extract thumbnail for raw file
 *
 * See %or_rawfile_get_thumbnail() for details.
 * Will return %OR_ERROR_CANT_OPEN if the file can't be open.
 *
 * @param filename The path to the file to extract from.
 * @param preferred_size Preferred thumbnail size.
 * @param [in/out] thumb The thumbnail object ref to store it in
 * If the ref is NULL, then a new one is allocated. It is
 * the responsibility of the caller to release it.
 * @return An error code.
 */
or_error or_get_extract_thumbnail(const char* filename,
                                  uint32_t preferred_size,
                                  ORThumbnailRef *thumb);

/** @brief Allocate a Thumbnail object.
 *
 * @return A Thumbnail object. Use %or_thumbnail_release() to free it.
 */
ORThumbnailRef or_thumbnail_new(void);

/** @brief Release a Thumbnail object.
 *
 * @param thumb The Thumbnail objet to release.
 * @return An error code. %OR_ERROR_NONE in case of success, and %OR_ERROR_NOTAREF
 * if a NULL pointer is passed.
 */
or_error or_thumbnail_release(ORThumbnailRef thumb);

/** @brief Get the thumbnail format.
 *
 * @return A data type indicating the format.
 */
or_data_type or_thumbnail_format(ORThumbnailRef thumb);

/** @brief Get the pointer to the data.
 *
 * See %or_thumbnail_data_size() to know the size.
 *
 * @return A pointer, owned by the %Thumbnail object. May be NULL.
 */
void* or_thumbnail_data(ORThumbnailRef thumb);

/** @brief Get the data size.
 *
 * @return The data size.
 */
size_t or_thumbnail_data_size(ORThumbnailRef thumb);

/** @brief Get the %Thumbnail dimensions in pixels.
 *
 * @param [out] x The horizontal dimension. Can be NULL.
 * @param [out] y The vertical dimension. Can be NULL.
 */
void or_thumbnail_dimensions(ORThumbnailRef thumb, uint32_t *x, uint32_t *y);

#ifdef __cplusplus
}
#endif
/** @} */

#endif
