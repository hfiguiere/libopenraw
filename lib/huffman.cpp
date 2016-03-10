/* -*- tab-width:4; c-basic-offset:4 -*- */
/*
 * libopenraw - huffman.cpp
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

#include <string>
#include <iostream>
#include "huffman.hpp"
#include "bititerator.hpp"

namespace OpenRaw {
namespace Internals {

void HuffmanDecoder::printTable_(std::string prefix, unsigned int pos)  const
{
	const HuffmanNode &cur = m_p[pos];
	if (cur.isLeaf) {
		std::cerr << prefix << " " << cur.data << "\n";
	} else {
		printTable_(prefix + "0", pos + 1);
		printTable_(prefix + "1", cur.data);
	}
}

HuffmanDecoder::HuffmanDecoder(const HuffmanNode* const p) : m_p(p)
{
}

void HuffmanDecoder::printTable() const
{
	printTable_("", 0);
}

unsigned int HuffmanDecoder::decode(BitIterator& i)
{
	int cur = 0;
	while (!m_p[cur].isLeaf) {
		unsigned int bit = i.get(1);
		if (bit)
			cur = m_p[cur].data;
		else
			cur = cur + 1;
	}
	return m_p[cur].data;
}

}
}
