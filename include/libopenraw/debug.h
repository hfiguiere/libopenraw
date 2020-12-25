/*
 * libopenraw - debug.h
 *
 * Copyright (C) 2006 Hubert Figuiere
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

#ifndef LIBOPENRAW_DEBUG_H_
#define LIBOPENRAW_DEBUG_H_

/** @addtogroup public_api
 *
 * @{
 */
#ifdef __cplusplus
extern "C" {
#endif

    /** @brief Debug levels. */
    typedef enum _debug_level {
        ERROR = 0,
        WARNING,
        NOTICE,
        DEBUG1,
        DEBUG2
    } debug_level;

    /** @brief Set the debug level. */
    void or_debug_set_level(debug_level lvl);

#ifdef __cplusplus
}
#endif
/** @} */

#endif
