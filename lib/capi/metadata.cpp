/*
 * libopenraw - metadata.cpp
 *
 * Copyright (C) 2016 Hubert Figuiere
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

#include "metavalue.hpp"

extern "C" {

const char*
or_metavalue_get_string(ORConstMetaValueRef value, uint32_t idx)
{
  // TODO validate parameters
  return reinterpret_cast<const OpenRaw::MetaValue*>(value)->getString(idx).c_str();
}

}
