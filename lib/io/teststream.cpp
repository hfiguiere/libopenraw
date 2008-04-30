/*
 * libopenraw - teststream.cpp
 *
 * Copyright (C) 2006 Hubert Figuiere
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

/** @brief test the IO::Stream class */

#include <string.h>

#include <iostream>
#include <cstdlib>

#include "stream.h"
#include "file.h"
#include "streamclone.h"

using namespace OpenRaw;

int main (int /*argc*/, char ** /*argv*/)
{
	IO::File *file = new IO::File("testfile.tmp");
	char buf1[128];
	int ret = file->open();
	if (ret != 0) {
		std::cerr << "failed: " __FILE__ ": "  << __LINE__ << std::endl;
		std::cerr << "Couldn't open test file. Test skipped.\n";
		exit(0);
	}

	size_t r = file->read(buf1, 6);
	if (r != 6) {
		std::cerr << "failed: "  __FILE__ ": " << __LINE__ << std::endl;
		exit(1);
	}
	
	IO::StreamClone * clone = new IO::StreamClone(file, 2);

	ret = clone->open();
	if (ret != 0) {
		std::cerr << "failed: " __FILE__ ": "  << __LINE__ << std::endl;
		exit(1);
	}
	char buf2[128];
	r = file->read(buf2, 4);
	if (r != 4) {
		std::cerr << "failed: "  __FILE__ ": " << __LINE__ << std::endl;
		exit(1);
	}

	if (strncmp(buf1 + 2, buf2, 4) != 0) {
		std::cerr << "failed: "  __FILE__ ": " << __LINE__ << std::endl;
		exit(1);
	}
	clone->close();

	file->close();
}
