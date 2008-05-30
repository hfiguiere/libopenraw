/*
 * libopenraw - rawdata.cpp
 *
 * Copyright (C) 2007 Hubert Figuiere
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
/* @brief C api for rawdata
 */


#include <libopenraw/libopenraw.h>

#include <libopenraw++/rawdata.h>

using OpenRaw::RawData;

extern "C" {

	or_error or_get_extract_rawdata(const char* filename, uint32_t options,
																	ORRawDataRef *rawdata)
	{
		or_error ret = OR_ERROR_NONE;

		RawData ** pRawData = reinterpret_cast<RawData **>(rawdata);
		*pRawData = RawData::getAndExtractRawData(filename,
																							options, ret);
		return ret;
	}

	ORRawDataRef
	or_rawdata_new(void)
	{
		RawData * rawdata = new RawData();
		return reinterpret_cast<ORRawDataRef>(rawdata);
	}

	or_error
	or_rawdata_release(ORRawDataRef rawdata)
	{
		if (rawdata == NULL) {
			return OR_ERROR_NOTAREF;
		}
		delete reinterpret_cast<RawData *>(rawdata);
		return OR_ERROR_NONE;
	}


	or_data_type 
	or_rawdata_format(ORRawDataRef rawdata)
	{
		return reinterpret_cast<RawData *>(rawdata)->dataType();
	}


	void *
	or_rawdata_data(ORRawDataRef rawdata)
	{
		return reinterpret_cast<RawData *>(rawdata)->data();		
	}


	size_t
	or_rawdata_data_size(ORRawDataRef rawdata)
	{
		return reinterpret_cast<RawData *>(rawdata)->size();		
	}


	void
	or_rawdata_dimensions(ORRawDataRef rawdata, 
						  uint32_t *x, uint32_t *y)
	{
		RawData* t = reinterpret_cast<RawData *>(rawdata);
		if (x != NULL) {
			*x = t->x();
		}
		if (y != NULL) {
			*y = t->y();
		}
	}

	uint32_t
	or_rawdata_bpc(ORRawDataRef rawdata)
	{
		return reinterpret_cast<RawData *>(rawdata)->bpc();		
	}

	or_cfa_pattern
	or_rawdata_get_cfa_pattern(ORRawDataRef rawdata)
	{
		return reinterpret_cast<RawData *>(rawdata)->cfaPattern();
	}

	or_error
	or_rawdata_get_minmax(ORRawDataRef rawdata, uint16_t *min, uint16_t *max)
	{
		RawData* t = reinterpret_cast<RawData *>(rawdata);
		if(min) {
			*min = t->min();
		}
		if(max) {
			*max = t->max();
		}
		return OR_ERROR_NONE;
	}

}
