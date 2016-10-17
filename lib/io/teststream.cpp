/*
 * libopenraw - teststream.cpp
 *
 * Copyright (C) 2006-2016 Hubert Figuiere
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

#include "stream.hpp"
#include "file.hpp"
#include "streamclone.hpp"

using namespace OpenRaw;

std::string g_testfile;

int test_main (int argc, char * argv[])
{
    if (argc == 1) {
        // no argument, lets run like we are in "check"
        const char * srcdir = getenv("srcdir");

        BOOST_ASSERT(srcdir != NULL);
        g_testfile = std::string(srcdir);
        g_testfile += "/io/testfile.tmp";
    }
    else {
        g_testfile = argv[1];
    }
    auto file = IO::Stream::Ptr(new IO::File(g_testfile.c_str()));
    char buf1[128];
    int ret = file->open();
    BOOST_CHECK(ret == 0);

    size_t r = file->read(buf1, 6);
    BOOST_CHECK(r == 6);

    BOOST_CHECK(memcmp(buf1, "abcdef", 6) == 0);

    off_t file_size = file->filesize();
    BOOST_CHECK(file_size == 63);

    const off_t clone_offset = 2;

    auto clone = new IO::StreamClone(file, clone_offset);

    ret = clone->open();
    BOOST_CHECK(ret == 0);

    BOOST_CHECK(clone->filesize() == (file_size - clone_offset));

    char buf2[128];
    r = clone->read(buf2, 4);
    BOOST_CHECK(r == 4);

    BOOST_CHECK(strncmp(buf1 + clone_offset, buf2, 4) == 0);

    uint8_t c = file->readByte();

    BOOST_CHECK(c == 'g');

    // seek

    int new_pos = clone->seek(0, SEEK_CUR);
    BOOST_CHECK(new_pos == 5);

    new_pos = clone->seek(1, SEEK_CUR);
    BOOST_CHECK(new_pos == 6);

    new_pos = clone->seek(2, SEEK_SET);
    BOOST_CHECK(new_pos == 2);

    c = file->readByte();
    BOOST_CHECK(c == 'e');

    new_pos = clone->seek(0, SEEK_CUR);
    BOOST_CHECK(new_pos == 3);

    c = file->readByte();
    BOOST_CHECK(c == 'f');

    new_pos = clone->seek(-2, SEEK_END);
    BOOST_CHECK(new_pos == 59);

    c = file->readByte();
    BOOST_CHECK(c == 'Z');


    clone->close();
    delete clone;
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
