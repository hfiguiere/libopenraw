/*
 * libopenraw - stream.cpp
 *
 * Copyright (C) 2006-2016 Hubert Figuière
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


#include <libopenraw/consts.h>

#include "stream.hpp"
#include "exception.hpp"

namespace OpenRaw {
namespace IO {

Stream::Stream(const char *filename)
  : m_fileName(filename),
    m_error(OR_ERROR_NONE)
{
}

Stream::~Stream()
{
}

uint8_t Stream::readByte() noexcept(false)
{
  uint8_t theByte;
  int r = read(&theByte, 1);
  if (r != 1) {
    // TODO add the error code
    throw Internals::IOException("Stream::readByte() failed.");
  }
  return theByte;
}

}
}
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  tab-width:2
  c-basic-offset:2
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
