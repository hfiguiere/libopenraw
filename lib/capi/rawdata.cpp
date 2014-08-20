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


#include <libopenraw/rawdata.h>

#include <libopenraw++/rawdata.h>
#include <libopenraw++/bitmapdata.h>

using OpenRaw::RawData;
using OpenRaw::BitmapData;

extern "C" {

/** check pointer validity */
#define CHECK_PTR(p,r) \
	if(p == NULL) { return r; }
	
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
						  uint32_t *width, uint32_t *height)
	{
		RawData* t = reinterpret_cast<RawData *>(rawdata);
		if (width != NULL) {
			*width = t->width();
		}
		if (height != NULL) {
			*height = t->height();
		}
	}
	
	void
	or_rawdata_get_roi(ORRawDataRef rawdata, 
				   uint32_t *x, uint32_t *y,
				   uint32_t *width, uint32_t *height)
	{
		RawData* t = reinterpret_cast<RawData *>(rawdata);
		if (x != NULL) {
			*x = t->roi_x();
		}
		if (y != NULL) {
			*y = t->roi_y();
		}
		if (width != NULL) {
			*width = t->roi_width();
		}
		if (height != NULL) {
			*height = t->roi_height();
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
		return reinterpret_cast<RawData *>(rawdata)
			->cfaPattern()->patternType();
	}

	or_error
	or_rawdata_get_levels(ORRawDataRef rawdata, uint16_t *black,
                              uint16_t *white)
	{
		RawData* t = reinterpret_cast<RawData *>(rawdata);
		if(black) {
			*black = t->blackLevel();
		}
		if(white) {
			*white = t->whiteLevel();
		}
		return OR_ERROR_NONE;
	}

or_error
or_rawdata_get_rendered_image(ORRawDataRef rawdata, ORBitmapDataRef bitmapdata,
							  uint32_t options)
{
	RawData * prawdata = reinterpret_cast<RawData *>(rawdata);
	CHECK_PTR(rawdata, OR_ERROR_NOTAREF);
	return prawdata->getRenderedImage(*reinterpret_cast<BitmapData*>(bitmapdata), options);
}
	
}
