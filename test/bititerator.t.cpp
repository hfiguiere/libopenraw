/* -*- tab-width:4; c-basic-offset:4 -*- */
/*
 * libopenraw - bititerator.t.cpp
 *
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
/** @brief unit test for BitIterator */

#include <boost/test/included/unit_test.hpp>

#include "bititerator.hpp"

using OpenRaw::Internals::BitIterator;

boost::unit_test::test_suite* init_unit_test_suite(int, char**)
{
  return nullptr;
}

BOOST_AUTO_TEST_CASE(test_bititerator)
{
    uint8_t buffer[] = { 0xff, 0x10, 0x01, 0x22 };
    BitIterator bits(buffer, 4);

    auto t = bits.peek(9);
    BOOST_CHECK_EQUAL(t, 0x1fe);
    auto t2 = bits.get(9);
    BOOST_CHECK_EQUAL(t, t2);
    bits.skip(2);
    t = bits.peek(1);
    BOOST_CHECK_EQUAL(t, 1);
    bits.skip(1);
    t = bits.get(4);
    BOOST_CHECK_EQUAL(t, 0);
    t = bits.get(12);
    BOOST_CHECK_EQUAL(t, 0x12);

    // peek the last 4 bits
    t = bits.peek(4);
    BOOST_CHECK_EQUAL(t, 0x2);

    // peeking past the end.
    t = bits.peek(5);
    BOOST_CHECK_EQUAL(t, 0x4);
}
