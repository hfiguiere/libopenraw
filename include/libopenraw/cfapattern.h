/*
 * libopenraw - cfapattern.h
 *
 * Copyright (C) 2016 Hubert Figuière
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

typedef const struct _CfaPattern *ORCfaPatternRef;

void or_cfapattern_set_size(ORCfaPatternRef pattern, uint16_t x, uint16_t y);

or_cfa_pattern or_cfapattern_get_type(ORCfaPatternRef);

const uint8_t *or_cfapattern_get_pattern(ORCfaPatternRef pattern, uint16_t * count);


#ifdef __cplusplus
}
#endif

#endif



