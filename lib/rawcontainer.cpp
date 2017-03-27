/* -*- Mode: C++; c-basic-offset:4; tab-width:4; indent-tabs-mode:nil -*- */
/*
 * libopenraw - rawcontainer.cpp
 *
 * Copyright (C) 2006-2017 Hubert Figuière
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

#include <fcntl.h>
#include <memory>

#include <libopenraw/debug.h>

#include "trace.hpp"
#include "endianutils.hpp"
#include "rawcontainer.hpp"

using namespace Debug;

namespace OpenRaw {
namespace Internals {


RawContainer::RawContainer(const IO::Stream::Ptr &_file, off_t _offset)
  : m_file(_file),
    m_offset(_offset),
    m_endian(ENDIAN_NULL)
{
  m_file->open();
  m_file->seek(_offset, SEEK_SET);
}


RawContainer::~RawContainer()
{
  m_file->close();
}

bool RawContainer::skip(off_t offset)
{
  m_file->seek(offset, SEEK_CUR);
  return true;
}

Option<int8_t>
RawContainer::readInt8(const IO::Stream::Ptr &f)
{
  unsigned char buf;
  int s = f->read(&buf, 1);
  if (s != 1) {
    return Option<int8_t>();
  }
  return Option<int8_t>(buf);
}

Option<uint8_t>
RawContainer::readUInt8(const IO::Stream::Ptr &f)
{
  unsigned char buf;
  int s = f->read(&buf, 1);
  if (s != 1) {
    return Option<uint8_t>();
  }
  return Option<uint8_t>(buf);
}

Option<int16_t>
RawContainer::readInt16(const IO::Stream::Ptr &f)
{
  if (m_endian == ENDIAN_NULL) {

    LOGERR("null endian\n");

    return Option<int16_t>();
  }
  unsigned char buf[2];
  int s = f->read(buf, 2);
  if (s != 2) {
    return Option<int16_t>();
  }
  if (m_endian == ENDIAN_LITTLE) {
    return Option<int16_t>(EL16(buf));
  } else {
    return Option<int16_t>(BE16(buf));
  }
}


/**
 * Return the number of element read.
 */
size_t
RawContainer::readUInt16Array(const IO::Stream::Ptr &f, std::vector<uint16_t> & v, size_t count)
{
  if (m_endian == ENDIAN_NULL) {
    LOGERR("null endian\n");
    return 0;
  }

  if (v.size() < count) {
    v.resize(count, 0);
  }
  uint8_t buf[2];
  size_t num_read = 0;
  for (size_t i = 0; i < count; i++) {
    int s = f->read(buf, 2);
    uint16_t val;
    if (s != 2) {
      return num_read;
    }
    if (m_endian == ENDIAN_LITTLE) {
      val = EL16(buf);
    } else {
      val = BE16(buf);
    }
    v[i] = val;
    num_read++;
  }

  return num_read;
}


Option<int32_t>
RawContainer::readInt32(const IO::Stream::Ptr &f)
{
  if (m_endian == ENDIAN_NULL) {
    LOGERR("null endian\n");
    return Option<int32_t>();
  }
  unsigned char buf[4];
  int s = f->read(buf, 4);
  if (s != 4) {
    LOGERR("read %d bytes\n", s);
    return Option<int32_t>();
  }

  if (m_endian == ENDIAN_LITTLE) {
    return Option<int32_t>(EL32(buf));
  } else {
    return Option<int32_t>(BE32(buf));
  }
}


Option<uint16_t>
RawContainer::readUInt16(const IO::Stream::Ptr &f)
{
  if (m_endian == ENDIAN_NULL) {

    LOGERR("null endian\n");

    return Option<uint16_t>();
  }
  unsigned char buf[2];
  int s = f->read(buf, 2);
  if (s != 2) {
    return Option<uint16_t>();
  }
  if (m_endian == ENDIAN_LITTLE) {
    return Option<uint16_t>(EL16(buf));
  } else {
    return Option<uint16_t>(BE16(buf));
  }
}


Option<uint32_t>
RawContainer::readUInt32(const IO::Stream::Ptr &f)
{
  if (m_endian == ENDIAN_NULL) {
    LOGERR("null endian\n");

    return Option<uint32_t>();
  }
  unsigned char buf[4];
  int s = f->read(buf, 4);
  if (s != 4) {
    return Option<uint32_t>();
  }

  if (m_endian == ENDIAN_LITTLE) {
    return Option<uint32_t>(EL32(buf));
  } else {
    return Option<uint32_t>(BE32(buf));
  }
}

size_t
RawContainer::fetchData(void *buf, off_t _offset,
                        size_t buf_size)
{
  size_t s = 0;
  m_file->seek(_offset, SEEK_SET);
  s = m_file->read(buf, buf_size);
  return s;
}

off_t
RawContainer::size() const
{
  return m_file->filesize();
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
