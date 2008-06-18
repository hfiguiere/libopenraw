/* -*- mode:c++; tab-width:4; c-basic-offset:4 -*- */
/*
 * libopenraw - nefdiffiterator.h
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

#ifndef __NEFDIFFITERATOR_H_
#define __NEFDIFFITERATOR_H_

#include "bititerator.h"
#include "huffman.h"

namespace OpenRaw {
namespace Internals {

class NefDiffIterator {
	BitIterator m_iter;
	HuffmanDecoder m_decoder;

	public:
	static const HuffmanNode Lossy12Bit[];
	static const HuffmanNode LossLess14Bit[];

	NefDiffIterator (const HuffmanNode* const, const void *);
	int get();
};

}
}

#endif
