/* -*- tab-width:4; indent-tabs-mode:'t c-file-style:"stroustrup" -*- */
/*
 * Copyright (C) 2008 Novell, Inc.
 * Copyright (C) 2009-2016 Hubert Figuiere
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


#include <boost/test/minimal.hpp>

#include "unpack.hpp"
#include "ifd.hpp"


int test_unpack()
{
	const uint8_t packed[32] = {0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF,
								0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0x00,
								0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0xEF,
								0x12, 0x34, 0x56, 0x78, 0x90, 0xAB, 0xCD, 0x00};
	uint16_t unpacked[20];

	OpenRaw::Internals::Unpack
		unpack(32, OpenRaw::Internals::IFD::COMPRESS_NIKON_PACK);

	size_t s;
	or_error err = unpack.unpack_be12to16((uint8_t*)unpacked, 40, packed,
									  sizeof(packed), s);
	BOOST_CHECK(s = size_t(sizeof(unpacked)));
	BOOST_CHECK(err == OR_ERROR_NONE);
	for (size_t i = 0; i < 2; ++i) {
		BOOST_CHECK(unpacked[10 * i + 0] == 0x0123);
		BOOST_CHECK(unpacked[10 * i + 1] == 0x0456);
		BOOST_CHECK(unpacked[10 * i + 2] == 0x0789);
		BOOST_CHECK(unpacked[10 * i + 3] == 0x00AB);
		BOOST_CHECK(unpacked[10 * i + 4] == 0x0CDE);
		BOOST_CHECK(unpacked[10 * i + 5] == 0x0F12);
		BOOST_CHECK(unpacked[10 * i + 6] == 0x0345);
		BOOST_CHECK(unpacked[10 * i + 7] == 0x0678);
		BOOST_CHECK(unpacked[10 * i + 8] == 0x090A);
		BOOST_CHECK(unpacked[10 * i + 9] == 0x0BCD);
	}
	return 0;
}

int test_unpack2()
{
	const uint8_t packed[3] = {0x12, 0x34, 0x56};
	uint16_t unpacked[2];

	OpenRaw::Internals::Unpack unpack(32,
									  OpenRaw::Internals::IFD::COMPRESS_NONE);

	size_t s;
	or_error err = unpack.unpack_be12to16((uint8_t*)unpacked, 4, packed,
									  sizeof(packed), s);
	BOOST_CHECK(s == size_t(sizeof(unpacked)));
	BOOST_CHECK(err == OR_ERROR_NONE);
	BOOST_CHECK(unpacked[0] == 0x0123);
	BOOST_CHECK(unpacked[1] == 0x0456);
	return 0;
}

int test_main( int /*argc*/, char * /*argv*/[] ) 
{
	test_unpack();
	test_unpack2();
	return 0;
}
