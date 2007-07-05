/*
 * Copyright (C) 2007 Hubert Figuiere
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */



#include <string>

#include <boost/test/auto_unit_test.hpp>
#include <boost/crc.hpp>      // for boost::crc_basic, boost::crc_optimal

#include <libopenraw++/rawdata.h>

#include "io/file.h"
#include "rawcontainer.h"
#include "jfifcontainer.h"
#include "ljpegdecompressor.h"
#include "ljpegdecompressor_priv.h"

using boost::unit_test::test_suite;
using OpenRaw::RawData;
using OpenRaw::IO::File;

std::string g_testfile;

using namespace OpenRaw::Internals;

void test_ljpeg()
{
	RawData *decompData;
	File *s = new File(g_testfile.c_str());
	RawContainer *container = new JFIFContainer(s, 0);

	LJpegDecompressor decompressor(s, container);

	decompData = decompressor.decompress();

	boost::crc_optimal<8, 0x1021, 0xFFFF, 0, false, false>  crc_ccitt2;
	const uint8_t * data = static_cast<uint8_t *>(decompData->data());
	size_t data_len = decompData->size();
	crc_ccitt2 = std::for_each( data, data + data_len, crc_ccitt2 );
	BOOST_CHECK_EQUAL(crc_ccitt2(), 0x49);

	delete decompData;
	delete container;
	delete s;
}



test_suite*
init_unit_test_suite( int argc, char * argv[] ) 
{
	test_suite* test = BOOST_TEST_SUITE("test ljpeg");
	
	if (argc == 1) {
		// no argument, lets run like we are in "check"
		const char * srcdir = getenv("srcdir");
		
		BOOST_ASSERT(srcdir != NULL);
		g_testfile = std::string(srcdir);
		g_testfile += "/ljpegtest1.jpg";
	}
	else {
		g_testfile = argv[1];
	}
	
	test->add(BOOST_TEST_CASE(&test_ljpeg));

	return test;
}
