/* -*- Mode: C++ -*- */
/*
 * libopenraw - olympusdecompressor.h
 *
 * Copyright (C) 2011-2016 Hubert Figuiere
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


#ifndef OR_INTERNALS_OLYMPUSDECOMPRESSOR_H_
#define OR_INTERNALS_OLYMPUSDECOMPRESSOR_H_

#include <stddef.h>
#include <stdint.h>

#include "decompressor.hpp"

namespace OpenRaw {

class RawData;

namespace Internals {

class RawContainer;

class OlympusDecompressor
  : public Decompressor
{
public:
OlympusDecompressor(const uint8_t *buffer, size_t size,
                    RawContainer * container, uint32_t w, uint32_t h)
  : Decompressor(NULL, container)
    , m_buffer(buffer)
    , m_size(size)
    , m_h(h)
    , m_w(w)
  {
  }
  virtual RawDataPtr decompress() override;
private:
  const uint8_t *m_buffer;
  size_t m_size;

  uint32_t m_h;
  uint32_t m_w;
};

}
}
#endif
