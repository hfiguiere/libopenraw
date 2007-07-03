/*
 * libopenraw - cfa.cpp
 *
 * Copyright (C) 2007 Hubert Figuiere
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */



#include <iostream>
#include <libopenraw/libopenraw.h>
#include <libopenraw/debug.h>
#include <libopenraw++/thumbnail.h>
#include <libopenraw++/rawfile.h>
#include <libopenraw++/rawdata.h>

#include <boost/scoped_ptr.hpp>

using OpenRaw::Thumbnail;
using boost::scoped_ptr;

int
main(int argc, char** argv)
{
	bool keepCompressed = false;
	if (argc < 2) {
		std::cerr << "missing parameter" << std::endl;
		return 1;
	}

	int c;
	do {
		c = getopt(argc, argv, "r");
		if(c != -1) {
			if(c == 'r') {
				keepCompressed = true;
			}
		}
	} while(c != -1);

	OpenRaw::init();
	or_debug_set_level(DEBUG2);
	FILE * f;

	scoped_ptr<OpenRaw::RawFile> raw_file(OpenRaw::RawFile::newRawFile(argv[optind]));

	OpenRaw::RawData rdata;
	uint32_t options = (keepCompressed ? OR_OPTIONS_DONT_DECOMPRESS : 0);
	raw_file->getRawData(rdata, options);

	if(keepCompressed) {
		std::cout << "keep compressed" << std::endl;
	}
	std::cout << "data size = " << rdata.size() << std::endl;
	std::cout << "data type = " << rdata.dataType() << std::endl;

	if(rdata.dataType() == OR_DATA_TYPE_CFA) {
		f = fopen("image.pgm", "wb");
		fprintf(f, "P5\n");
		fprintf(f, "%d %d\n", rdata.x(), rdata.y());
		fprintf(f, "%d\n", (1 << rdata.bpc()) - 1);
	}
	else {
		f = fopen("image.cfa", "wb");
	}
	fwrite(rdata.data(), 1, rdata.size(), f);
	fclose(f);
	
	return 0;
}

