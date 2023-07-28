/*
 * libopenraw - consts.h
 *
 * Copyright (c) 2008 Novell, Inc.
 * Copyright (C) 2005-2020 Hubert Figuière
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
 * @file The libopenraw public consts types
 * @author Hubert Figuiere <hub@figuiere.net>
 */

#ifndef LIBOPENRAW_CONSTS_H_
#define LIBOPENRAW_CONSTS_H_

#include <stdint.h>

/** @addtogroup public_api
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Error codes returned by libopenraw.
 */
typedef enum {
    OR_ERROR_NONE = 0, /**< no error */
    OR_ERROR_BUF_TOO_SMALL = 1, /**< Buffer is too small. */
    OR_ERROR_NOTAREF = 2,       /**< The object is not ref */
    OR_ERROR_CANT_OPEN = 3,     /**< Can't open file. Check OS error codes */
    OR_ERROR_CLOSED_STREAM = 4, /**< Stream closed */
    OR_ERROR_NOT_FOUND = 5,     /**< Requested "object" not found */
    OR_ERROR_INVALID_PARAM = 6, /**< Invalid parameter */
    OR_ERROR_INVALID_FORMAT = 7, /**< Invalid format */
    OR_ERROR_DECOMPRESSION = 8,  /**< Decompression error */
    OR_ERROR_NOT_IMPLEMENTED = 9, /**< Function is not implemented */
    OR_ERROR_ALREADY_OPEN = 10, /**< Stream already open */
    OR_ERROR_UNKNOWN = 42, /**< Unknown error. */
    OR_ERROR_LAST_
} or_error;

/** @brief Types of RAW files */
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
    OR_RAWFILE_TYPE_RW2,         /**< Panasonic RAW, RW2 and RWL */
    OR_RAWFILE_TYPE_RAF,         /**< FujiFilm RAF */
    OR_RAWFILE_TYPE_CR3,         /**< Canon CR3 */
    OR_RAWFILE_TYPE_GPR,         /**< GoPro GPR (DNG-variation) */
    OR_RAWFILE_TYPE_SR2,         /**< Sony SR2 */
} or_rawfile_type;

/** @brief Data types */
typedef enum {
    OR_DATA_TYPE_NONE = 0,
    OR_DATA_TYPE_PIXMAP_8RGB = 1,    /**< 8bit per channel RGB pixmap */
    OR_DATA_TYPE_PIXMAP_16RGB = 2,   /**< 16bit per channel RGB pixmap */
    OR_DATA_TYPE_JPEG = 3,           /**< JPEG data */
    OR_DATA_TYPE_TIFF = 4,           /**< TIFF container */
    OR_DATA_TYPE_PNG = 5,            /**< PNG container */
    OR_DATA_TYPE_RAW = 6,            /**< RAW container */
    OR_DATA_TYPE_COMPRESSED_RAW = 7, /**< compressed RAW container */

    OR_DATA_TYPE_UNKNOWN = 100,
} or_data_type;

/** @brief CFA pattern types */
typedef enum {
    OR_CFA_PATTERN_NONE = 0, /**< Invalid value */
    OR_CFA_PATTERN_NON_RGB22 = 1, /**< Non RGB 2x2 CFA */
    OR_CFA_PATTERN_RGGB = 2,
    OR_CFA_PATTERN_GBRG = 3,
    OR_CFA_PATTERN_BGGR = 4,
    OR_CFA_PATTERN_GRBG = 5,
    _OR_CFA_PATTERN_INVALID
} or_cfa_pattern;

/** @brief CFA colour components */
typedef enum {
    OR_PATTERN_COLOUR_RED = 0, /**< Red */
    OR_PATTERN_COLOUR_GREEN = 1, /**< Green */
    OR_PATTERN_COLOUR_BLUE = 2 /** Blue */
} or_cfa_pattern_colour;

/** @brief Options */
typedef enum {
    OR_OPTIONS_NONE = 0x00000000, /**< No options */
    OR_OPTIONS_DONT_DECOMPRESS = 0x00000001 /**< Don't decompress */

} or_options;

/** @brief Where the colour matrix comes from.
 * Typically DNG is provided. The others are built-in.
 */
typedef enum {
    OR_COLOUR_MATRIX_UNKNOWN = 0, /**< Unknown. This usually signify an error */
    OR_COLOUR_MATRIX_BUILTIN = 1, /**< Colour matrix in library */
    OR_COLOUR_MATRIX_PROVIDED = 2, /**< Colour matrix provided by file */
} or_colour_matrix_origin;

/** @brief This is the type ID, a combination of vendor model
 *  It maps a specific camera. Only for the NATIVE file format.
 */
typedef uint32_t or_rawfile_typeid;

/** @brief Make a %or_rawfile_typeid with a vendor and camera. */
#define OR_MAKE_FILE_TYPEID(vendor, camera) ((vendor << 16) | (camera & 0xffff))
/** @brief Get the vendor from the %or_rawfile_typeid */
#define OR_GET_FILE_TYPEID_VENDOR(ftypeid) ((ftypeid & 0xffff0000) >> 16)
/** @brief Get the camera from the %or_rawfile_typeid */
#define OR_GET_FILE_TYPEID_CAMERA(ftypeid) (ftypeid & 0xffff)

/** @brief Type of IfdDir */
typedef enum {
    /// Generic
    OR_IFD_OTHER = 0,
    /// Main (like in TIFF)
    OR_IFD_MAIN = 1,
    /// Exif metadata
    OR_IFD_EXIF = 2,
    /// MakerNote
    OR_IFD_MNOTE = 3,
    /// RAW data
    OR_IFD_RAW = 4,
    /// SubIFD
    OR_IFD_SUBIFD = 5,
    /// INVALID value
    OR_IFD_INVALID = 10000,
} or_ifd_dir_type;


#ifdef __cplusplus
}
#endif

/** @} */

#endif
