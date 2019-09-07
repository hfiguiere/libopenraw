/*
 * libopenraw - mosaicinfo.h
 *
 * Copyright (C) 2016-2019 Hubert Figuière
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


#ifndef LIBOPENRAW_CFAPATTERN_H_
#define LIBOPENRAW_CFAPATTERN_H_

#include <stdint.h>

#include <libopenraw/consts.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef const struct _MosaicInfo *ORMosaicInfoRef;

void or_mosaicinfo_set_size(ORMosaicInfoRef pattern, uint16_t x, uint16_t y);
void or_mosaicinfo_get_size(ORMosaicInfoRef pattern, uint16_t *x, uint16_t *y);

or_cfa_pattern or_mosaicinfo_get_type(ORMosaicInfoRef);

const uint8_t *or_mosaicinfo_get_pattern(ORMosaicInfoRef pattern, uint16_t * count);


#ifdef __cplusplus
}
#endif

#endif



