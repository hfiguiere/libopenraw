/* -*- tab-width:4; c-basic-offset:4 -*- */
/*
 * libopenraw - nefdiffiterator.cpp
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

#include "nefdiffiterator.hpp"

namespace OpenRaw {
namespace Internals {

int NefDiffIterator::get()
{
	unsigned int t = m_decoder.decode(m_iter);
	unsigned int len = t & 15;
	unsigned int shl = t >> 4;


	unsigned int bits = m_iter.get(len - shl);

	int diff = ((bits << 1) + 1) << shl >> 1;
	if ((diff & (1 << (len-1))) == 0)
		diff -= (1 << len) - !shl;

	return diff;
}

// 00              5
// 010             4
// 011             3
// 100             6
// 101             2
// 110             7
// 1110            1
// 11110           0
// 111110          8
// 1111110         9
// 11111110        11
// 111111110       10
// 1111111110      12
// 1111111111      0
const HuffmanNode NefDiffIterator::Lossy12Bit[] = {
	/* 0  */ {0, 6},  /* root       */
	/* 1  */ {0, 3},  /* 0          */
	/* 2  */ {1, 5},  /* 00         */
	/* 3  */ {0, 5},  /* 01         */
	/* 4  */ {1, 4},  /* 010        */
	/* 5  */ {1, 3},  /* 011        */
	/* 6  */ {0, 10}, /* 1          */
	/* 7  */ {0, 9},  /* 10         */
	/* 8  */ {1, 6},  /* 100        */
	/* 9  */ {1, 2},  /* 101        */
	/* 10 */ {0, 12}, /* 11         */
	/* 11 */ {1, 7},  /* 110        */
	/* 12 */ {0, 14}, /* 111        */
	/* 13 */ {1, 1},  /* 1110       */
	/* 14 */ {0, 16}, /* 1111       */
	/* 15 */ {1, 0},  /* 11110      */
	/* 16 */ {0, 18}, /* 11111      */
	/* 17 */ {1, 8},  /* 111110     */
	/* 18 */ {0, 20}, /* 111111     */
	/* 19 */ {1, 9},  /* 1111110    */
	/* 20 */ {0, 22}, /* 1111111    */
	/* 21 */ {1, 11}, /* 11111110   */
	/* 22 */ {0, 24}, /* 11111111   */
	/* 23 */ {1, 10}, /* 111111110  */
	/* 24 */ {0, 26}, /* 111111111  */
	/* 25 */ {1, 12}, /* 1111111110 */
	/* 26 */ {1, 0},  /* 1111111111 */
};

// 00              5
// 010             6
// 011             4
// 100             7
// 101             8
// 1100            3
// 1101            9
// 11100           2
// 11101           1
// 111100          0
// 111101          10
// 111110          11
// 1111110         12
// 11111110        13
// 11111111        14
const HuffmanNode NefDiffIterator::Lossy14Bit[] = {
	/* 0  */ {0, 6},  /* root       */
	/* 1  */ {0, 3},  /* 0          */
	/* 2  */ {1, 5},  /* 00         */
	/* 3  */ {0, 5},  /* 01         */
	/* 4  */ {1, 6},  /* 010        */
	/* 5  */ {1, 4},  /* 011        */
	/* 6  */ {0, 10}, /* 1          */
	/* 7  */ {0, 9},  /* 10         */
	/* 8  */ {1, 7},  /* 100        */
	/* 9  */ {1, 8},  /* 101        */
	/* 10 */ {0, 14}, /* 11         */
	/* 11 */ {0, 13}, /* 110        */
	/* 12 */ {1, 3},  /* 1100       */
	/* 13 */ {1, 9},  /* 1101       */
	/* 14 */ {0, 18}, /* 111        */
	/* 15 */ {0, 17}, /* 1110       */
	/* 16 */ {1, 2},  /* 11100      */
	/* 17 */ {1, 1},  /* 11101      */
	/* 18 */ {0, 22}, /* 1111       */
	/* 19 */ {0, 21}, /* 11110      */
	/* 20 */ {1, 0},  /* 111100     */
	/* 21 */ {1, 10}, /* 111101     */
	/* 22 */ {0, 24}, /* 11111      */
	/* 23 */ {1, 11}, /* 111110     */
	/* 24 */ {0, 26}, /* 111111     */
	/* 25 */ {1, 12}, /* 1111110    */
	/* 26 */ {0, 28}, /* 1111111    */
	/* 27 */ {1, 13}, /* 11111110   */
	/* 28 */ {1, 14}, /* 11111111   */
};

// 00              7
// 010             6
// 011             8
// 100             5
// 101             9
// 1100            4
// 1101            10
// 11100           3
// 11101           11
// 111100          12
// 111101          2
// 111110          0
// 1111110         1
// 11111110        13
// 11111111        14
const HuffmanNode NefDiffIterator::LossLess14Bit[] = {
	/* 0  */ {0, 6},  /* root       */
	/* 1  */ {0, 3},  /* 0          */
	/* 2  */ {1, 7},  /* 00         */
	/* 3  */ {0, 5},  /* 01         */
	/* 4  */ {1, 6},  /* 010        */
	/* 5  */ {1, 8},  /* 011        */
	/* 6  */ {0, 10}, /* 1          */
	/* 7  */ {0, 9},  /* 10         */
	/* 8  */ {1, 5},  /* 100        */
	/* 9  */ {1, 9},  /* 101        */
	/* 10 */ {0, 14}, /* 11         */
	/* 11 */ {0, 13}, /* 110        */
	/* 12 */ {1, 4},  /* 1100       */
	/* 13 */ {1, 10}, /* 1101       */
	/* 14 */ {0, 18}, /* 111        */
	/* 15 */ {0, 17}, /* 1110       */
	/* 16 */ {1, 3},  /* 11100      */
	/* 17 */ {1, 11}, /* 11101      */
	/* 18 */ {0, 22}, /* 1111       */
	/* 19 */ {0, 21}, /* 11110      */
	/* 20 */ {1, 12}, /* 111100     */
	/* 21 */ {1, 2},  /* 111101     */
	/* 22 */ {0, 24}, /* 11111      */
	/* 23 */ {1, 0},  /* 111110     */
	/* 24 */ {0, 26}, /* 111111     */
	/* 25 */ {1, 1},  /* 1111110    */
	/* 26 */ {0, 28}, /* 1111111    */
	/* 27 */ {1, 13}, /* 11111110   */
	/* 28 */ {1, 14}, /* 11111111   */
};

NefDiffIterator::NefDiffIterator(const HuffmanNode* const t,
                                 const uint8_t *p, size_t size) :
	m_iter(p, size), m_decoder(t)
{
}

}
}
