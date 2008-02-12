/*
 * libopenraw - consts.h
 *
 * Copyright (C) 2005-2008 Hubert Figuiere
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
/**
 * @brief the libopenraw public consts types
 * @author Hubert Figuiere <hub@figuiere.net>
 */

#ifndef __LIBOPENRAW_CONSTS_H__
#define __LIBOPENRAW_CONSTS_H__


#ifdef __cplusplus
extern "C" {
#endif

	
/** 
 * Error codes returned by libopenraw. 
 */
	typedef enum {
		OR_ERROR_NONE = 0,         /**< no error */
		OR_ERROR_BUF_TOO_SMALL = 1,
		OR_ERROR_NOTAREF = 2,      /**< the object is not ref */
		OR_ERROR_CANT_OPEN = 3,    /**< can't open file. Check OS error codes */
		OR_ERROR_CLOSED_STREAM = 4,/**< stream closed */
		OR_ERROR_NOT_FOUND = 5,    /**< requested "object" not found */
		OR_ERROR_INVALID_PARAM = 6,
		OR_ERROR_UNKNOWN = 42,
		OR_ERROR_LAST_ 
	} or_error;

	
	/** different types of RAW files 
	 */
	typedef enum {
		OR_RAWFILE_TYPE_UNKNOWN = 0, /**< no type. Invalid value. */
		OR_RAWFILE_TYPE_CR2, /**< Canon CR2 */
		OR_RAWFILE_TYPE_CRW, /**< Canon CRW */
		OR_RAWFILE_TYPE_NEF, /**< Nikon NEF */
		OR_RAWFILE_TYPE_MRW, /**< Minolta MRW */
		OR_RAWFILE_TYPE_ARW, /**< Sony ARW */
		OR_RAWFILE_TYPE_DNG, /**< Adobe DNG */
		OR_RAWFILE_TYPE_ORF, /**< Olympus ORF */
		OR_RAWFILE_TYPE_PEF, /**< Pentax PEF */
		OR_RAWFILE_TYPE_ERF  /**< Epson ERF */
	} or_rawfile_type;

	typedef enum {
		OR_DATA_TYPE_NONE = 0,
		OR_DATA_TYPE_PIXMAP_8RGB, /**< 8bit per channel RGB pixmap */
		OR_DATA_TYPE_JPEG,        /**< JPEG data */
		OR_DATA_TYPE_TIFF,        /**< TIFF container */ 
		OR_DATA_TYPE_PNG,         /**< PNG container */
		OR_DATA_TYPE_CFA,         /**< bayer CFA container */
		OR_DATA_TYPE_COMPRESSED_CFA, /**< compressed bayer CFA container */

		OR_DATA_TYPE_UNKNOWN
	} or_data_type;

	typedef enum {
		OR_CFA_PATTERN_NONE = 0,   /**< Invalid value */
		OR_CFA_PATTERN_NON_RGB22 = 1,
		OR_CFA_PATTERN_RGGB = 2,
		OR_CFA_PATTERN_GBRG = 3,
		OR_CFA_PATTERN_BGGR = 4,
		OR_CFA_PATTERN_GRBG = 5
	} or_cfa_pattern;

	typedef enum {
		OR_OPTIONS_NONE            = 0x00000000,
		OR_OPTIONS_DONT_DECOMPRESS = 0x00000001   /**< don't decompress */

	} or_options;

#ifdef __cplusplus
}
#endif

#endif
