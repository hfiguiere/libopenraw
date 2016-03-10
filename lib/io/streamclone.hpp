/* -*- Mode: C++ -*- */
/*
 * libopenraw - streamclone.h
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


#ifndef OR_INTERNALS_IO_STREAMCLONE_H_
#define OR_INTERNALS_IO_STREAMCLONE_H_

#include <stddef.h>
#include <sys/types.h>

#include "stream.hpp"

namespace OpenRaw {
namespace IO {

/** @brief cloned stream. Allow reading from a different offset
 */
class StreamClone
  : public Stream
{
public:
  StreamClone(const Stream::Ptr &clone, off_t offset);
  virtual ~StreamClone();

  StreamClone(const StreamClone& f) = delete;
  StreamClone & operator=(const StreamClone&) = delete;

  virtual Error open() override;
  virtual int close() override;
  virtual int seek(off_t offset, int whence) override;
  virtual int read(void *buf, size_t count) override;
  virtual off_t filesize() override;

private:

  Stream::Ptr m_cloned;
  off_t m_offset;
};

}
}

#endif
