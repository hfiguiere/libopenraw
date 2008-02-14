/*
 * libopenraw - unpack.cpp
 *
 * Copyright (C) 2008 Hubert Figuiere
 * Copyright (C) 2008 Novell, Inc.
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


#ifndef __UNPACK_H__
#define __UNPACK_H__

#include <stdint.h>
#include <stddef.h>

namespace OpenRaw {	namespace Internals {

	size_t unpack_be12to16(uint8_t *dest, size_t outsize, 
						 const uint8_t *src, size_t insize);

} }

#endif
