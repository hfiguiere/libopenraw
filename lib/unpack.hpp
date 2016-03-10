/*
 * libopenraw - unpack.h
 *
 * Copyright (C) 2008-2013 Hubert Figuiere
 * Copyright (C) 2008 Novell, Inc.
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


#ifndef OR_INTERNALS_UNPACK_H_
#define OR_INTERNALS_UNPACK_H_

#include <stdint.h>
#include <stddef.h>

#include <libopenraw/consts.h>

namespace OpenRaw {	namespace Internals {

	/** Unpack class. Because we need to maintain a state */
	class Unpack
	{
	public:
		Unpack(uint32_t w, uint32_t t);
		// noncopyable
		Unpack(const Unpack&) = delete;
		Unpack & operator=(const Unpack&) = delete;

		size_t block_size();
		or_error unpack_be12to16(uint8_t *dest, size_t destsize, const uint8_t *src, size_t size, size_t & outsize);
	private:
		uint32_t m_w;
		uint32_t m_type;
	};

} }

#endif
