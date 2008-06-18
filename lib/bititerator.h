/* -*- tab-width:4; c-basic-offset:4 -*- */
/*
 * libopenraw - bititerator.cpp
 *
 * Copyright (C) 2008 Rafael Avila de Espindola.
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

#ifndef __BITITERATOR_H_
#define __BITITERATOR_H_

#include <stdint.h>
#include <cstring>

namespace OpenRaw {
namespace Internals {

class BitIterator {
	const uint8_t* m_p;
	uint32_t m_bitBuffer;
	uint8_t m_bitsOnBuffer;
	void load(size_t numBits);

 public:
	BitIterator(const void *);
	uint32_t get(size_t);
};

}
}

#endif
