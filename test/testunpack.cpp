/*
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


#include <boost/test/auto_unit_test.hpp>

#include "unpack.h"

using boost::unit_test::test_suite;


void test_unpack()
{
	const uint8_t packed[] = {0x12, 0x34, 0x56, 0x78, 0x90, 0xAB };
	uint16_t unpacked[4];

	OpenRaw::Internals::Unpack unpack(10, 10, 1);

	size_t s = unpack.unpack_be12to16((uint8_t*)unpacked, 8, 
									  packed, 6);
	BOOST_CHECK_EQUAL(s, (size_t)8);
	BOOST_CHECK_EQUAL(unpacked[0], 0x0123);
	BOOST_CHECK_EQUAL(unpacked[1], 0x0456);
	BOOST_CHECK_EQUAL(unpacked[2], 0x0789);
	BOOST_CHECK_EQUAL(unpacked[3], 0x00AB);
}


test_suite*
init_unit_test_suite( int /*argc*/, char ** /*argv*/ ) 
{
	test_suite* test = BOOST_TEST_SUITE("test unpack");
	
	test->add(BOOST_TEST_CASE(&test_unpack));

	return test;
}

