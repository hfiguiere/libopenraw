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

#include <assert.h>
#include "bititerator.h"

namespace OpenRaw {
namespace Internals {

BitIterator::BitIterator(const void * const p) :
	m_p(static_cast<const uint8_t * const>(p)),
	m_bitBuffer(0), m_bitsOnBuffer(0)

{
}

void BitIterator::load(size_t numBits)
{
	size_t numBytes = (numBits + 7) / 8;

	//align the bits on the right
	m_bitBuffer >>= (32 - m_bitsOnBuffer);

	m_bitsOnBuffer += 8 * numBytes;

	//load the new bits from the right
	for (size_t i = 0; i < numBytes; ++i) {
		m_bitBuffer = (m_bitBuffer << 8) | *m_p;
		++m_p;
	}

	//align the bits on the left
	m_bitBuffer = m_bitBuffer << (32 - m_bitsOnBuffer);
}

uint32_t BitIterator::get(size_t n)
{
	assert(n <= 25);

	if (n == 0)
		return 0;

	if (n > m_bitsOnBuffer)
		load(n - m_bitsOnBuffer);

	assert(n <= m_bitsOnBuffer);

	uint32_t ret = m_bitBuffer >> (32 - n);
	m_bitsOnBuffer -= n;
	m_bitBuffer <<= n;

	return ret;
}

}
}
