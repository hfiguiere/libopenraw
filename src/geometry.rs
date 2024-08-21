// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - geometry.rs
 *
 * Copyright (C) 2022-2024 Hubert Figuière
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

//! Geometry define struct to define the geometry of things.

/// Rectangle struct.
#[derive(Clone, Debug, Default, PartialEq)]
pub struct Rect {
    pub x: u32,
    pub y: u32,
    pub width: u32,
    pub height: u32,
}

impl Rect {
    /// Create a rectangle from a [`Point`] and a [`Size`]
    pub fn new(origin: Point, size: Size) -> Rect {
        Rect {
            x: origin.x,
            y: origin.y,
            width: size.width,
            height: size.height,
        }
    }

    /// The origin of the `Rect`.
    pub fn origin(&self) -> Point {
        Point {
            x: self.x,
            y: self.y,
        }
    }

    /// The size of the `Rect`.
    pub fn size(&self) -> Size {
        Size {
            width: self.width,
            height: self.height,
        }
    }

    /// Generate a `Vec<u32>` with the values in x, y, w, h order.
    pub fn to_vec(&self) -> Vec<u32> {
        [self.x, self.y, self.width, self.height].to_vec()
    }
}

/// Point struct
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Point {
    pub x: u32,
    pub y: u32,
}

/// Size struct
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Size {
    pub width: u32,
    pub height: u32,
}

/// Aspect ratio. Like 1:1, 16:9, 4:3, 3:2, 65:24
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct AspectRatio(pub u32, pub u32);

impl AspectRatio {
    /// Crop the size to the aspect ratio.
    pub fn crop(&self, from: Size) -> Size {
        let ratio = self.0 as f64 / self.1 as f64;
        if (from.width as f64 / from.height as f64) > ratio {
            let new_width = from.height as f64 * ratio;
            Size {
                width: new_width as u32,
                height: from.height,
            }
        } else {
            let new_height = from.width as f64 / ratio;
            Size {
                width: from.width,
                height: new_height as u32,
            }
        }
    }

    /// Crop the rectangle into the aspect ratio.
    pub fn crop_into(&self, from: &Rect) -> Rect {
        let cropped_size = self.crop(from.size());
        let offset_y = (from.height as i64 - cropped_size.height as i64).abs() / 2;
        let offset_x = (from.width as i64 - cropped_size.width as i64).abs() / 2;
        Rect::new(
            Point {
                x: from.x + offset_x as u32,
                y: from.y + offset_y as u32,
            },
            cropped_size,
        )
    }
}

impl std::fmt::Display for AspectRatio {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> Result<(), std::fmt::Error> {
        write!(f, "{}:{}", self.0, self.1)
    }
}

#[cfg(test)]
mod test {
    use super::{AspectRatio, Point, Rect, Size};

    #[test]
    fn test_aspect_ratio_crop() {
        let size = Size {
            width: 5472,
            height: 3648,
        };

        let aspect_ratio = AspectRatio(3, 2);
        assert_eq!(size, aspect_ratio.crop(size), "3:2");

        let aspect_ratio = AspectRatio(16, 9);
        assert_eq!(
            Size {
                width: 5472,
                height: 3078
            },
            aspect_ratio.crop(size),
            "16:9"
        );

        let aspect_ratio = AspectRatio(4, 3);
        assert_eq!(
            Size {
                width: 4864,
                height: 3648
            },
            aspect_ratio.crop(size),
            "4:3"
        );

        let aspect_ratio = AspectRatio(1, 1);
        assert_eq!(
            Size {
                width: 3648,
                height: 3648
            },
            aspect_ratio.crop(size),
            "1:1"
        );
    }

    #[test]
    fn test_aspect_ratio_crop_into() {
        let rect = Rect {
            x: 0,
            y: 0,
            width: 5472,
            height: 3648,
        };

        let aspect_ratio = AspectRatio(3, 2);
        let dest = aspect_ratio.crop_into(&rect);
        assert_eq!(Point { x: 0, y: 0 }, dest.origin(), "3:2");

        let aspect_ratio = AspectRatio(16, 9);
        let dest = aspect_ratio.crop_into(&rect);
        assert_eq!(Point { x: 0, y: 285 }, dest.origin(), "16:9");

        let aspect_ratio = AspectRatio(4, 3);
        let dest = aspect_ratio.crop_into(&rect);
        assert_eq!(Point { x: 304, y: 0 }, dest.origin(), "4:3");

        let aspect_ratio = AspectRatio(1, 1);
        let dest = aspect_ratio.crop_into(&rect);
        assert_eq!(Point { x: 912, y: 0 }, dest.origin(), "1:1");
    }

    #[test]
    fn test_aspect_ratio_crop_into_w_offset() {
        let rect = Rect {
            x: 10,
            y: 10,
            width: 5472,
            height: 3648,
        };

        let aspect_ratio = AspectRatio(3, 2);
        let dest = aspect_ratio.crop_into(&rect);
        assert_eq!(Point { x: 10, y: 10 }, dest.origin(), "3:2");

        let aspect_ratio = AspectRatio(16, 9);
        let dest = aspect_ratio.crop_into(&rect);
        assert_eq!(Point { x: 10, y: 295 }, dest.origin(), "16:9");

        let aspect_ratio = AspectRatio(4, 3);
        let dest = aspect_ratio.crop_into(&rect);
        assert_eq!(Point { x: 314, y: 10 }, dest.origin(), "4:3");

        let aspect_ratio = AspectRatio(1, 1);
        let dest = aspect_ratio.crop_into(&rect);
        assert_eq!(Point { x: 922, y: 10 }, dest.origin(), "1:1");
    }
}
