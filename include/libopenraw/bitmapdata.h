/*
 * libopenraw - bitmapdata.h
 *
 * Copyright (C) 2008 Novell, Inc.
 * Copyright (C) 2012 Hubert Figuière
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


#ifndef LIBOPENRAW_BITMAPDATA_H_
#define LIBOPENRAW_BITMAPDATA_H_

#include <stdlib.h>

#include <libopenraw/types.h>

#ifdef __cplusplus
extern "C" {
#endif

	
ORBitmapDataRef
or_bitmapdata_new(void);

or_error
or_bitmapdata_release(ORBitmapDataRef bitmapdata);

or_data_type 
or_bitmapdata_format(ORBitmapDataRef bitmapdata);

void *
or_bitmapdata_data(ORBitmapDataRef bitmapdata);

size_t
or_bitmapdata_data_size(ORBitmapDataRef bitmapdata);

void
or_bitmapdata_dimensions(ORBitmapDataRef bitmapdata, 
			  uint32_t *x, uint32_t *y);

uint32_t
or_bitmapdata_bpc(ORBitmapDataRef bitmapdata);

#ifdef __cplusplus
}
#endif

#endif
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
