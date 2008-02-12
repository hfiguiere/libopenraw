/*
 * libopenraw - rawdata.h
 *
 * Copyright (C) 2007 Hubert Figuiere
 * Copyright (C) 2008 Novell, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */


#ifndef __LIBOPENRAW_RAWDATA_H_
#define __LIBOPENRAW_RAWDATA_H_

#include <libopenraw/types.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct _RawData *ORRawDataRef;

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

	uint32_t
	or_rawdata_bpc(ORRawDataRef rawdata);

	/** Return the bayer type for the raw data.
	 * @return one of the constant defined in %or_cfa_pattern
	 */
	or_cfa_pattern
	or_rawdata_get_cfa_pattern(ORRawDataRef rawdata);

#ifdef __cplusplus
}
#endif

#endif
