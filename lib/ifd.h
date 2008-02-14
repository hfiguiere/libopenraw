/*
 * libopenraw - ifd.h
 *
 * Copyright (C) 2006-2007 Hubert Figuiere
 *
 * Defintions taken from libexif:
 * Copyright (C) 2001 Lutz Müller <lutz@users.sourceforge.net>
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
 * @brief Define IFD values like fields ID and types
 */

#ifndef __OPENRAW_IFD_H__
#define __OPENRAW_IFD_H__

namespace OpenRaw {
	namespace Internals {
		namespace IFD {

#define _INCLUDE_EXIF
#include "libopenraw/exif.h"
#undef _INCLUDE_EXIF

			/** type for Exif field/tag
					taken from libexif
			 */
			typedef enum {
				EXIF_FORMAT_BYTE       =  1,
				EXIF_FORMAT_ASCII      =  2,
				EXIF_FORMAT_SHORT      =  3,
				EXIF_FORMAT_LONG       =  4,
				EXIF_FORMAT_RATIONAL   =  5,
				EXIF_FORMAT_SBYTE      =  6,
				EXIF_FORMAT_UNDEFINED  =  7,
				EXIF_FORMAT_SSHORT     =  8,
				EXIF_FORMAT_SLONG      =  9,
				EXIF_FORMAT_SRATIONAL  = 10,
				EXIF_FORMAT_FLOAT      = 11,
				EXIF_FORMAT_DOUBLE     = 12
			} ExifTagType;


			typedef enum {
				CFA_RED = 0,
				CFA_GREEN = 1,
				CFA_BLUE = 2,
				CFA_CYAN = 3,
				CFA_MAGENTA = 4,
				CFA_YELLOW = 5,
				CFA_WHITE = 6
			} CfaComponent;

			typedef enum {
				COMPRESS_NONE = 1,
				COMPRESS_JPEG = 6,
				COMPRESS_NIKON_PACK = 32769,
				COMPRESS_NIKON_QUANTIZED = 34713
			} TiffCompress;
		}
	}
}

#endif
