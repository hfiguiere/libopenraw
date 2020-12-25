/* -*- mode:c++; indent-tabs-mode:nil; c-basic-offset:4; tab-width:4; -*- */
/*
 * libopenraw - debug.cpp
 *
 * Copyright (C) 2006-2019 Hubert Figuière
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

#include <libopenraw/debug.h>

#include "capi.h"
#include "trace.hpp"

#ifdef __cplusplus
extern "C" {
#endif

API_EXPORT
void or_debug_set_level(debug_level lvl)
{
    Debug::Trace::setDebugLevel(lvl);
}


#ifdef __cplusplus
}
#endif
