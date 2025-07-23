// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - xtrans.rs
 *
 * Copyright (C) 2025 Hubert Figuière
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
/*
 * This file as based on original code from dcraw.c rev 1.478
 * Copyright 1997-2018 by Dave Coffin, dcoffin a cybercom o net
 *
 * Orignal license statement for dcraw.c:
 * No license is required to download and use dcraw.c.  However,
 * to lawfully redistribute dcraw, you must either (a) offer, at
 * no extra charge, full source code* for all executable files
 * containing RESTRICTED functions, (b) distribute this code under
 * the GPL Version 2 or later, (c) remove all RESTRICTED functions,
 * re-implement them, or copy them from an earlier, unrestricted
 * Revision of dcraw.c, or (d) purchase a license from the author.
 *
 *
 * No portion of the RESTRICTED code was used.
 *
 * The code was translated by c2rust then manually converted to fit.
 */

//! X-Trans demoisaicking using Frank Markesteijn's algorithm

use multiversion::multiversion;
use once_cell::sync::Lazy;

use crate::bitmap::ImageBuffer;
use crate::mosaic::PatternColour;
use crate::{FloatPixel, Result};

/// Pixel type.
type PixelType = u16;
/// Signed pixel type (for CIELab)
type SignedPixelType = i16;

static COLOURS: usize = 3;
static RGB_CAM: [[f64; 4]; 3] = [
    [1.0, 0.0, 0.0, 0.0],
    [0.0, 1.0, 0.0, 0.0],
    [0.0, 0.0, 1.0, 0.0],
];
static XYZ_RGB: [[f64; 3]; 3] = [
    [0.412453, 0.357580, 0.180423],
    [0.212671, 0.715160, 0.072169],
    [0.019334, 0.119193, 0.950227],
];
const FLT_MAX: f32 = 3.402_823_5e38;

#[inline(always)]
fn fcol(row: isize, col: isize, xtrans: &[PatternColour]) -> PatternColour {
    xtrans[((row + 6) as usize % 6) * 6 + ((col + 6) as usize % 6)]
}

fn border_interpolate(image: &mut ImageBuffer<PixelType>, xtrans: &[PatternColour], border: usize) {
    let width = image.width;
    let height = image.height;

    for row in 0..height {
        let mut col = 0_usize;
        while col < width {
            if col == border && row >= border && row < (height - border) {
                col = width - border;
            }
            let mut sum: [u32; 8] = [0; 8];
            let mut y = row.wrapping_sub(1);
            while y != row.wrapping_add(2) {
                let mut x = col.wrapping_sub(1);
                while x != col.wrapping_add(2) {
                    if y < height && x < width {
                        let f = fcol(y as isize, x as isize, xtrans) as usize;
                        sum[f] =
                            sum[f].wrapping_add(image.data[image.pixel_index(y, x) + f] as u32);
                        sum[f.wrapping_add(4)] = (sum[f.wrapping_add(4)]).wrapping_add(1);
                    }
                    x = x.wrapping_add(1);
                }
                y = y.wrapping_add(1);
            }
            let f = fcol(row as isize, col as isize, xtrans) as usize;
            for c in 0..COLOURS {
                if c != f && sum[c.wrapping_add(4)] != 0 {
                    let index = image.pixel_index(row, col);
                    image.data[index + c] = (sum[c]).wrapping_div(sum[c.wrapping_add(4)]) as u16;
                }
            }
            col = col.wrapping_add(1);
        }
    }
}

