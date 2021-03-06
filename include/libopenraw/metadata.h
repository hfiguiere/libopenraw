/*
 * libopenraw - metadata.h
 *
 * Copyright (C) 2007-2020 Hubert Figuière
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

#include <stdint.h>
#include <stdbool.h>

#define INCLUDE_EXIF_
#include <libopenraw/exif.h>
#undef INCLUDE_EXIF_

#include <libopenraw/types.h>
#include <libopenraw/consts.h>

/** @defgroup metadata_api Metadata API
 * @ingroup public_api
 * @{
 */
#ifdef __cplusplus
extern "C" {
#endif

typedef struct _MetadataIterator* ORMetadataIteratorRef; /**< @brief A metadata iterator */

typedef struct _MetaValue* ORMetaValueRef; /**< @brief A metadata value */
typedef const struct _MetaValue* ORConstMetaValueRef; /**< @brief A const metadata value */

/** The meta data namespaces, 16 MSB of the index */
enum {
    META_NS_EXIF = (1 << 16), /**< EXIF namespace */
    META_NS_TIFF = (2 << 16)  /**< TIFF namespace */
};

/** @brief Mask the namespace out.*/
#define META_NS_MASKOUT(x) (x & 0xffff)
/** @brief Mask the index out.*/
#define META_INDEX_MASKOUT(x) (x & (0xffff<<16))

/** @brief Get the string out of the %MetaValue.
 *
 * @param idx Pass 0. @todo Remove the idx parameter.
 * @return A NUL terminated string. NULL if not found. The pointer is owned by the
 * %MetaValue.
 */
const char* or_metavalue_get_string(ORConstMetaValueRef value, uint32_t idx);

/** @brief Convert the %MetaValue to a string.
 *
 * @param full FALSE if the conversion should abridge the result.
 * @return A NUL terminated string. NULL if not found. The pointer is owned by the
 * %MetaValue.
 */
const char* or_metavalue_get_as_string(ORConstMetaValueRef value, bool full);

/** @brief Get the value count
 *
 * @return The value count.
 */
uint32_t or_metavalue_get_count(ORMetaValueRef value);

/** @brief Free the %MetaValue */
void or_metavalue_release(ORMetaValueRef value);

/** @brief Move to the next metadata value
 * @param iterator The metadata iterator.
 * @return 0 if no more.
 */
int or_metadata_iterator_next(ORMetadataIteratorRef iterator);

/** @brief Get the metadata entry from the iterator.
 *
 * @param iterator The iterator.
 * @param ifd Pointer to the IfdDirRef.
 * @param id Pointer to id (nullable)
 * @param type Pointer to exif tag type (nullable)
 * @param value Pointer to store a newly allocated ORConstMetaValue (nullable)
 * @return 0 if error. In that case none of the values are valid.
 */
int
or_metadata_iterator_get_entry(ORMetadataIteratorRef iterator,
                               ORIfdDirRef* ifd, uint16_t* id,
                               ExifTagType* type, ORMetaValueRef* value);

/** @brief Free the iterator
 * @param iterator The iterator to free.
 */
void or_metadata_iterator_free(ORMetadataIteratorRef iterator);

#ifdef __cplusplus
}
#endif

/** @} */
