/*
 * Copyright (C) 2008-2020 Hubert Figuiere
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

#include <stdio.h>

#include <boost/test/included/unit_test.hpp>

#include "libopenraw/rawfile.h"

boost::unit_test::test_suite* init_unit_test_suite(int, char**)
{
    return nullptr;
}

BOOST_AUTO_TEST_CASE(test_extensions)
{
    const char **exts = or_get_file_extensions();
    BOOST_CHECK(exts);

    int i = 0;
    while (*exts) {
        i++;
        exts++;
    }

    BOOST_CHECK(i == 15);
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