#[multiversion(targets(
    "x86_64+avx+avx2+fma+bmi1+bmi2",
    "x86_64+avx+avx2",
    "x86+sse",
    "aarch64+neon"
))]
fn cielab(rgb: &[PixelType]) -> [SignedPixelType; 3] {
    static D65_WHITE: [f64; 3] = [0.950456, 1.0, 1.088754];
    static CBRT: Lazy<[f32; 65536]> = Lazy::new(|| {
        let mut cbrt: [f32; 65536] = [0.; 65536];
        for (i, value) in cbrt.iter_mut().enumerate() {
            let r = i as f64 / 65535.0;
            *value = (if r > 0.008856 {
                r.powf(1.0 / 3.0)
            } else {
                7.787 * r + 16.0 / 116.0
            }) as f32;
        }

        cbrt
    });
    static XYZ_CAM: Lazy<[[f32; 4]; 3]> = Lazy::new(|| {
        let mut xyz_cam: [[f32; 4]; 3] = [[0.; 4]; 3];
        for i in 0..3 {
            for j in 0..COLOURS {
                let mut k = 0_usize;
                xyz_cam[i][j] = k as f32;
                while k < 3 {
                    xyz_cam[i][j] = (xyz_cam[i][j] as f64
                        + XYZ_RGB[i][k] * RGB_CAM[k][j] / D65_WHITE[i])
                        as f32;
                    k += 1;
                }
            }
        }

        xyz_cam
    });

    let mut xyz: [f32; 3] = [0.5; 3];
    for (c, colours) in rgb.iter().enumerate().take(COLOURS) {
        xyz[0] += XYZ_CAM[0][c] * *colours as f32;
        xyz[1] += XYZ_CAM[1][c] * *colours as f32;
        xyz[2] += XYZ_CAM[2][c] * *colours as f32;
    }
    xyz[0] = CBRT[clip(xyz[0] as i32) as usize];
    xyz[1] = CBRT[clip(xyz[1] as i32) as usize];
    xyz[2] = CBRT[clip(xyz[1] as i32) as usize];
    [
        (64.0 * (116.0 * xyz[1] - 16.0)) as SignedPixelType,
        (64.0 * 500.0 * (xyz[0] - xyz[1])) as SignedPixelType,
        (64.0 * 200.0 * (xyz[1] - xyz[2])) as SignedPixelType,
    ]
}

#[inline(always)]
fn abs(x: i32) -> i32 {
    (x ^ (x >> 31)) - (x >> 31)
}

#[inline(always)]
fn sqr<T>(x: T) -> T
where
    T: std::ops::Mul<Output = T> + Copy + std::fmt::Display + num_traits::WrappingMul,
{
    T::wrapping_mul(&x, &x)
}

#[inline(always)]
fn lim(x: i32, min: i32, max: i32) -> i32 {
    std::cmp::max(min, std::cmp::min(x, max))
}

#[inline(always)]
fn clip(x: i32) -> i32 {
    lim(x, 0, 65535)
}

const UTS: usize = 512;
const TS: isize = 512;

/// Convert the buffer `finput` to have the expected layout...  Also
/// performs f64 -> u16.
///
/// The expected layout is chunky quad pixels with only the colour
/// filter component value at the location set. So a red pixal in the
/// mosaic would [ red, 0, 0, 0 ]
///
fn convert_buffer(
    finput: &ImageBuffer<FloatPixel>,
    xtrans: &[PatternColour],
) -> ImageBuffer<PixelType> {
    let buffer = vec![0_u16; finput.height * finput.width * 4];
    let mut image = ImageBuffer::with_data(buffer, finput.width, finput.height, 16, 4);
    for row in 0..finput.height {
        for col in 0..finput.width {
            let c = fcol(row as isize, col as isize, xtrans) as usize;
            let index = image.pixel_index(row, col);
            image.data[index + c] = (finput.data[finput.pixel_index(row, col)]
                * (PixelType::MAX as FloatPixel)) as PixelType;
        }
    }
    image
}

