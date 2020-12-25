/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
/*
 * libopenraw - rawdata.h
 *
 * Copyright (C) 2007-2019 Hubert Figuiere
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


#ifndef LIBOPENRAW_RAWDATA_H_
#define LIBOPENRAW_RAWDATA_H_

#include <stddef.h>
#include <stdint.h>

#include <libopenraw/consts.h>
#include <libopenraw/types.h>
#include <libopenraw/mosaicinfo.h>

/** @file Functions to access manipulate %ORRawData */

/** @defgroup rawdata_api RawData API
 * @ingroup public_api
 *
 * @brief Access the raw data.
 * @{
 */
#ifdef __cplusplus
extern "C" {
#endif

	/** @brief Extract the RAW data from the raw file.
	 * @param filename the raw file name
	 * @param options the options to pass
	 * @param rawdata the destination RawData. Must allocated.
	 */
	or_error or_get_extract_rawdata(const char* filename, uint32_t options,
																	ORRawDataRef *rawdata);
	/** @brief Allocate a new RawData
	 * @return A newly allocated RawData. Must be released by %or_rawdata_release
	 */
	ORRawDataRef or_rawdata_new(void);

	/** @brief Release the rawdata */
	or_error or_rawdata_release(ORRawDataRef rawdata);

	/** @brief Get the format of the RAW data */
	or_data_type or_rawdata_format(ORRawDataRef rawdata);

	/** @brief Get a pointer to the RAW data
	 *
	 * The pointer is owned by the RawData object.
	 */
	void* or_rawdata_data(ORRawDataRef rawdata);

	/** @brief Get the size of the RAW data in bytes */
	size_t or_rawdata_data_size(ORRawDataRef rawdata);

	/** @brief Get the RAW data dimensions in pixels
	 * @param [out] x the horizontal dimension
	 * @param [out] y the vertical dimension
	 */
	void
	or_rawdata_dimensions(ORRawDataRef rawdata,
						  uint32_t *x, uint32_t *y);

	/** @brief Get the active area for the raw data.
	 *
	 * The active area is the usefull part of the RAW data
	 * it is specific per camera and isn't the crop.
	 *
	 * @param rawdata the RawData object
	 * @param [out] x the X origin
	 * @param [out] y the Y origin
	 * @param [out] width the width
	 * @param [out] height the height.
	 * @return an error code or %OR_ERROR_NONE in case of success.
	 */
	or_error
	or_rawdata_get_active_area(ORRawDataRef rawdata,
							   uint32_t *x, uint32_t *y,
							   uint32_t *width, uint32_t *height);

	/** @brief Return the bits per component
	 *
	 * @return the number of bits per component in the RAW data.
	 */
	uint32_t
	or_rawdata_bpc(ORRawDataRef rawdata);

	/** @brief Return the bayer type for the raw data.
	 *
	 * @return one of the constant defined in %or_cfa_pattern
	 */
	or_cfa_pattern
	or_rawdata_get_cfa_pattern_type(ORRawDataRef rawdata);

	/** @brief Return the mosaic info
	 *
	 * @return a MosaicInfo object. It is owned by the RawData. Can't be NULL.
	 */
	ORMosaicInfoRef
	or_rawdata_get_mosaicinfo(ORRawDataRef rawdata);

	/** @brief Return the compression type for the RawData.
	 *
	 * @return the numerical value.
	 */
	uint32_t
	or_rawdata_get_compression(ORRawDataRef rawdata);

	/** @brief Return the levels values for the raw data.
	 *
	 * These are possible values, not actual values.
	 *
	 * @param rawdata the raw data object
	 * @param [out] black the pointer to the black value.
	 * @param [out] white the pointer to the white value.
	 * @return the error code.
	 */
	or_error
	or_rawdata_get_levels(ORRawDataRef rawdata, uint16_t *black,
                             uint16_t *white);

	/** @brief Get the colour matrix.
	 * @param rawdata the raw data object
	 * @param index the matrix index.
	 * @param [out] size of %matrix. Returns the actual size.
	 * @return the matrix. Pointer is owned by the RawData.
	 */
	const double*
	or_rawdata_get_colour_matrix(ORRawDataRef rawdata, uint32_t index,
				     uint32_t *size);

	/** @brief Get the rendered image from the raw data
	 * @param rawdata the raw data.
	 * @param bitmapdata the preallocated bitmap data.
	 * @param options option for rendering. Pass 0 for now.
	 * @return an error code, %OR_ERROR_NONE in case of success.
	 */
	or_error
	or_rawdata_get_rendered_image(ORRawDataRef rawdata,
				      ORBitmapDataRef bitmapdata,
				      uint32_t options);

#ifdef __cplusplus
}
#endif
/** @} */
#endif
