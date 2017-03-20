/*
 * libopenraw - rw2container.cpp
 *
 * Copyright (C) 2011-2017 Hubert Figuière
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

#include <libopenraw/debug.h>

#include "trace.hpp"
#include "rw2container.hpp"

using namespace Debug;

namespace OpenRaw {

namespace Internals {

Rw2Container::Rw2Container(const IO::Stream::Ptr &_file, off_t _offset)
    : IfdFileContainer(_file, _offset), subtype_(0)
{
}

Rw2Container::~Rw2Container()
{
}

IfdFileContainer::EndianType Rw2Container::isMagicHeader(const char *p, int len)
{
    if (len < 4) {
        // we need at least 4 bytes to check
        return ENDIAN_NULL;
    }
    if ((p[0] == 'I') && (p[1] == 'I')) {
        if ((p[2] == 'U') && (p[3] == 0)) {

            LOGDBG1("Identified EL RW2 file.\n");
            return ENDIAN_LITTLE;
        }
    }

    LOGERR("Unidentified RW2 file\n");

    return ENDIAN_NULL;
}
}
}
