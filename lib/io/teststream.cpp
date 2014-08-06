/*
 * libopenraw - teststream.cpp
 *
 * Copyright (C) 2006-2014 Hubert Figuiere
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

#include <string>
#include <iostream>
#include <cstdlib>

#include <boost/test/minimal.hpp>

#include "stream.h"
#include "file.h"
#include "streamclone.h"

using namespace OpenRaw;

std::string g_testfile;

int test_main (int argc, char * argv[])
{
    if (argc == 1) {
        // no argument, lets run like we are in "check"
        const char * srcdir = getenv("srcdir");

        BOOST_ASSERT(srcdir != NULL);
        g_testfile = std::string(srcdir);
        g_testfile += "/testfile.tmp";
    }
    else {
        g_testfile = argv[1];
    }
    IO::File *file = new IO::File(g_testfile.c_str());
    char buf1[128];
    int ret = file->open();
    BOOST_CHECK(ret == 0);

    size_t r = file->read(buf1, 6);
    BOOST_CHECK(r == 6);

    BOOST_CHECK(memcmp(buf1, "abcdef", 6) == 0);

    IO::StreamClone * clone = new IO::StreamClone(file, 2);

    ret = clone->open();
    BOOST_CHECK(ret == 0);

    char buf2[128];
    r = file->read(buf2, 4);
    BOOST_CHECK(r == 4);

    BOOST_CHECK(strncmp(buf1 + 2, buf2, 4) == 0);

    uint8_t c = file->readByte();

    BOOST_CHECK(c == 'g');

    clone->close();

    file->close();
    return 0;
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
