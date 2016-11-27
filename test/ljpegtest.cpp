/*
 * Copyright (C) 2007-2016 Hubert Figuiere
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

#include <boost/test/minimal.hpp>
#include <boost/crc.hpp>      // for boost::crc_basic, boost::crc_optimal

#include "rawdata.hpp"
#include "io/file.hpp"
#include "rawcontainer.hpp"
#include "jfifcontainer.hpp"
#include "ljpegdecompressor.hpp"
#include "ljpegdecompressor_priv.hpp"


using OpenRaw::IO::File;

std::string g_testfile;

using namespace OpenRaw::Internals;

int test_main(int argc, char *argv[])
{
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

	File::Ptr s(new File(g_testfile.c_str()));
	RawContainer *container = new JfifContainer(s, 0);

	LJpegDecompressor decompressor(s.get(), container);

	OpenRaw::RawDataPtr decompData = decompressor.decompress();

	boost::crc_optimal<16, 0x1021, 0xFFFF, 0, false, false>  crc_ccitt2;
	const uint8_t * data = static_cast<uint8_t *>(decompData->data());
	size_t data_len = decompData->size();
	crc_ccitt2 = std::for_each( data, data + data_len, crc_ccitt2 );

	BOOST_CHECK(crc_ccitt2() == 0x20cc);

	delete container;

	return 0;
}

