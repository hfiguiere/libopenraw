/*
 * libopenraw - metadata.cpp
 *
 * Copyright (C) 2016-2020 Hubert Figuière
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

#include <libopenraw/metadata.h>

#include "capi.h"
#include "metavalue.hpp"
#include "metadata.hpp"
#include "ifddir.hpp"

using OpenRaw::Internals::IfdDir;

extern "C" {

/** check pointer validity */
#define CHECK_PTR(p, r)                                                        \
    if (p == nullptr) {                                                           \
        return r;                                                              \
    }

API_EXPORT const char*
or_metavalue_get_string(ORConstMetaValueRef value, uint32_t idx)
{
  // TODO validate parameters
  return reinterpret_cast<const OpenRaw::MetaValue*>(value)->getString(idx).c_str();
}

API_EXPORT const char*
or_metavalue_get_as_string(ORConstMetaValueRef value)
{
  // TODO validate parameters
  return reinterpret_cast<const OpenRaw::MetaValue*>(value)->getAsString().c_str();
}

API_EXPORT void
or_metavalue_release(ORMetaValueRef value)
{
  if (!value) {
    return;
  }
  auto obj = reinterpret_cast<OpenRaw::MetaValue*>(value);
  delete obj;
}


API_EXPORT int
or_metadata_iterator_next(ORMetadataIteratorRef iterator)
{
  CHECK_PTR(iterator, 0);
  auto iter = reinterpret_cast<OpenRaw::MetadataIterator*>(iterator);
  if (iter && iter->next()) {

    return 1;
  }
  return 0;
}

API_EXPORT int
or_metadata_iterator_get_entry(ORMetadataIteratorRef iterator,
                               ORIfdDirRef* ifd, uint16_t* id,
                               ExifTagType* type, ORMetaValueRef* value)
{
  CHECK_PTR(iterator, 0);
  auto iter = reinterpret_cast<OpenRaw::MetadataIterator*>(iterator);
  if (ifd) {
    auto t = iter->getIfd();
    if (t) {
      auto wrap = new WrappedPointer<IfdDir>(t);
      *ifd = reinterpret_cast<ORIfdDirRef>(wrap);
    } else {
      return 0;
    }
  }
  if (id) {
    auto i = iter->getEntryId();
    if (i) {
      *id = *i;
    } else {
      return 0;
    }
  }
  if (type) {
    auto t = iter->getEntryType();
    if (t) {
      *type = *t;
    } else {
      return 0;
    }
  }
  if (value) {
    auto v = iter->getMetaValue();
    if (v) {
      *value = reinterpret_cast<ORMetaValueRef>(v);
    } else {
      *value = nullptr;
      LOGERR("Couldn't get value\n");
    }
  }
  return 1;
}

API_EXPORT void
or_metadata_iterator_free(ORMetadataIteratorRef iterator)
{
  if (!iterator) {
    return;
  }
  auto iter = reinterpret_cast<OpenRaw::MetadataIterator*>(iterator);
  delete iter;
}

}
