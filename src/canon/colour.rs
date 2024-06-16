// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - canon.rs
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

//! Canon colour data.
//! Source:
//!  ExifTool <https://exiftool.org/TagNames/Canon.html#ColorData1>
//!  RawSpeed <https://github.com/darktable-org/rawspeed/blob/a79bd3878389159a44b72b0e3eac9dca8a46568a/src/librawspeed/decoders/Cr2Decoder.cpp#L250>

use std::convert::TryInto;

/// Colour format.
#[derive(Debug)]
pub(crate) enum ColourFormat {
    ColourData1,
    ColourData2,
    ColourData3,
    ColourData4,
    ColourData5,
    ColourData6,
    ColourData7,
    ColourData8,
    ColourData9,
    ColourData10,
    ColourData11,
}

impl ColourFormat {
    /// Identify the ColourData struct from `colour_data`.
    ///
    /// The first u16 is a version field, except for ColourData1 and
    /// ColourData2.  So the length of the `colour_data` block may be
    /// used.
    pub(crate) fn identify(colour_data: &[u16]) -> Option<Self> {
        use ColourFormat::*;
        log::debug!(
            "Probing ColourData. (len={} version={})",
            colour_data.len(),
            colour_data[0] as i16
        );

        match colour_data.len() {
            // 20D and 350D
            582 => Some(ColourData1),
            // 1DmkII and 1DSmkII
            653 => Some(ColourData2),

            // These are CR3.

            // M50, EOS R, EOS RP, SX70, M6mkII,
            // M200, 90D, 250D and 850D
            1816 | 1820 | 1824 => Some(ColourData9),
            // 1DXmkIII, R5, R6
            2024 | 3656 => Some(ColourData10),
            // R3, R7 and R6mkII
            3973 | 3778 => Some(ColourData11),
            _ => {
                // The first u16 is a version field (i16), except for
                // ColourData1 and ColourData2.
                match colour_data[0] as i16 {
                    // 1DmkIIN, 5D, 30D, 400D
                    1 => Some(ColourData3),
                    // 1DmkIII, 1DSmkIII, 1DmkIV, 5DmkII, 7D,
                    // 40D, 50D, 60D, 450D, 500D, 550D, 1000D
                    // and 1100D (9).
                    2..=7 | 9 => Some(ColourData4),
                    // (-3) G10, G11, G12, G15, G16, G1X, G1XMkII, G7X, G5X,
                    // (-4) G7X MkII, G1XMkIII
                    -4..=-3 => Some(ColourData5),
                    10 | 11 => {
                        match colour_data.len() {
                            // 600D, 1200D
                            1273 | 1275 => Some(ColourData6),
                            // some are version 10, some are 11
                            // 70D, 1DX firmware 1.x, 1DX, 70D,
                            // 5DmkIII, 6D, 7DmkII, 100D, 650D, 700D,
                            // 8000D, M and M2.
                            // ColourData7
                            _ => Some(ColourData7),
                        }
                    }
                    // 5DS/5DSR, 80D, 1300D, EOS 1DXmkII,
                    // 5DmkIV, 6DmkII, 77D, 80D, 200D,
                    // 800D, 1300D, 2000D, 4000D and 9000D.
                    12..=15 => Some(ColourData8),
                    _ => {
                        log::error!(
                            "Unknown ColourData. (len={} version={})",
                            colour_data.len(),
                            colour_data[0] as i16
                        );
                        None
                    }
                }
            }
        }
    }

    /// Extract the black levels if they exist.
    pub(crate) fn blacks(&self, colour_data: &[u16]) -> Option<[u16; 4]> {
        use ColourFormat::*;

        let version = colour_data[0];
        match *self {
            ColourData1 | ColourData2 => None,
            ColourData3 => colour_data[196..200].try_into().ok(),
            ColourData4 => match version {
                2 | 3 => None,
                4 | 5 => colour_data[692..696].try_into().ok(),
                6 | 7 => colour_data[715..719].try_into().ok(),
                9 => colour_data[719..723].try_into().ok(),
                _ => {
                    log::error!("Invalid version {} for {:?}", colour_data[0], self);
                    None
                }
            },
            ColourData5 => match version as i16 {
                -4 => colour_data[333..337].try_into().ok(),
                -3 => colour_data[264..268].try_into().ok(),
                _ => {
                    log::error!("Invalid version {} for {:?}", version, self);
                    None
                }
            },
            ColourData6 => colour_data[479..483].try_into().ok(),
            ColourData7 => match version {
                10 => colour_data[504..508].try_into().ok(),
                11 => colour_data[728..732].try_into().ok(),
                _ => {
                    log::error!("Invalid version {} for {:?}", version, self);
                    None
                }
            },
            ColourData8 => match version {
                14 => colour_data[556..560].try_into().ok(),
                12 | 13 | 15 => colour_data[778..782].try_into().ok(),
                _ => {
                    log::error!("Invalid version {} for {:?}", version, self);
                    None
                }
            },
            ColourData9 => colour_data[329..333].try_into().ok(),
            ColourData10 => colour_data[343..347].try_into().ok(),
            ColourData11 => colour_data[363..367].try_into().ok(),
        }
    }

    /// Convert the color data from the MakerNote to a white balance triple
    ///
    pub(crate) fn as_shot(&self, colour_data: &[u16]) -> Option<[f64; 3]> {
        use ColourFormat::*;

        match *self {
            ColourData1 => {
                let r = colour_data[25] as f64;
                let g = colour_data[25 + 1] as f64;
                let b = colour_data[25 + 3] as f64;
                Some([g / r, 1.0, g / b])
            }
            ColourData2 => {
                let r = colour_data[24] as f64;
                let g = colour_data[24 + 1] as f64;
                let b = colour_data[24 + 3] as f64;
                Some([g / r, 1.0, g / b])
            }
            ColourData3 | ColourData4 | ColourData6 | ColourData7 | ColourData8 => {
                let r = colour_data[63] as f64;
                let g = colour_data[63 + 1] as f64;
                let b = colour_data[63 + 3] as f64;
                Some([g / r, 1.0, g / b])
            }
            ColourData5 | ColourData9 => {
                let r = colour_data[71] as f64;
                let g = colour_data[71 + 1] as f64;
                let b = colour_data[71 + 3] as f64;
                Some([g / r, 1.0, g / b])
            }
            ColourData10 => {
                let r = colour_data[85] as f64;
                let g = colour_data[85 + 1] as f64;
                let b = colour_data[85 + 3] as f64;
                Some([g / r, 1.0, g / b])
            }
            ColourData11 => {
                let r = colour_data[105] as f64;
                let g = colour_data[105 + 1] as f64;
                let b = colour_data[105 + 3] as f64;
                Some([g / r, 1.0, g / b])
            }
        }
    }
}
