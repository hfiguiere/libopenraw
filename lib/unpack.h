/*
 * libopenraw - unpack.cpp
 *
 * Copyright (C) 2008 Hubert Figuiere
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


#ifndef __UNPACK_H__
#define __UNPACK_H__

#include <stdint.h>
#include <stddef.h>

#include <boost/noncopyable.hpp>

namespace OpenRaw {	namespace Internals {

	/** Unpack class. Because we need to maintain a state */
	class Unpack
		: public boost::noncopyable
	{
	public:
		Unpack(uint32_t w, uint32_t t);

		size_t block_size();
		size_t unpack_be12to16(uint8_t *dest, const uint8_t *src, size_t size);
	private:
		uint32_t m_w;
		uint32_t m_type;
	};

} }

#endif
