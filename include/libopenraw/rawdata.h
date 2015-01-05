/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
/*
 * libopenraw - rawdata.h
 *
 * Copyright (C) 2007-2016 Hubert Figuiere
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
#include <libopenraw/cfapattern.h>

#ifdef __cplusplus
extern "C" {
#endif

	/** Extract the RAW data from the raw file.
	 * @param filename the raw file name
	 * @param options the options to pass
	 * @param 
	 */
	or_error or_get_extract_rawdata(const char* filename, uint32_t options,
																	ORRawDataRef *rawdata);
	
	ORRawDataRef
	or_rawdata_new(void);

	or_error
	or_rawdata_release(ORRawDataRef rawdata);

	or_data_type 
	or_rawdata_format(ORRawDataRef rawdata);

	void *
	or_rawdata_data(ORRawDataRef rawdata);

	size_t
	or_rawdata_data_size(ORRawDataRef rawdata);

	void
	or_rawdata_dimensions(ORRawDataRef rawdata, 
						  uint32_t *x, uint32_t *y);
	
	void
	or_rawdata_get_roi(ORRawDataRef rawdata, 
				   uint32_t *x, uint32_t *y,
				   uint32_t *width, uint32_t *height);	

	uint32_t
	or_rawdata_bpc(ORRawDataRef rawdata);

	/** Return the bayer type for the raw data.
	 * @return one of the constant defined in %or_cfa_pattern
	 */
	or_cfa_pattern
	or_rawdata_get_cfa_pattern_type(ORRawDataRef rawdata);

	ORCfaPatternRef
	or_rawdata_get_cfa_pattern(ORRawDataRef rawdata);

	/** Return the compression type for the RawData.
	 * @return the numerical value.
	 */
	uint32_t
	or_rawdata_get_compression(ORRawDataRef rawdata);

	/** Return the levels values for the raw data.
	 * This are possible values, not actual values.
	 * @param rawdata the raw data object
	 * @param black the pointer to the black value.
	 * @param white the pointer to the white value.
	 * @return the error code.
	 */
	or_error
	or_rawdata_get_levels(ORRawDataRef rawdata, uint16_t *black,
                             uint16_t *white);

	/** Get the colour matrix.
	 * @param rawdata the raw data object
	 * @param index the matrix index.
	 * @param size of %matrix. Returns the actual size.
	 * @return the matrix. Pointer is valid as long as %rawdata is.
	 */
	const double*
	or_rawdata_get_colour_matrix(ORRawDataRef rawdata, uint32_t index,
				     uint32_t *size);

	/** Get the rendered image from the raw data
	 * @param rawdata the raw data.
	 * @param bitmapdata the preallocated bitmap data.
	 * @param options option for rendering. Pass 0 for now.
	 */
	or_error
	or_rawdata_get_rendered_image(ORRawDataRef rawdata,
				      ORBitmapDataRef bitmapdata,
				      uint32_t options);

#ifdef __cplusplus
}
#endif

#endif
