/*
 * libopenraw - prefix.h
 *
 * Copyright (C) 2012 Hubert Figuiere
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
 * @brief the prefix public API header. Must be top include most of the time.
 * @author Hubert Figuiere <hub@figuiere.net>
 */

#ifndef LIBOPENRAW_PREFIX_H_
#define LIBOPENRAW_PREFIX_H_

#if (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1))
#define OR_DEPRECATED  __attribute__((__deprecated__))
#else
#define OR_DEPRECATED
#endif /* __GNUC__ */

#endif
