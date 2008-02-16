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
#include "ifd.h"

namespace OpenRaw {	namespace Internals {

	using namespace Debug;

	Unpack::Unpack(uint32_t w, uint32_t h, uint32_t t)
		: m_w(w), m_h(h),
		  m_col(0), m_row(0),
		  m_type(t)
	{
	}

	size_t Unpack::block_size()
	{
		size_t bs;
		if(m_type == IFD::COMPRESS_NIKON_PACK) {
			bs = (m_w / 2 * 3) + (m_w / 10);
		}
		else {
			bs = m_w / 2 * 3;
		}
		return bs;
	}


	size_t Unpack::row_advance()
	{ 
		size_t skip_input = 0;
		if((m_type == IFD::COMPRESS_NIKON_PACK) && ((m_col % 10) == 9)) {
			// skip one byte.
			skip_input = 1;
		}
		m_col++; 
		if(m_col == m_w) {
			m_col = 0;
			m_row++;
		}
		return skip_input;
	}


	/** source is in BE byte order 
	 * the output is always 16-bits values in native (host) byte order.
	 */
	size_t Unpack::unpack_be12to16(uint8_t *dest, size_t outsize, 
								   const uint8_t *src, size_t insize) 
	{
		size_t skip;
		size_t inleft = insize;
		size_t outleft = outsize;
		uint16_t short_dest;
		if(inleft<= 0) {
			return 0;
		}
		do {
			if(inleft && outleft) {
				short_dest = ((*src & 0xf0) >> 4) << 8;
				outleft--;
				if(outleft) {
					short_dest |= (*src & 0x0f) << 4;
					inleft--; src++;
				}
			}
			if(inleft && outleft) {
				short_dest |= (*src & 0xf0) >> 4;
				*(uint16_t*)dest = short_dest;
				outleft--; dest+=2;
				skip = row_advance();
				if(skip) {
					src += skip;
					inleft -= skip;
				}
			}
			if(inleft && outleft) {
				short_dest = (*src & 0x0f) << 8;
				inleft--; src++;
				outleft--;
			}
			if(inleft && outleft) {
				short_dest |= *src;
				*(uint16_t*)dest = short_dest;				
				inleft--; src++;
				outleft--; dest+=2;
				skip = row_advance();
				if(skip) {
					src += skip;
					inleft -= skip;
				}
			}
		} while(inleft && outleft);
		if(inleft) {
			Trace(WARNING) << "Left " << inleft << " bytes at the end.\n";
		}
		return outsize - outleft;
	}

} }
