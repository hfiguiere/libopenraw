/* -*- mode:c++; tab-width:4; c-basic-offset:4 -*- */
/*
 * libopenraw - huffman.h
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

#ifndef OR_INTERNALS_HUFFMAN_H_
#define OR_INTERNALS_HUFFMAN_H_

#include <string>

namespace OpenRaw {
namespace Internals {

class BitIterator;

struct HuffmanNode {
	unsigned isLeaf :1;
	unsigned data :31;
};

class HuffmanDecoder {
	unsigned int m_numNodes;
	const HuffmanNode * const m_p;

	void printTable_(std::string, unsigned int) const;
public:
	HuffmanDecoder(const HuffmanNode* const);
	void printTable() const;
	unsigned int decode(BitIterator& i);
};

}
}

#endif
