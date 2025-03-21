// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - decompress/tiled.rs
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

//! Tiled JPEG decompression

use rayon::prelude::*;

use crate::bitmap::Bitmap;
use crate::{DataType, RawImage, Result};

use super::ljpeg::{LJpeg, Tile};

pub struct TiledLJpeg {}

impl TiledLJpeg {
    pub fn new() -> TiledLJpeg {
        TiledLJpeg {}
    }

    /// Combine tiles into the final buffer. `width` and `height` are the final dimensions.
    fn combine_tiles(width: usize, height: usize, tiles: &[Option<Tile>]) -> Vec<u16> {
        let data_size = width * height;
        let mut buffer = uninit_vec!(data_size);

        let mut first_row = 0_usize;
        let mut first_col = 0_usize;

        let buf_slice = buffer.as_mut_slice();
        for tile in tiles {
            if tile.is_none() {
                log::error!("tile is none");
                continue;
            }
            let tile = tile.as_ref().unwrap();
            // Edge tiles might be wider than necessary, ie width
            // isn't a multiple of the image width.
            let row_len = std::cmp::min(tile.u_width as usize, width - first_col);
            for r in 0..tile.u_height as usize {
                if first_row + r >= height {
                    // We are past the last row. This is because the
                    // tiles aren't necessarily a multiple of the
                    // image size. Mostly in DNG converted.
                    break;
                }
                let pos = (first_row + r) * width + first_col;
                let tile_pos = r * tile.u_width as usize;
                buf_slice[pos..][..row_len].copy_from_slice(&tile.buf[tile_pos..][..row_len]);
            }
            first_col += tile.u_width as usize;
            if first_col >= width {
                first_col = 0;
                first_row += tile.u_height as usize;
                if first_row >= height {
                    break;
                }
            }
        }

        buffer
    }

    /// Decompress the RawImage into a new RawImage.
    pub fn decompress(
        &self,
        rawdata: RawImage,
        #[cfg(feature = "probe")] probe: &Option<crate::Probe>,
    ) -> Result<RawImage> {
        if let Some(tiles) = rawdata.tile_data() {
            probe!(probe, "ljpeg.tiled", "true");
            let tile_size = rawdata.tile_size();
            let dec_tiles: Vec<Option<Tile>> = tiles
                .par_iter()
                .map(|tile| {
                    log::debug!("Decompressing tile");
                    // Tiles should be fine to have `is_raw` set to false.
                    let mut decompressor = LJpeg::new(false);
                    decompressor
                        .decompress_buffer(
                            tile.as_slice(),
                            true,
                            #[cfg(feature = "probe")]
                            &None,
                        )
                        .ok()
                })
                .map(|tile| {
                    tile.map(|mut tile| {
                        if let Some(tile_size) = tile_size {
                            tile.u_width = tile_size.0;
                            tile.u_height = tile_size.1;
                        }
                        tile
                    })
                })
                .collect();

            let data = Self::combine_tiles(
                rawdata.width() as usize,
                rawdata.height() as usize,
                &dec_tiles,
            );

            let mut rawdata = rawdata.replace_data(data);
            rawdata.set_data_type(DataType::Raw);

            Ok(rawdata)
        } else {
            Ok(rawdata)
        }
    }
}

#[cfg(test)]
mod test {

    use super::TiledLJpeg;
    use crate::decompress::ljpeg::Tile;

    #[test]
    fn test_combine_tiles() {
        let width = 4_usize;
        let height = 4_usize;
        let tiles = vec![
            Some(Tile {
                width: 2,
                height: 2,
                u_width: 2,
                u_height: 2,
                bpc: 16,
                buf: vec![100; 4],
            }),
            Some(Tile {
                width: 2,
                height: 2,
                u_width: 2,
                u_height: 2,
                bpc: 16,
                buf: vec![200; 4],
            }),
            Some(Tile {
                width: 2,
                height: 2,
                u_width: 2,
                u_height: 2,
                bpc: 16,
                buf: vec![300; 4],
            }),
            Some(Tile {
                width: 2,
                height: 2,
                u_width: 2,
                u_height: 2,
                bpc: 16,
                buf: vec![400; 4],
            }),
        ];

        let output = TiledLJpeg::combine_tiles(width, height, &tiles);
        assert_eq!(output[0], 100);
        assert_eq!(output[2], 200);
        assert_eq!(output[4], 100);
        assert_eq!(output[8], 300);
        assert_eq!(output[10], 400);
    }
}
