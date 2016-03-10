/* -*- mode:c++; tab-width:4; c-basic-offset:4 -*- */
/*
 * libopenraw - nefdiffiterator.h
 *
 * Copyright (C) 2008 Rafael Avila de Espindola.
 * Copyright (C) 2013-2016 Hubert Figuiere
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

#ifndef OR_INTERNALS_NEFDIFFITERATOR_H_
#define OR_INTERNALS_NEFDIFFITERATOR_H_

#include <stddef.h>
#include <stdint.h>

#include "bititerator.hpp"
#include "huffman.hpp"

namespace OpenRaw {
namespace Internals {

class NefDiffIterator {
	BitIterator m_iter;
	HuffmanDecoder m_decoder;

	public:
	static const HuffmanNode Lossy12Bit[];
	static const HuffmanNode Lossy14Bit[];
	static const HuffmanNode LossLess14Bit[];

	NefDiffIterator (const HuffmanNode* const, const uint8_t *, size_t size);
	int get();
};

}
}

#endif
