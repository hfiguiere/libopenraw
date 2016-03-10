/* 
 * libopenraw - demosaic.cpp
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
 * This code has been adapted from GEGL:
 * Copyright 2006 Øyvind Kolås <pippin@gimp.org>
 * Copyright 2008 Bradley Broom <bmbroom@gmail.com>
 *
 * In libopenraw:
 * Copyright 2008-2015 Hubert Figuiere <hub@figuiere.net>
 * Copyright 2008 Novell Inc.
 */

#include <stdlib.h>

#include <algorithm>

#include <libopenraw/consts.h>

#include "bimedian_demosaic.hpp"


/* Returns the median of four floats. We define the median as the average of
 * the central two elements.
 */
static inline double
m4 (double a, double b, double c, double d)
{
    double t;
	
    /* Sort ab */
    if (a > b)
    {
        t = b;
        b = a;
        a = t;
    }
    /* Sort abc */
    if (b > c)
    {
        t = c;
        c = b;
        if (a > t)
        {
            b = a;
            a = t;
        }
        else
            b = t;
    }
    /* Return average of central two elements. */
    if (d >= c) /* Sorted order would be abcd */
        return (b + c) / 2.0;
    else if (d >= a) /* Sorted order would be either abdc or adbc */
        return (b + d) / 2.0;
    else /* Sorted order would be dabc */
        return (a + b) / 2.0;
}

/* Defines to make the row/col offsets below obvious. */
#define ROW src_x
#define COL 1

/* We expect src_extent to have a one pixel border around all four sides
 * of dst_extent.
 */
or_error
bimedian_demosaic (uint16_t *src, uint32_t src_x, uint32_t src_y, 
             or_cfa_pattern pattern, uint8_t *dst, uint32_t &out_x, uint32_t &out_y)
{
    uint32_t x,y;
    uint32_t offset, doffset;
    double *src_buf;
    double *dst_buf;

    int npattern = 0;
    switch(pattern) {
    case OR_CFA_PATTERN_GRBG:
        npattern = 0;
        break;
    case OR_CFA_PATTERN_BGGR:
        npattern = 1;
        break;
    case OR_CFA_PATTERN_GBRG:
        npattern = 2;
        break;
    case OR_CFA_PATTERN_RGGB:
        npattern = 3;
        break;

    default:
        // invalid
        return OR_ERROR_INVALID_FORMAT;
    }

    out_x = out_y = 0;
    src_buf = (double*)calloc(src_x * src_y, sizeof(*src_buf));
    dst_buf = (double*)calloc(src_x * src_y * 3, sizeof(*dst_buf));

    std::copy(src, src + (src_x * src_y), src_buf);

    offset = ROW + COL;
    doffset = 0;
    for(y = 1 ; y < src_y - 1; y++)
    {
        for (x = 1 ; x < src_x - 1; x++)
        {
            double red=0.0;
            double green=0.0;
            double blue=0.0;
			
            if ((y + npattern%2)%2==0) {
                if ((x+npattern/2)%2==1) {
                    /* GRG
                     * BGB
                     * GRG
                     */
                    blue =(src_buf[offset-COL]+src_buf[offset+COL])/2.0;
                    green=src_buf[offset];
                    red  =(src_buf[offset-ROW]+src_buf[offset+ROW])/2.0;
                }
                else {
                    /* RGR
                     * GBG
                     * RGR
                     */
                    blue =src_buf[offset];
                    green=m4(src_buf[offset-ROW], src_buf[offset-COL],
                             src_buf[offset+COL], src_buf[offset+ROW]);
                    red  =m4(src_buf[offset-ROW-COL], src_buf[offset-ROW+COL],
                             src_buf[offset+ROW-COL], src_buf[offset+ROW+COL]);
                }
            }
            else {
                if ((x+npattern/2)%2==1) {
                    /* BGB
                     * GRG
                     * BGB
                     */
                    blue =m4(src_buf[offset-ROW-COL], src_buf[offset-ROW+COL],
                             src_buf[offset+ROW-COL], src_buf[offset+ROW+COL]);
                    green=m4(src_buf[offset-ROW], src_buf[offset-COL],
                             src_buf[offset+COL], src_buf[offset+ROW]);
                    red  =src_buf[offset];
                }
                else {
                    /* GBG
                     * RGR
                     * GBG
                     */
                    blue =(src_buf[offset-ROW]+src_buf[offset+ROW])/2.0;
                    green=src_buf[offset];
                    red  =(src_buf[offset-COL]+src_buf[offset+COL])/2.0;
                }
            }
			
            dst_buf [doffset*3+0] = red / 16.0;
            dst_buf [doffset*3+1] = green / 16.0;
            dst_buf [doffset*3+2] = blue / 16.0;
			
            offset++;
            doffset++;
        }
        offset+=2;
    }
    out_x = src_x - 2;
    out_y = src_y - 2;
    std::copy(dst_buf, dst_buf + (out_x * out_y * 3), dst);		
    free(src_buf);
    free(dst_buf);

    return OR_ERROR_NONE;
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
