/*
 * libopenraw - mosaicinfo.h
 *
 * Copyright (C) 2016-2020 Hubert Figuière
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

/** @defgroup mosaicinfo_api Mosaic Info API
 * @ingroup public_api
 *
 * @brief Access to the mosaic info
 * @{
 */
#ifdef __cplusplus
extern "C" {
#endif

/** @brief A MosaicInfo object */
typedef const struct _MosaicInfo *ORMosaicInfoRef;

/** @brief Set the size of the mosaic */
void or_mosaicinfo_set_size(ORMosaicInfoRef pattern, uint16_t x, uint16_t y);
/** @brief Get the size of the mosaic */
void or_mosaicinfo_get_size(ORMosaicInfoRef pattern, uint16_t *x, uint16_t *y);

/** @brief Get the type of the mosaic */
or_cfa_pattern or_mosaicinfo_get_type(ORMosaicInfoRef);

/** @brief Get the pattern.
 *
 * This will return an array of %or_cfa_pattern_colour indicating the individual colours
 * of the mosaic colour filter array.
 *
 * @param pattern The %MosaicInfo
 * @param [out] count the size of the array returned.
 * @return The pattern. The pointer is owned by the %MosaicInfo object.
 */
const uint8_t* or_mosaicinfo_get_pattern(ORMosaicInfoRef pattern, uint16_t* count);


#ifdef __cplusplus
}
#endif

/** @} */
#endif



