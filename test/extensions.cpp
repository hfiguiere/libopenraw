/*
 * Copyright (C) 2008 Hubert Figuiere
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
#include <boost/test/minimal.hpp>

#include "libopenraw/rawfile.h"

#include <stdlib.h>

int test_main( int, char *[] )             // note the name!
{
    const char **exts = or_get_file_extensions();
    if(exts == NULL) {
        fprintf(stderr, "extension list is NULL\n");
        return 1;
    }
    int i = 0;
    while(*exts) {
        i++;
        exts++;
    }
    if(i != 13) {
        fprintf(stderr, "extension list has the wrong number: %d\n", i);
        return 1;
    }
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
