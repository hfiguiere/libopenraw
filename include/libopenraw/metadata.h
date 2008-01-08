/*
 * libopenraw - metadata.h
 *
 * Copyright (C) 2007 Hubert Figuiere
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */


#ifndef __LIBOPENRAW_METADATA_H_
#define __LIBOPENRAW_METADATA_H_

#define _INCLUDE_EXIF
#include <libopenraw/exif.h>
#undef _INCLUDE_EXIF

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _MetaValue *ORMetaValueRef;
typedef const struct _MetaValue *ORConstMetaValueRef;

/** The meta data namespaces, 16 high bits of the index */
enum {
	META_NS_EXIF = (1 << 16),
	META_NS_TIFF = (2 << 16)
};

#define META_NS_MASKOUT(x) (x & 0xffff)
#define META_INDEX_MASKOUT(x) (x & (0xffff<<16))

#ifdef __cplusplus
}
#endif

#endif
