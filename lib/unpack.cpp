/*
 * libopenraw - unpack.cpp
 *
 * Copyright (C) 2008-2016 Hubert Figuiere
 * Copyright (C) 2008 Novell, Inc.
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

#include <assert.h>

#include <libopenraw/consts.h>

#include "unpack.hpp"
#include "trace.hpp"
#include "ifd.hpp"

namespace OpenRaw {
namespace Internals {

using namespace Debug;

Unpack::Unpack(uint32_t w, uint32_t t)
  : m_w(w), m_type(t)
{
}

/* Return the size of an image row. */
size_t Unpack::block_size()
{
  size_t bs;
  if(m_type == IFD::COMPRESS_NIKON_PACK) {
    bs = (m_w / 2 * 3) + (m_w / 10);
  }
  else {
    bs = m_w / 2 * 3;
  }
  return bs;
}


/** source is in BE byte order
 * the output is always 16-bits values in native (host) byte order.
 * the source must correspond to an image row.
 */
or_error Unpack::unpack_be12to16(uint8_t *dest, size_t destsize, const uint8_t *src,
                                 size_t size, size_t & out)
{
  or_error err = OR_ERROR_NONE;
  uint16_t *dest16 = reinterpret_cast<uint16_t *>(dest);
  size_t pad = (m_type == IFD::COMPRESS_NIKON_PACK) ? 1 : 0;
  size_t n = size / (15 + pad);
  size_t rest = size % (15 + pad);
  size_t ret = n * 20 + rest / 3 * 4;

  out = 0;

  /* The inner loop advances 10 columns, which corresponds to 15 input
     bytes, 20 output bytes and, in a Nikon pack, one padding byte.*/
  if (pad) {
    if ((size % 16) != 0) {
      LOGERR("be12to16 incorrect padding.\n");
      return OR_ERROR_DECOMPRESSION;
    }
  }
  if ((rest % 3) != 0) {
    LOGERR("be12to16 incorrect rest.\n");
    return OR_ERROR_DECOMPRESSION;
  }

  for (size_t i = 0; i < n + 1; i++) {
    size_t m = (i == n) ? rest / 3 : 5;
    if((reinterpret_cast<uint8_t *>(dest16) - dest) + (m * 4) >  destsize) {
      err = OR_ERROR_DECOMPRESSION;
      LOGERR("overflow !\n");
      break;
    }
    for(size_t j = 0; j < m; j++) {
      /* Read 3 bytes */
      uint32_t t = *src++;
      t <<= 8;
      t |= *src++;
      t <<= 8;
      t |= *src++;

      /* Write two 16 bit values. */
      *dest16 = (t & (0xfff << 12)) >> 12;
      dest16++;

      *dest16 = t & 0xfff;
      dest16++;
    }

    src += pad;
  }

  out = ret;
  return err;
}

} }
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
