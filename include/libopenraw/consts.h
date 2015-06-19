/*
 * libopenraw - consts.h
 *
 * Copyright (c) 2008 Novell, Inc.
 * Copyright (C) 2005-2015 Hubert Figuiere
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

#ifndef LIBOPENRAW_CONSTS_H_
#define LIBOPENRAW_CONSTS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Error codes returned by libopenraw.
 */
typedef enum {
    OR_ERROR_NONE = 0, /**< no error */
    OR_ERROR_BUF_TOO_SMALL = 1,
    OR_ERROR_NOTAREF = 2,       /**< the object is not ref */
    OR_ERROR_CANT_OPEN = 3,     /**< can't open file. Check OS error codes */
    OR_ERROR_CLOSED_STREAM = 4, /**< stream closed */
    OR_ERROR_NOT_FOUND = 5,     /**< requested "object" not found */
    OR_ERROR_INVALID_PARAM = 6,
    OR_ERROR_INVALID_FORMAT = 7, /**< invalid format */
    OR_ERROR_DECOMPRESSION = 8,  /**< decompression error */
    OR_ERROR_UNKNOWN = 42,
    OR_ERROR_LAST_
} or_error;

/** different types of RAW files
 */
typedef enum {
    OR_RAWFILE_TYPE_UNKNOWN = 0, /**< no type. Invalid value. */
    OR_RAWFILE_TYPE_CR2,         /**< Canon CR2 */
    OR_RAWFILE_TYPE_CRW,         /**< Canon CRW */
    OR_RAWFILE_TYPE_NEF,         /**< Nikon NEF */
    OR_RAWFILE_TYPE_MRW,         /**< Minolta MRW */
    OR_RAWFILE_TYPE_ARW,         /**< Sony ARW */
    OR_RAWFILE_TYPE_DNG,         /**< Adobe DNG */
    OR_RAWFILE_TYPE_ORF,         /**< Olympus ORF */
    OR_RAWFILE_TYPE_PEF,         /**< Pentax PEF */
    OR_RAWFILE_TYPE_ERF,         /**< Epson ERF */
    OR_RAWFILE_TYPE_TIFF,        /**< Generic TIFF */
    OR_RAWFILE_TYPE_NRW,         /**< Nikon NRW */
    OR_RAWFILE_TYPE_RW2,         /**< Panasonic RAW and RW2 */
    OR_RAWFILE_TYPE_RAF,         /**< FujiFilm RAF */
    _OR_RAWFILE_TYPE_LAST
} or_rawfile_type;

typedef enum {
    OR_DATA_TYPE_NONE = 0,
    OR_DATA_TYPE_PIXMAP_8RGB,    /**< 8bit per channel RGB pixmap */
    OR_DATA_TYPE_PIXMAP_16RGB,   /**< 16bit per channel RGB pixmap */
    OR_DATA_TYPE_JPEG,           /**< JPEG data */
    OR_DATA_TYPE_TIFF,           /**< TIFF container */
    OR_DATA_TYPE_PNG,            /**< PNG container */
    OR_DATA_TYPE_RAW,            /**< RAW container */
    OR_DATA_TYPE_COMPRESSED_RAW, /**< compressed RAW container */

    OR_DATA_TYPE_UNKNOWN
} or_data_type;

typedef enum {
    OR_CFA_PATTERN_NONE = 0, /**< Invalid value */
    OR_CFA_PATTERN_NON_RGB22 = 1,
    OR_CFA_PATTERN_RGGB = 2,
    OR_CFA_PATTERN_GBRG = 3,
    OR_CFA_PATTERN_BGGR = 4,
    OR_CFA_PATTERN_GRBG = 5,
    _OR_CFA_PATTERN_INVALID
} or_cfa_pattern;

typedef enum {
    OR_PATTERN_COLOUR_RED = 0,
    OR_PATTERN_COLOUR_GREEN = 1,
    OR_PATTERN_COLOUR_BLUE = 2
} or_cfa_pattern_colour;

typedef enum {
    OR_OPTIONS_NONE = 0x00000000,
    OR_OPTIONS_DONT_DECOMPRESS = 0x00000001 /**< don't decompress */

} or_options;

/** this is the type ID, a combination of vendor model
 *  It maps a specific camera. Only for the NATIVE file format.
 */
typedef uint32_t or_rawfile_typeid;

/** make a %or_rawfile_typeid with a vendor and camera. */
#define OR_MAKE_FILE_TYPEID(vendor, camera) ((vendor << 16) | (camera & 0xffff))
/** get the vendor from the %or_rawfile_typeid */
#define OR_GET_FILE_TYPEID_VENDOR(ftypeid) ((ftypeid & 0xffff0000) >> 16)
/** get the camera from the %or_rawfile_typeid */
#define OR_GET_FILE_TYPEID_CAMERA(ftypeid) (ftypeid & 0xffff)

#ifdef __cplusplus
}
#endif

#endif
