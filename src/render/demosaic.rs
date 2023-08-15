// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - render/demosaic.rs
 *
 * Copyright (C) 2023 Hubert Figuière
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

//! Implement demosaic

use crate::bitmap::ImageBuffer;

use crate::{
    mosaic::{Pattern, PatternType},
    Error,
};

/// Calculate the median of 4 float.
fn m4(mut a: f64, mut b: f64, mut c: f64, d: f64) -> f64 {
    /* Sort ab */
    if a > b {
        std::mem::swap(&mut a, &mut b);
    }
    /* Sort abc */
    if b > c {
        let t = c;
        c = b;
        if a > t {
            b = a;
            a = t;
        } else {
            b = t;
        }
    }
    /* Return average of central two elements. */
    if d >= c {
        // Sorted order would be abcd
        (b + c) / 2.0
    } else if d >= a {
        // Sorted order would be either abdc or adbc
        (b + d) / 2.0
    } else {
        // Sorted order would be dabc
        (a + b) / 2.0
    }
}

/// Bimedian demosaic for 2x2 bayer CFA. Use float 0..1.0 range.
pub(crate) fn bimedian(
    input: &ImageBuffer<f64>,
    pattern: &Pattern,
) -> crate::Result<ImageBuffer<f64>> {
    let npattern = match pattern.pattern_type() {
        PatternType::Bggr => 0,
        PatternType::Grbg => 1,
        PatternType::Gbrg => 2,
        PatternType::Rggb => 3,
        _ => return Err(Error::InvalidFormat),
    };

    let mut dst: Vec<f64> = vec![0.0; input.width as usize * input.height as usize * 3];

    #[allow(non_snake_case)]
    // Offset to get the same column on next row (or previous if negative)
    let DROW: usize = input.width as usize;
    // Offset to et the next or previous column
    const DCOL: usize = 1;
    // We start on column one of row one (0 based)
    let mut offset = DROW + DCOL;
    let mut doffset = 0;
    let src = &input.data;

    for y in 1..input.height - 1 {
        for x in 1..input.width - 1 {
            let red: f64;
            let green: f64;
            let blue: f64;

            if (y + npattern % 2) % 2 == 0 {
                if (x + npattern / 2) % 2 == 1 {
                    /* GRG
                     * BGB
                     * GRG
                     */
                    blue = (src[offset - DCOL] + src[offset + DCOL]) / 2.0;
                    green = src[offset];
                    red = (src[offset - DROW] + src[offset + DROW]) / 2.0;
                } else {
                    /* RGR
                     * GBG
                     * RGR
                     */
                    blue = src[offset];
                    green = m4(
                        src[offset - DROW],
                        src[offset - DCOL],
                        src[offset + DCOL],
                        src[offset + DROW],
                    );
                    red = m4(
                        src[offset - DROW - DCOL],
                        src[offset - DROW + DCOL],
                        src[offset + DROW - DCOL],
                        src[offset + DROW + DCOL],
                    );
                }
            } else if (x + npattern / 2) % 2 == 1 {
                /* BGB
                 * GRG
                 * BGB
                 */
                blue = m4(
                    src[offset - DROW - DCOL],
                    src[offset - DROW + DCOL],
                    src[offset + DROW - DCOL],
                    src[offset + DROW + DCOL],
                );
                green = m4(
                    src[offset - DROW],
                    src[offset - DCOL],
                    src[offset + DCOL],
                    src[offset + DROW],
                );
                red = src[offset];
            } else {
                /* GBG
                 * RGR
                 * GBG
                 */
                blue = (src[offset - DROW] + src[offset + DROW]) / 2.0;
                green = src[offset];
                red = (src[offset - DCOL] + src[offset + DCOL]) / 2.0;
            }

            dst[doffset * 3] = red;
            dst[doffset * 3 + 1] = green;
            dst[doffset * 3 + 2] = blue;

            offset += 1;
            doffset += 1;
        }
        // We must skip 2 each row.
        offset += 2;
    }
    let out_w = input.width - 2;
    let out_h = input.height - 2;
    // This is necessary to have a consistent size with the output.
    // Notably, the `image` crate doesn't like it.
    // The assumption is that the resize should shrink the buffer.
    dst.resize((3 * out_w * out_h) as usize, 0.0);

    Ok(ImageBuffer::with_data(dst, out_w, out_h, input.bpc, 3))
}

#[cfg(test)]
mod test {
    use crate::bitmap::ImageBuffer;
    use crate::mosaic::Pattern;

    use super::bimedian;

    /// Demosaic. `result is the value of the pixel at 1,1.
    fn test_demosaic(buffer: Vec<f64>, pattern: &Pattern, result: Vec<f64>) {
        let image = ImageBuffer::with_data(buffer, 8, 8, 16, 1);
        let output = bimedian(&image, pattern);
        assert!(output.is_ok());
        let output = output.unwrap();
        assert_eq!(output.width, 6);
        assert_eq!(output.height, 6);
        assert_eq!(output.cc, 3);
        assert_eq!(output.data.len(), 36 * output.cc as usize);
        println!("{:?}: {:?}", pattern, output.data);
        assert_eq!(
            output.pixel_at(0, 0),
            Some(result),
            "Demosaic {:?} Failed",
            pattern
        );
    }

    #[test]
    fn test_demosaic_xggx() {
        #[rustfmt::skip]
        let buffer = vec![
            0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0,
            1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0,
            0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0,
            1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0,
            0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0,
            1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0,
            0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0,
            1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0,
        ];

        test_demosaic(buffer.clone(), &Pattern::Rggb, vec![0.0_f64, 1.0, 0.0]);
        test_demosaic(buffer, &Pattern::Bggr, vec![0.0_f64, 1.0, 0.0]);
    }

    #[test]
    fn test_demosaic_gxxg() {
        #[rustfmt::skip]
        let buffer = vec![
            0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0,
            0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        ];

        test_demosaic(buffer.clone(), &Pattern::Gbrg, vec![0.0_f64, 0.0, 1.0]);
        test_demosaic(buffer, &Pattern::Grbg, vec![1.0_f64, 0.0, 0.0]);
    }
}
