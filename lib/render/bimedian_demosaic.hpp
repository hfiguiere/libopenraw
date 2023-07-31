/*
 * libopenraw - bimedian_demosaic.hpp
 *
 * Copyright 2010-2018 Hubert Figuiere <hub@figuiere.net>
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
 *
 */

#pragma once

#include <stdint.h>

#include <libopenraw/consts.h>

extern "C" or_error
bimedian_demosaic (uint16_t *src, uint32_t src_x, uint32_t src_y,
		   or_cfa_pattern pattern, uint16_t *dst,
                   uint32_t &out_x, uint32_t &out_y);
