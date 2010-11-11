/*
 * libopenraw - consts.h
 *
 * Copyright (c) 2008 Novell, Inc.
 * Copyright (C) 2005-2010 Hubert Figuiere
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
/**
 * @brief the libopenraw public consts types
 * @author Hubert Figuiere <hub@figuiere.net>
 */

#ifndef __LIBOPENRAW_CONSTS_H__
#define __LIBOPENRAW_CONSTS_H__

#include <stdint.h>

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
		OR_ERROR_INVALID_FORMAT = 7, /**< invalid format */
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
		OR_RAWFILE_TYPE_ERF, /**< Epson ERF */
		OR_RAWFILE_TYPE_TIFF /**< Generic TIFF */
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



	/** this is the type ID, a combination of vendor model 
	 *  It maps a specific camera. Only for the NATIVE file format.
	 */
	typedef uint32_t or_rawfile_typeid;

	/** make a %or_rawfile_typeid with a vendor and camera. */
    #define OR_MAKE_FILE_TYPEID(vendor,camera) ((vendor << 16) | (camera & 0xffff))
	/** get the vendor from the %or_rawfile_typeid */
    #define OR_GET_FILE_TYPEID_VENDOR(ftypeid) ((ftypeid & 0xffff0000) >> 16)
	/** get the camera from the %or_rawfile_typeid */
    #define OR_GET_FILE_TYPEID_CAMERA(ftypeid) (ftypeid & 0xffff)

	/** The vendor ID: the high order 16-bits of the %or_rawfile_typeid
	 */
	enum {
		OR_TYPEID_VENDOR_NONE = 0,
		OR_TYPEID_VENDOR_CANON = 1,
		OR_TYPEID_VENDOR_NIKON = 2,
		OR_TYPEID_VENDOR_LEICA = 3,
		OR_TYPEID_VENDOR_PENTAX = 4,
		OR_TYPEID_VENDOR_EPSON = 5,
		OR_TYPEID_VENDOR_MINOLTA = 6,
		OR_TYPEID_VENDOR_OLYMPUS = 7,
		OR_TYPEID_VENDOR_SONY = 8,
		OR_TYPEID_VENDOR_SAMSUNG = 9,
		OR_TYPEID_VENDOR_RICOH = 10,
		OR_TYPEID_VENDOR_PANASONIC = 11,
		OR_TYPEID_VENDOR_MAMIYA = 12,
		/* not really a camera vendor. For the converter. */
		OR_TYPEID_VENDOR_ADOBE = 13,
		_OR_TYPEID_VENDOR_LAST
	};

	enum {
		OR_TYPEID_UNKNOWN = 0
	};

	enum {
		OR_TYPEID_ADOBE_UNKNOWN = 0,
		OR_TYPEID_ADOBE_DNG_GENERIC = 1,
		_OR_TYPEID_ADOBE_LAST
	};

	/** Canon type IDs */
	enum {
		OR_TYPEID_CANON_UNKNOWN = 0,
		OR_TYPEID_CANON_20D,
		OR_TYPEID_CANON_30D,
		OR_TYPEID_CANON_40D,
		OR_TYPEID_CANON_350D,
		OR_TYPEID_CANON_400D,
		OR_TYPEID_CANON_450D,
		OR_TYPEID_CANON_5D,
		OR_TYPEID_CANON_1D,
		OR_TYPEID_CANON_1DMKII,
		OR_TYPEID_CANON_1DMKIII,
		OR_TYPEID_CANON_1DS,
		OR_TYPEID_CANON_1DSMKII,
		OR_TYPEID_CANON_1DSMKIII,
		OR_TYPEID_CANON_300D,
		OR_TYPEID_CANON_D30,
		OR_TYPEID_CANON_D60,
		OR_TYPEID_CANON_10D,
		OR_TYPEID_CANON_PRO1,
		OR_TYPEID_CANON_G1,
		OR_TYPEID_CANON_G2,
		OR_TYPEID_CANON_G3,
		OR_TYPEID_CANON_G5,
		OR_TYPEID_CANON_G6,
		OR_TYPEID_CANON_G7,
		OR_TYPEID_CANON_G9,
		OR_TYPEID_CANON_A610,
		OR_TYPEID_CANON_20DA,
		OR_TYPEID_CANON_7D,
		OR_TYPEID_CANON_G11,
		OR_TYPEID_CANON_1DMKIV,
		OR_TYPEID_CANON_500D,
		OR_TYPEID_CANON_5DMKII,
		OR_TYPEID_CANON_550D,
		OR_TYPEID_CANON_1000D,
		OR_TYPEID_CANON_G10,
		OR_TYPEID_CANON_50D,
		OR_TYPEID_CANON_60D,
		_OR_TYPEID_CANON_LAST
	};


	/** Nikon type IDs */
	enum {
		OR_TYPEID_NIKON_UNKNOWN = 0,
		OR_TYPEID_NIKON_COOLPIX_5700 ,
		OR_TYPEID_NIKON_D1,
		OR_TYPEID_NIKON_D1X,
		OR_TYPEID_NIKON_D100,
		OR_TYPEID_NIKON_D2H,
		OR_TYPEID_NIKON_D2X,
		OR_TYPEID_NIKON_D200,
		OR_TYPEID_NIKON_D3,
		OR_TYPEID_NIKON_D300,
		OR_TYPEID_NIKON_D40,
		OR_TYPEID_NIKON_D40X,
		OR_TYPEID_NIKON_D50,
		OR_TYPEID_NIKON_D70,
		OR_TYPEID_NIKON_D70S,
		OR_TYPEID_NIKON_D80,
		OR_TYPEID_NIKON_D3000,
		_OR_TYPEID_NIKON_LAST
	};

	/** Leica type IDs */
	enum {
		OR_TYPEID_LEICA_UNKNOWN = 0,
		OR_TYPEID_LEICA_DMR = 1,
		OR_TYPEID_LEICA_M8 = 2,
		OR_TYPEID_LEICA_X1 = 3,
		_OR_TYPEID_LEICA_LAST
	};

	/** Pentax type IDs */
	enum {
		OR_TYPEID_PENTAX_UNKNOWN = 0,
		OR_TYPEID_PENTAX_K10D_PEF,
		OR_TYPEID_PENTAX_K10D_DNG,
		OR_TYPEID_PENTAX_IST_D,
		OR_TYPEID_PENTAX_IST_DL,
		OR_TYPEID_PENTAX_K100D_PEF,
		OR_TYPEID_PENTAX_K100D_SUPER_PEF,
		OR_TYPEID_PENTAX_K20D_PEF
	};

	/** Epson type IDs */
	enum {
		OR_TYPEID_EPSON_UNKNOWN = 0,
		OR_TYPEID_EPSON_RD1
	};

	/** Minolta type IDs */
	enum {
		OR_TYPEID_MINOLTA_UNKNOWN = 0,
		OR_TYPEID_MINOLTA_A1,
		OR_TYPEID_MINOLTA_A2,
		OR_TYPEID_MINOLTA_DIMAGE5,
		OR_TYPEID_MINOLTA_DIMAGE7,
		OR_TYPEID_MINOLTA_DIMAGE7I,
		OR_TYPEID_MINOLTA_DIMAGE7HI,
		OR_TYPEID_MINOLTA_MAXXUM_5D,
		OR_TYPEID_MINOLTA_MAXXUM_7D,
		OR_TYPEID_MINOLTA_A200
	};

	enum {
		OR_TYPEID_OLYMPUS_UNKNOWN = 0,
		OR_TYPEID_OLYMPUS_E1,
		OR_TYPEID_OLYMPUS_E10,
		OR_TYPEID_OLYMPUS_E3,
		OR_TYPEID_OLYMPUS_E300,
		OR_TYPEID_OLYMPUS_E330,
		OR_TYPEID_OLYMPUS_E400,
		OR_TYPEID_OLYMPUS_E410,
		OR_TYPEID_OLYMPUS_E500,
		OR_TYPEID_OLYMPUS_E510,
		OR_TYPEID_OLYMPUS_SP350,
		OR_TYPEID_OLYMPUS_SP510,
		OR_TYPEID_OLYMPUS_SP550,
		OR_TYPEID_OLYMPUS_SP500,
		OR_TYPEID_OLYMPUS_EP1,
		OR_TYPEID_OLYMPUS_E620
	};

	enum {
		OR_TYPEID_SAMSUNG_UNKNOWN = 0,
		OR_TYPEID_SAMSUNG_GX10,
		OR_TYPEID_SAMSUNG_PRO815
	};


	enum {
		OR_TYPEID_RICOH_UNKNOWN = 0,
		OR_TYPEID_RICOH_GR2 = 1,
		OR_TYPEID_RICOH_GXR = 2,
		_OR_TYPEID_RICOH_LAST
	};

	enum {
		OR_TYPEID_SONY_UNKNOWN = 0,
		OR_TYPEID_SONY_A100,
		OR_TYPEID_SONY_A200,
		OR_TYPEID_SONY_A700,
		OR_TYPEID_SONY_A550,
		_OR_TYPEID_SONY_LAST
	};

#ifdef __cplusplus
}
#endif

#endif
