/* -*- Mode: C++ -*- */
/*
 * libopenraw - jfifcontainer.h
 *
 * Copyright (C) 2006-2020 Hubert Figuière
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

#pragma once

#include <stdint.h>
#include <sys/types.h>
#include <setjmp.h>

#include <memory>

#include "ifddir.hpp"
#include "io/stream.hpp"
#include "rawcontainer.hpp"

extern "C" {
#include <jpeglib.h>
}

namespace OpenRaw {

class BitmapData;

namespace Internals {

class IfdFileContainer;

/**
 * @brief a JPEG container
 */
class JfifContainer
  : public RawContainer
{
public:
  JfifContainer(const IO::Stream::Ptr &file, off_t offset);
  /** destructor */
  virtual ~JfifContainer();

  bool getDimensions(uint32_t &x, uint32_t &y);
  bool getDecompressedData(BitmapData &data);

  /** Main ifd is 0 */
  IfdDir::Ref mainIfd();
  /** Return ifd at index */
  IfdDir::Ref getIfdDirAt(int idx);
  /** Return Exif ifd */
  IfdDir::Ref exifIfd();
  /** Return the ifd container */
  std::unique_ptr<IfdFileContainer> & ifdContainer();
  /** Return the exif offset to be added to any offset from the start of the JFIF */
  size_t exifOffset() const {
    return m_exif_offset;
  }

  jmp_buf & jpegjmp() {
    return m_jpegjmp;
  }
private:
  int _loadHeader();

  struct jpeg_decompress_struct m_cinfo;
  struct jpeg_error_mgr m_jerr;
  jmp_buf m_jpegjmp;
  bool m_headerLoaded;
  std::unique_ptr<IfdFileContainer> m_ifd;
  size_t m_exif_offset;
};

}
}
