/*
 * libopenraw - streamclone.cpp
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


#include <stdio.h>
#include <memory>
#include <string>

#include "io/stream.hpp"
#include "libopenraw/consts.h"

#include "streamclone.hpp"


namespace OpenRaw {
namespace IO {

StreamClone::StreamClone(const Stream::Ptr & clone,
                         off_t offset)
  : Stream(clone->get_path().c_str()),
    m_cloned(clone), m_offset(offset)
{

}

StreamClone::~StreamClone()
{
}


Stream::Error StreamClone::open()
{
  if (m_cloned == NULL) {
    set_error(OR_ERROR_CLOSED_STREAM);
    return OR_ERROR_CLOSED_STREAM;
  }
  m_cloned->seek(m_offset, SEEK_SET);
  //no-op
  //FIXME determine what is the policy for opening clone 
  //streams
  return OR_ERROR_NONE;
}

int StreamClone::close()
{
  m_cloned = NULL;
  return 0;
}


int StreamClone::seek(off_t offset, int whence)
{
  if (m_cloned == NULL) {
    set_error(OR_ERROR_CLOSED_STREAM);
    return -1;
  }
  if (whence == SEEK_SET) {
    offset += m_offset;
  }
  int new_pos = m_cloned->seek(offset, whence);
  new_pos -= m_offset;
  return new_pos;
}


int StreamClone::read(void *buf, size_t count)
{
  if (m_cloned == NULL) {
    set_error(OR_ERROR_CLOSED_STREAM);
    return -1;
  }
  return m_cloned->read(buf, count);
}


off_t StreamClone::filesize()
{
  if (m_cloned == NULL) {
    set_error(OR_ERROR_CLOSED_STREAM);
    return -1;
  }
  return m_cloned->filesize() - m_offset;
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
