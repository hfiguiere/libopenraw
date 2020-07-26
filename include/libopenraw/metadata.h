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

#define INCLUDE_EXIF_
#include <libopenraw/exif.h>
#undef INCLUDE_EXIF_

#include <libopenraw/types.h>
#include <libopenraw/consts.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _MetadataIterator* ORMetadataIteratorRef;

typedef struct _MetaValue* ORMetaValueRef;
typedef const struct _MetaValue* ORConstMetaValueRef;

/** The meta data namespaces, 16 high bits of the index */
enum {
	META_NS_EXIF = (1 << 16),
	META_NS_TIFF = (2 << 16)
};

#define META_NS_MASKOUT(x) (x & 0xffff)
#define META_INDEX_MASKOUT(x) (x & (0xffff<<16))

const char* or_metavalue_get_string(ORConstMetaValueRef value, uint32_t idx);
const char* or_metavalue_get_as_string(ORConstMetaValueRef value);

void or_metavalue_release(ORMetaValueRef value);

/** Get the next metadata value
 * @param iterator The iterator.
 * @return 0 if none
 */
int or_metadata_iterator_next(ORMetadataIteratorRef iterator);

/** Get the metadata entry
 * @param iterator The iterator.
 * @param ifd Pointer to the IfdDirRef.
 * @param id Pointer to id (nullable)
 * @param type Pointer to exif tag type (nullable)
 * @param value Pointer to store a newly allocated ORConstMetaValue (nullable)
 * @return 0 if error. In that case none of the values is valid.
 */
int
or_metadata_iterator_get_entry(ORMetadataIteratorRef iterator,
                               ORIfdDirRef* ifd, uint16_t* id,
                               ExifTagType* type, ORMetaValueRef* value);

/** Free the iterator
 * @param iterator The iterator.
 */
void or_metadata_iterator_free(ORMetadataIteratorRef iterator);

#ifdef __cplusplus
}
#endif
