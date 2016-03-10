/*
 * libopenraw - render/grayscale.h
 *
 * Copyright (C) 2012-2016 Hubert Figuiere
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

#include "render/grayscale.hpp"

or_error
grayscale_to_rgb (uint16_t *src, uint32_t src_x, uint32_t src_y, 
                  uint16_t *dst)
{
  uint32_t x,y;
  uint32_t offset, doffset;
  
  offset = 0;
  doffset = 0;
  for(y = 0 ; y < src_y; y++)
  {
    for (x = 0 ; x < src_x; x++)
    {
      // change this. we currently clip.
      dst [doffset*3+0] = src[offset];
      dst [doffset*3+1] = src[offset];
      dst [doffset*3+2] = src[offset];
      
      offset++;
	    doffset++;
    }
  }

  return OR_ERROR_NONE;
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
