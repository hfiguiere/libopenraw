/* -*- Mode: C++ -*- */
/*
 * libopenraw - memstream.h
 *
 * Copyright (C) 2007-2016 Hubert Figuière
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

#ifndef OR_INTERNALS_IO_MEMSTREAM_H_
#define OR_INTERNALS_IO_MEMSTREAM_H_

#include <stddef.h>
#include <sys/types.h>

#include <libopenraw/consts.h>

#include "io/stream.hpp"

namespace OpenRaw {
namespace IO {

class MemStream
  : public Stream
{
public:
  MemStream(void *ptr, size_t s);

  virtual ~MemStream()
    {}

  MemStream(const MemStream& f) = delete;
  MemStream & operator=(const MemStream&) = delete;

  virtual or_error open() override;
  virtual int close() override;
  virtual int seek(off_t offset, int whence) override;
  virtual int read(void *buf, size_t count) override;
  virtual off_t filesize() override;


private:
  void * m_ptr;
  size_t m_size;
  unsigned char * m_current;
};

}
}

#endif
