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


#include "unpack.h"
#include "debug.h"

namespace OpenRaw {	namespace Internals {

	using namespace Debug;

	size_t unpack_12to16(uint8_t *dest, size_t outsize, 
						 const uint8_t *src, size_t insize) 
	{
		size_t inleft = insize;
		size_t outleft = outsize;
		do {
			if(inleft && outleft) {
				*dest = (*src & 0xf0) >> 4;
				outleft--; dest++;
				if(outleft) {
					*dest = (*src & 0x0f) << 4;
					inleft--; src++;
				}
			}
			if(inleft && outleft) {
				*dest |= (*src & 0xf0) >> 4;
				outleft--; dest++;
				*dest = (*src & 0x0f);
				if(outleft) {
					inleft--; src++;
					outleft--; dest++;		
				}
			}
			if(inleft && outleft) {
				*dest = *src;
				inleft--; src++;
				outleft--; dest++;
			}
		} while(inleft && outleft);
		if(inleft) {
			Trace(DEBUG1) << "left " << inleft << " at the end.\n";
		}
		return outsize - outleft;
	}

} }