/// X-Trans interpolate.
///
/// `finput` is the raw data as a filtered pixmap.
/// `xtrans` is the xtrans pattern.
/// Output dimension are `output_width` and `output_height`
/// `passes` should be between 1 and 3. The bigger the slower.
///
pub(crate) fn xtrans_interpolate(
    finput: ImageBuffer<FloatPixel>,
    xtrans: &[PatternColour],
    output_width: usize,
    output_height: usize,
    passes: i32,
) -> Result<ImageBuffer<FloatPixel>> {
    let mut input = convert_buffer(&finput, xtrans);
    let width = input.width as usize;
    let mut hm: [i32; 8] = [0; 8];
    let mut colour: [[i32; 8]; 3] = [[0; 8]; 3];
    static ORTH: [isize; 12] = [1, 0, 0, 1, -1, 0, 0, -1, 1, 0, 0, 1];
    static PATT: [[isize; 16]; 2] = [
        [0, 1, 0, -1, 2, 0, -1, 0, 1, 1, 1, -1, 0, 0, 0, 0],
        [0, 1, 0, -2, 1, 0, -2, 0, 1, 1, -2, -2, 1, -1, -1, 1],
    ];
    static DIR: [isize; 4] = [1, TS, TS + 1, TS - 1];
    let mut allhex: [[[[isize; 8]; 2]; 3]; 3] = [[[[0; 8]; 2]; 3]; 3];
    let mut hex: *mut isize;
    let mut min: PixelType;
    let mut max: PixelType;
    let mut sgrow: u16 = 0;
    let mut sgcol: u16 = 0;
    let mut rix: *mut [PixelType; 3];
    let mut pix: *mut [PixelType; 4];
    let mut lix: *mut [SignedPixelType; 3];

    log::debug!("{passes}-pass X-Trans interpolation...");

    let ndir: usize = if passes > 1 { 8 } else { 4 };

    let mut buffer = vec![0_u16; UTS * UTS * ndir * 3];
    let mut rgb = buffer.as_mut_ptr() as *mut [[[PixelType; 3]; UTS]; UTS];

    let mut lab = vec![[[0_i16; 3]; UTS]; UTS];
    let mut drv = vec![[[0.0_f32; UTS]; UTS]; ndir];
    let mut homo = vec![[[0_u8; UTS]; UTS]; ndir];

    /* Map a green hexagon around each non-green pixel and vice versa:	*/
    #[allow(clippy::needless_range_loop)]
    for row in 0..3_usize {
        for col in 0..3_usize {
            let mut d = 0_usize;
            let mut ng = 0_usize;
            while d < 10 {
                let g = (fcol(row as isize, col as isize, xtrans) == PatternColour::Green) as usize;
                if fcol(row as isize + ORTH[d], col as isize + ORTH[d + 2], xtrans)
                    == PatternColour::Green
                {
                    ng = 0;
                } else {
                    ng += 1;
                }
                if ng == 4 {
                    sgrow = row as u16;
                    sgcol = col as u16;
                }
                if ng == g + 1 {
                    for c in 0..8_usize {
                        let v = ORTH[d] * PATT[g][c * 2] + ORTH[d + 1] * PATT[g][c * 2 + 1];
                        let h = ORTH[d + 2] * PATT[g][c * 2] + ORTH[d + 3] * PATT[g][c * 2 + 1];
                        allhex[row][col][0][c ^ (g * 2) & d] = (h + v * width as isize) as isize;
                        allhex[row][col][1][c ^ (g * 2) & d] = (h + v * TS) as isize;
                    }
                }
                d += 2;
            }
        }
    }

    /* Set green1 and green3 to the minimum and maximum allowed values:	*/
    let mut row = 2_usize;
    while row < (output_height - 2) {
        max = 0;
        min = !(max as i32) as PixelType;
        let mut col = 2_usize;
        while col < (output_width - 2) {
            if !(fcol(row as isize, col as isize, xtrans) == PatternColour::Green && {
                max = 0;
                min = !(max as i32) as PixelType;
                min as i32 != 0
            }) {
                let index = input.pixel_index(row, col);
                pix = input.data[index..index + 4].as_mut_ptr() as *mut [PixelType; 4];
                hex = (allhex[row % 3][col % 3][0]).as_mut_ptr();
                if max == 0 {
                    for c in 0..6_isize {
                        let val = unsafe { (*pix.offset(*hex.offset(c)))[1] };
                        if min > val {
                            min = val;
                        }
                        if max < val {
                            max = val;
                        }
                    }
                }
                unsafe {
                    (*pix)[1] = min;
                    (*pix)[3] = max;
                }
                match (row - sgrow as usize) % 3 {
                    1 => {
                        if row < output_height - 3 {
                            row += 1;
                            col -= 1;
                        }
                    }
                    2 => {
                        max = 0;
                        min = !(max as i32) as PixelType;
                        if min as i32 != 0
                            && {
                                col += 2;
                                col < output_width - 3
                            }
                            && row > 2
                        {
                            row -= 1;
                        }
                    }
                    _ => {}
                }
            }
            col += 1;
        }
        row += 1;
    }
    let mut top = 3_i32;
    while top < output_height as i32 - 19 {
        let mut left = 3_i32;
        while left < output_width as i32 - 19 {
            let mut mrow = std::cmp::min(top + TS as i32, output_height as i32 - 3);
            let mut mcol = std::cmp::min(left + TS as i32, output_width as i32 - 3);
            for row in top..mrow {
                for col in left..mcol {
                    unsafe {
                        std::ptr::copy_nonoverlapping(
                            input.data[input.pixel_index(row as usize, col as usize)..].as_ptr(),
                            ((*rgb)[(row - top) as usize][(col - left) as usize]).as_mut_ptr(),
                            3,
                        );
                    }
                }
            }
            for c in 0..3_usize {
                unsafe {
                    std::ptr::copy_nonoverlapping(
                        (*rgb).as_ptr() as *const [[[PixelType; 3]; UTS]; UTS],
                        (*rgb.add(c + 1)).as_mut_ptr() as *mut [[[PixelType; 3]; UTS]; UTS],
                        1,
                    );
                }
            }

            /* Interpolate green horizontally, vertically, and along both diagonals: */
            for row in top..mrow {
                for col in left..mcol {
                    let f = fcol(row as isize, col as isize, xtrans);
                    if f == PatternColour::Green {
                        continue;
                    }
                    let f = f as usize;
                    let index = input.pixel_index(row as usize, col as usize);
                    pix = input.data[index..index + 4].as_mut_ptr() as *mut [PixelType; 4];
                    hex = (allhex[row as usize % 3][col as usize % 3][0]).as_mut_ptr();
                    unsafe {
                        colour[1][0] = 174
                            * ((*pix.offset(*hex.add(1)))[1] as i32
                                + (*pix.offset(*hex))[1] as i32)
                            - 46 * ((*pix.offset(2 * *hex.add(1)))[1] as i32
                                + (*pix.offset(2 * *hex))[1] as i32);
                        colour[1][1] = 223 * (*pix.offset(*hex.offset(3)))[1] as i32
                            + (*pix.offset(*hex.add(2)))[1] as i32 * 33
                            + 92 * (*pix)[f] as i32
                            - (*pix.offset(-(*hex.add(2))))[f] as i32;
                    }
                    for c in 0..2_usize {
                        unsafe {
                            colour[1][2 + c] = 164 * (*pix.offset(*hex.add(4 + c)))[1] as i32
                                + 92 * (*pix.offset(-2 * *hex.add(4 + c)))[1] as i32
                                + 33 * (2 * (*pix)[f] as i32
                                    - (*pix.offset(3 * *hex.add(4 + c)))[f] as i32
                                    - (*pix.offset(-3 * *hex.add(4 + c)))[f] as i32);
                        }
                    }
                    for c in 0..4_usize {
                        unsafe {
                            (*rgb.offset(
                                (c as i32 ^ ((row - sgrow as i32) % 3 == 0) as i32) as isize,
                            ))[(row - top) as usize][(col - left) as usize][1] =
                                lim(colour[1][c] >> 8, (*pix)[1] as i32, (*pix)[3] as i32)
                                    as PixelType;
                        }
                    }
                }
            }

            let mut pass: i32 = 0;
            while pass < passes {
                if pass == 1 {
                    unsafe {
                        rgb = rgb.offset(4);
                        std::ptr::copy_nonoverlapping(
                            buffer.as_ptr() as *const [[[PixelType; 3]; UTS]; UTS],
                            rgb,
                            4,
                        );
                    }
                }

                /* Recalculate green from interpolated values of closer pixels:	*/
                if pass != 0 {
                    for row in (top + 2) as usize..(mrow - 2) as usize {
                        for col in (left + 2) as usize..(mcol - 2) as usize {
                            let f = fcol(row as isize, col as isize, xtrans);
                            if f == PatternColour::Green {
                                continue;
                            }
                            let f = f as usize;
                            let index = input.pixel_index(row, col);
                            pix = input.data[index..index + 4].as_mut_ptr() as *mut [PixelType; 4];
                            hex = (allhex[row % 3][col % 3][1]).as_mut_ptr();
                            for d in 3..6_usize {
                                rix = unsafe {
                                    &mut *(*(*rgb
                                        .add((d - 2) ^ ((row - sgrow as usize) % 3 == 0) as usize))
                                    .as_mut_ptr()
                                    .add(row - top as usize))
                                    .as_mut_ptr()
                                    .add(col - left as usize)
                                        as *mut [PixelType; 3]
                                };
                                unsafe {
                                    let val = (*rix.offset(-2 * *hex.add(d)))[1] as i32
                                        + 2 * (*rix.offset(*hex.add(d)))[1] as i32
                                        - (*rix.offset(-2 * *hex.add(d)))[f] as i32
                                        - 2 * (*rix.offset(*hex.add(d)))[f] as i32
                                        + 3 * (*rix)[f] as i32;
                                    (*rix)[1] = lim(val / 3, (*pix)[1] as i32, (*pix)[3] as i32)
                                        as PixelType;
                                }
                            }
                        }
                    }
                }

                /* Interpolate red and blue values for solitary green pixels:	*/
                let mut row = (top - sgrow as i32 + 4) / 3 * 3 + sgrow as i32;
                while row < mrow - 2 {
                    let mut col = (left - sgcol as i32 + 4) / 3 * 3 + sgcol as i32;
                    while col < mcol - 2 {
                        rix = unsafe {
                            &mut *(*(*rgb).as_mut_ptr().offset((row - top) as isize))
                                .as_mut_ptr()
                                .offset((col - left) as isize)
                                as *mut [PixelType; 3]
                        };
                        let mut h = fcol(row as isize, col as isize + 1, xtrans) as usize;
                        let mut diff = [0.; 6];
                        let mut i = 1_isize;
                        for d in 0..6_usize {
                            for c in 0..2 {
                                unsafe {
                                    let g = 2 * (*rix)[1] as i32
                                        - (*rix.offset(i << c))[1] as i32
                                        - (*rix.offset(-i << c))[1] as i32;
                                    colour[h as usize][d] = g
                                        + (*rix.offset(i << c))[h as usize] as i32
                                        + (*rix.offset(-i << c))[h as usize] as i32;
                                    if d > 1 {
                                        let value = sqr(((*rix.offset(i << c))[1] as i32)
                                            .wrapping_sub((*rix.offset(-i << c))[1] as i32)
                                            .wrapping_sub((*rix.offset(i << c))[h as usize] as i32)
                                            .wrapping_add(
                                                (*rix.offset(-i << c))[h as usize] as i32,
                                            ))
                                        .wrapping_add(sqr(g));
                                        diff[d] += value as f32;
                                    }
                                }
                                h ^= 2;
                                h = std::cmp::min(2, h);
                            }
                            if d > 1 && d & 1 != 0 && diff[d - 1] < diff[d] {
                                for c in 0..2_usize {
                                    colour[c * 2][d] = colour[c * 2][d - 1];
                                }
                            }
                            if d < 2 || d & 1 != 0 {
                                for c in 0..2_usize {
                                    unsafe {
                                        (*rix)[c * 2] = clip(colour[c * 2][d] / 2) as PixelType
                                    };
                                }
                                unsafe { rix = rix.add(UTS * UTS) };
                            }
                            i ^= TS ^ 1;
                            h ^= 2;
                        }
                        col += 3;
                    }
                    row += 3;
                }

                /* Interpolate red for blue pixels and vice versa: */
                for row in (top + 2) as usize..(mrow - 3) as usize {
                    for col in (left + 3) as usize..(mcol - 3) as usize {
                        let f = 2 - fcol(row as isize, col as isize, xtrans) as usize;
                        if f == 1 {
                            continue;
                        }
                        rix = unsafe {
                            &mut *(*(*rgb).as_mut_ptr().add(row - top as usize))
                                .as_mut_ptr()
                                .add(col - left as usize)
                                as *mut [PixelType; 3]
                        };
                        let c = if (row - sgrow as usize) % 3 != 0 {
                            TS
                        } else {
                            1
                        };
                        let h = 3 * (c ^ TS ^ 1);
                        for d in 0..4_isize {
                            let i: isize = unsafe {
                                if d > 1
                                    || (d ^ c) & 1 != 0
                                    || (abs((*rix)[1] as i32 - (*rix.offset(c))[1] as i32)
                                        + abs((*rix)[1] as i32 - (*rix.offset(-c))[1] as i32)
                                        < 2 * (abs((*rix)[1] as i32 - (*rix.offset(h))[1] as i32)
                                            + abs((*rix)[1] as i32 - (*rix.offset(-h))[1] as i32)))
                                {
                                    c
                                } else {
                                    h
                                }
                            };
                            unsafe {
                                (*rix)[f] = clip(
                                    ((*rix.offset(i))[f] as i32
                                        + (*rix.offset(-i))[f] as i32
                                        + 2 * (*rix)[1] as i32
                                        - (*rix.offset(i))[1] as i32
                                        - (*rix.offset(-i))[1] as i32)
                                        / 2,
                                ) as PixelType;
                                rix = rix.offset(TS * TS);
                            }
                        }
                    }
                }
                /* Fill in red and blue for 2x2 blocks of green */
                for row in (top + 2) as usize..(mrow - 2) as usize {
                    if (row - sgrow as usize) % 3 != 0 {
                        for col in (left + 2) as usize..(mcol - 2) as usize {
                            if (col - sgcol as usize) % 3 != 0 {
                                rix = unsafe {
                                    &mut *(*(*rgb).as_mut_ptr().add(row - top as usize))
                                        .as_mut_ptr()
                                        .add(col - left as usize)
                                        as *mut [PixelType; 3]
                                };
                                hex = (allhex[row % 3][col % 3][1]).as_mut_ptr();
                                let mut d = 0_usize;
                                while d < ndir {
                                    if unsafe { *hex.add(d) + *hex.add(d + 1) } != 0 {
                                        let g = unsafe {
                                            3 * (*rix)[1] as i32
                                                - 2 * (*rix.offset(*hex.add(d)))[1] as i32
                                                - (*rix.offset(*hex.add(d + 1)))[1] as i32
                                        };
                                        let mut c = 0_usize;
                                        while c < 4 {
                                            unsafe {
                                                (*rix)[c] = clip(
                                                    (g + 2 * (*rix.offset(*hex.add(d)))[c] as i32
                                                        + (*rix.offset(*hex.add(d + 1)))[c] as i32)
                                                        / 3,
                                                )
                                                    as PixelType;
                                            }
                                            c += 2;
                                        }
                                    } else {
                                        let g = unsafe {
                                            2 * (*rix)[1] as i32
                                                - (*rix.offset(*hex.add(d)))[1] as i32
                                                - (*rix.offset(*hex.add(d + 1)))[1] as i32
                                        };
                                        let mut c = 0_usize;
                                        while c < 4 {
                                            unsafe {
                                                (*rix)[c] = clip(
                                                    (g + (*rix.offset(*hex.add(d)))[c] as i32
                                                        + (*rix.offset(*hex.add(d + 1)))[c] as i32)
                                                        / 2,
                                                )
                                                    as PixelType;
                                            }
                                            c += 2;
                                        }
                                    }
                                    d += 2;
                                    unsafe {
                                        rix = rix.add(UTS * UTS);
                                    }
                                }
                            }
                        }
                    }
                }
                pass += 1;
            }
            rgb = buffer.as_mut_ptr() as *mut [[[PixelType; 3]; UTS]; UTS];
            mrow -= top;
            mcol -= left;

            /* Convert to CIELab and differentiate in all directions:	*/
            for d in 0..ndir {
                for (row, lab_row) in lab.iter_mut().enumerate().take((mrow - 2) as usize).skip(2) {
                    for (col, lab_value) in lab_row
                        .iter_mut()
                        .enumerate()
                        .take((mcol - 2) as usize)
                        .skip(2)
                    {
                        let rgb = unsafe {
                            std::slice::from_raw_parts(((*rgb.add(d))[row][col]).as_ptr(), 3)
                        };
                        *lab_value = cielab(rgb);
                    }
                }
                let f = DIR[d & 3];
                for (row, lab_row) in lab.iter_mut().enumerate().take((mrow - 3) as usize).skip(3) {
                    for (col, lab_value) in lab_row
                        .iter_mut()
                        .enumerate()
                        .take((mcol - 3) as usize)
                        .skip(3)
                    {
                        unsafe {
                            lix = &mut *(lab_value.as_mut_ptr() as *mut [SignedPixelType; 3]);
                            let g = 2 * (*lix)[0] as i32
                                - (*lix.offset(f))[0] as i32
                                - (*lix.offset(-f))[0] as i32;
                            drv[d][row][col] = (sqr(g)
                                + sqr(2 * (*lix)[1] as i32
                                    - (*lix.offset(f))[1] as i32
                                    - (*lix.offset(-f))[1] as i32
                                    + g * 500 / 232)
                                + sqr(2 * (*lix)[2] as i32
                                    - (*lix.offset(f))[2] as i32
                                    - (*lix.offset(-f))[2] as i32
                                    - g * 500 / 580))
                                as f32;
                        }
                    }
                }
            }

            /* Build homogeneity maps from the derivatives: */
            homo.iter_mut().flatten().flatten().for_each(|v| *v = 0);
            for row in 4..(mrow - 4) as usize {
                for col in 4..(mcol - 4) as usize {
                    let mut tr = FLT_MAX;
                    for value in drv.iter().take(ndir) {
                        if tr > value[row][col] {
                            tr = value[row][col];
                        }
                    }
                    tr *= 8.0;
                    for d in 0..ndir {
                        let mut v = -1;
                        while v <= 1 {
                            let mut h = -1;
                            while h <= 1 {
                                if drv[d][(row as i32 + v) as usize][(col as i32 + h) as usize]
                                    <= tr
                                {
                                    homo[d][row][col] += 1;
                                }
                                h += 1;
                            }
                            v += 1;
                        }
                    }
                }
            }

            /* Average the most homogenous pixels for the final result:	*/
            if output_height as i32 - top < 512 + 4 {
                mrow = output_height as i32 - top + 2;
            }
            if output_width as i32 - left < 512 + 4 {
                mcol = output_width as i32 - left + 2;
            }
            let first_row = if top < 8 { top } else { 8 };
            for row in first_row..mrow - 8 {
                let first_col = if left < 8 { left } else { 8 };
                for col in first_col..mcol - 8 {
                    for (d, homogenous) in hm.iter_mut().enumerate().take(ndir) {
                        *homogenous = 0;
                        let mut v = -2;
                        while v <= 2 {
                            let mut h = -2;
                            while h <= 2 {
                                *homogenous +=
                                    homo[d][(row + v) as usize][(col + h) as usize] as i32;
                                h += 1;
                            }
                            v += 1;
                        }
                    }
                    for d in 0..(ndir - 4) {
                        if hm[d] < hm[d + 4] {
                            hm[d] = 0;
                        } else if hm[d] > hm[d + 4] {
                            hm[d + 4] = 0;
                        }
                    }
                    max = hm[0] as PixelType;
                    for homogenous in hm.iter().take(ndir).skip(1) {
                        if (max as i32) < *homogenous {
                            max = *homogenous as PixelType;
                        }
                    }
                    max = (max as i32 - (max as i32 >> 3)) as PixelType;
                    let mut avg: [i32; 4] = [0; 4];
                    for (d, homogenous) in hm.iter().enumerate().take(ndir) {
                        if *homogenous >= max as i32 {
                            for (c, average) in avg.iter_mut().enumerate().take(3) {
                                unsafe {
                                    *average += (*rgb.add(d))[row as usize][col as usize][c] as i32;
                                }
                            }
                            avg[3] += 1;
                        }
                    }
                    for c in 0..3_usize {
                        let index = input.pixel_index((row + top) as usize, (col + left) as usize);
                        input.data[index + c] = (avg[c] / avg[3]) as PixelType;
                    }
                }
            }
            left += TS as i32 - 16;
        }
        top += TS as i32 - 16;
    }
    border_interpolate(&mut input, xtrans, 8);
    Ok(input.into_float_downsample(3))
}

#[cfg(test)]
mod test {

    use super::abs;
    use super::clip;
    use super::sqr;

    #[test]
    fn test_abs() {
        assert_eq!(abs(10), 10);
        assert_eq!(abs(-10), 10);
        assert_eq!(abs(-32000), 32000);
        assert_eq!(abs(32000), 32000);
    }

    #[test]
    fn test_clip() {
        assert_eq!(clip(1), 1);
        assert_eq!(clip(100_000), 65535);
        assert_eq!(clip(-100), 0);
    }

    #[test]
    fn test_sqr() {
        assert_eq!(sqr(9), 81);
    }

    #[test]
    #[should_panic]
    fn test_sqr_overflow() {
        assert_eq!(sqr(65535) as i64, 65535 * 65535);
    }
}
