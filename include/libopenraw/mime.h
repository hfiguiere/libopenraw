/*
 * libopenraw - mime.h
 *
 * Copyright (C) 2024 Hubert Figuière
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


#pragma once

#include <libopenraw/consts.h>

/** @defgroup mime_api MIME API
 * @ingroup public_api
 *
 * @brief MIME identification API
 * @{
 */
#ifdef __cplusplus
extern "C" {
#endif

/** @brief Return a NULL terminated list of mime type for raw files that the
 * library supposedly handle. This excludes JPEG.
 *
 * @return A NULL terminated list. Owned by the library.
 */
const char** or_get_mime_types();

/** @brief Get the %or_rawtype_file for the mime_type
 *
 * @param mime_type The MIME type
 *
 * @return The type or %OR_RAWFILE_TYPE_UNKNOWN if unknown.
 */
or_rawfile_type or_get_type_for_mime_type(const char* mime_type);

#ifdef __cplusplus
}
#endif

/** @} */
