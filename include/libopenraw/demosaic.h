/*
 * libopenraw - demosaic.h
 *
 * Copyright (C) 2008 Hubert Figuiere
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

#ifndef LIBOPENRAW_DEMOSAIC_H_
#define LIBOPENRAW_DEMOSAIC_H_

#include <stdint.h>
#include <libopenraw/consts.h>

#ifdef __cplusplus
extern "C" {
#endif

#if 0
/** low level demosaic. Use only if you know what you are doing */
void
or_demosaic (uint16_t *src, uint32_t src_x, uint32_t src_y, 
	     or_cfa_pattern pattern, uint8_t *dst);
#endif

#ifdef __cplusplus
}
#endif


#endif
