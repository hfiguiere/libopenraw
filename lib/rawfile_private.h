/*
 * libopenraw - rawfile_private.h
 *
 * Copyright (C) 2012 Hubert Figuiere
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

#ifndef _RAWFILE_PRIV_H__
#define _RAWFILE_PRIV_H__

#include <assert.h>

#include <map>

namespace OpenRaw {
namespace Internals {

/** describe the location of a thumbnail in an RAW file */
class ThumbDesc
{
public:
  ThumbDesc(uint32_t _x, uint32_t _y, ::or_data_type _type,
            size_t _offset, size_t _length)
    : x(_x), y(_y), type(_type)
    , offset(_offset), length(_length)
		{
#ifdef DEBUG
      assert(_length);
#endif
		}
  ThumbDesc()
    : x(0), y(0), type(OR_DATA_TYPE_NONE)
    , offset(0), length(0)
    {
    }
  uint32_t x;    /**< x size. Can be 0 */
  uint32_t y;    /**< y size. Can be 0 */
  ::or_data_type type; /**< the data type format */
  size_t   offset; /**< offset if the thumbnail data */
  size_t   length;
};

typedef std::map<uint32_t, ThumbDesc> ThumbLocations;

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
#endif