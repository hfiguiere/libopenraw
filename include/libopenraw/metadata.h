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
 *
 * @brief Access to the metadata
 * @{
 */
#ifdef __cplusplus
extern "C" {
#endif

typedef struct _MetadataIterator* ORMetadataIteratorRef; /**< @brief A metadata iterator */

typedef struct _MetaValue* ORMetaValueRef; /**< @brief A metadata value */
typedef const struct _MetaValue* ORConstMetaValueRef; /**< @brief A const metadata value */
typedef const struct _Metadata* ORMetadataRef; /**< @brief A metadata. Includes key and value */

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
 * @return A NUL terminated string. NULL if not found. The pointer is owned by the
 * %MetaValue.
 */
const char* or_metavalue_get_string(ORConstMetaValueRef value);

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
uint32_t or_metavalue_get_count(ORConstMetaValueRef value);

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
 * @return nullptr if error. Or a ORMetadata, owned by the iterator,
 *   invalidated when calling next.
 */
ORMetadataRef
or_metadata_iterator_get_entry(ORMetadataIteratorRef iterator);

/** @brief Get the current IFD from the iterator.
 *
 * @param iterator The iterator.
 * @return nullptr if error. Or a ORIfdDirRef, owned by the iterator,
 *   invalidated when calling next.
 */
ORIfdDirRef
or_metadata_iterator_get_dir(ORMetadataIteratorRef iterator);

/** @brief Free the iterator
 * @param iterator The iterator to free.
 */
void or_metadata_iterator_free(ORMetadataIteratorRef iterator);


const char* or_metadata_get_key(ORMetadataRef metadata);
ORConstMetaValueRef or_metadata_get_value(ORMetadataRef metadata);
int16_t or_metadata_get_type(ORMetadataRef metadata);

#ifdef __cplusplus
}
#endif

/** @} */
