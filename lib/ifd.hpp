/*
 * libopenraw - ifd.hpp
 *
 * Copyright (C) 2006-2007, 2012-2023 Hubert Figuière
 *
 * Defintions taken from libexif:
 * Copyright (C) 2001 Lutz Müller <lutz@users.sourceforge.net>
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

#pragma once

#include <stdint.h>
#include <math.h>

namespace OpenRaw {
namespace Internals {

/** @addtogroup ifd_parsing
 * @{
 */

/** @brief IFD namespace */
namespace IFD {

#define INCLUDE_EXIF_
#include "libopenraw/exif.h"
#undef INCLUDE_EXIF_

/** @brief Component colour from CFA. Duplicate of or_cfa_pattern_colour */
typedef enum {
    CFA_RED = 0,
    CFA_GREEN = 1,
    CFA_BLUE = 2,
    CFA_CYAN = 3,
    CFA_MAGENTA = 4,
    CFA_YELLOW = 5,
    CFA_WHITE = 6
} CfaComponent;

/** @brief TIFF compression */
typedef enum {
    COMPRESS_NONE = 1, /**< No compression */
    COMPRESS_JPEG = 6, /**< JPEG compression */
    COMPRESS_LJPEG = 7, /**< Lossless JPEG, see DNG */
    COMPRESS_ARW = 32767, /**< Sony ARW compression */
    COMPRESS_NIKON_PACK = 32769, /**< Nikon packed */
    COMPRESS_PENTAX_PACK = 32773, /**< Pentax packed */
    COMPRESS_NIKON_QUANTIZED = 34713, /**< Nikon quantized */
    COMPRESS_CUSTOM = 65535, /**< The value everybody seems to use */
    COMPRESS_OLYMPUS = 65536 /**< Olympus compression */
} TiffCompress;

/** @brief Convert an ORRational to a double */
inline
double to_double(const ORRational& r)
{
    if (r.denom == 0) {
        return INFINITY;
    }
    return (double)r.num / (double)r.denom;
}

/** @brief Convert an ORSRational to a double */
inline
double to_double(const ORSRational& r)
{
    if (r.denom == 0) {
        return INFINITY;
    }
    return (double)r.num / (double)r.denom;
}

}

/** @} */

}
}
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0))
  indent-tabs-mode:nil
  fill-column:80
  End:
*/
