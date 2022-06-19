/* -*- tab-width:4; c-basic-offset:4 -*- */
/*
 * libopenraw - bititerator.cpp
 *
 * Copyright (C) 2008 Rafael Avila de Espindola.
 * Copyright (C) 2022 Hubert Figuiere
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
#include <algorithm>
#include "bititerator.hpp"

namespace OpenRaw {
namespace Internals {

BitIterator::BitIterator(const uint8_t * const p, size_t size)
    : m_p(p)
    , m_size(size)
    , m_bitBuffer(0)
    , m_bitsOnBuffer(0)

{
}

void BitIterator::addByte(uint8_t byte)
{
    m_bitBuffer = (m_bitBuffer << 8) | byte;
    m_bitsOnBuffer += 8;
}

void BitIterator::load(size_t numBits)
{
	size_t numBytes = (numBits + 7) / 8;

	// align the bits on the right
	m_bitBuffer >>= (32 - m_bitsOnBuffer);

	// load the new bits from the right
	size_t i = 0;
	for (; i < numBytes && m_size > 0; ++i) {
		addByte(*m_p);
		++m_p;
		m_size--;
	}
	for (; i < numBytes; ++i) {
		addByte(0);
	}

    // align the bits on the left
    m_bitBuffer = m_bitBuffer << (32 - m_bitsOnBuffer);
}

uint32_t BitIterator::get(size_t n)
{
	uint32_t ret = peek(n);

	skip(n);

	return ret;
}

uint32_t BitIterator::peek(size_t n)
{
	assert(n <= 25);

	if (n == 0) {
		return 0;
	}

	if (n > m_bitsOnBuffer) {
		load(n - m_bitsOnBuffer);
	}

	assert(n <= m_bitsOnBuffer);

	return m_bitBuffer >> (32 - n);
}

void BitIterator::skip(size_t n)
{
	size_t num_bits = std::min(n, m_bitsOnBuffer);
	m_bitsOnBuffer -= num_bits;
	m_bitBuffer <<= num_bits;
}

}
}
