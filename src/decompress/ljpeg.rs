// SPDX-License-Identifier: LGPL-3.0-or-later AND IJG
/*
 * libopenraw - decompress/ljpeg.rs
 *
 * Copyright (C) 2022-2023 Hubert Figuière
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

// This code is a Rust adaptation of a C++ adaptation of IJG software with
// the follow notice:
/*
 * Code for JPEG lossless decoding.  Large parts are grabbed from the IJG
 * software, so:
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * Part of the Independent JPEG Group's software.
 * See the file Copyright for more details.
 *
 * Copyright (c) 1993 Brian C. Smith, The Regents of the University
 * of California
 * All rights reserved.
 *
 * Copyright (c) 1994 Kongji Huang and Brian C. Smith.
 * Cornell University
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * IN NO EVENT SHALL CORNELL UNIVERSITY BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF CORNELL
 * UNIVERSITY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * CORNELL UNIVERSITY SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND CORNELL UNIVERSITY HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

//! Lossless JPEG decompressor.

use std::io::SeekFrom;

use byteorder::{BigEndian, ReadBytesExt};

use super::sliced_buffer::SlicedBuffer;
use crate::mosaic::Pattern;
use crate::rawfile::ReadAndSeek;
use crate::{DataType, Error, RawData, Result};

const M_SOF0: u8 = 0xc0;
const M_SOF1: u8 = 0xc1;
const M_SOF2: u8 = 0xc2;
const M_SOF3: u8 = 0xc3;

const M_SOF5: u8 = 0xc5;
const M_SOF6: u8 = 0xc6;
const M_SOF7: u8 = 0xc7;

const M_JPG: u8 = 0xc8;
const M_SOF9: u8 = 0xc9;
const M_SOF10: u8 = 0xca;
const M_SOF11: u8 = 0xcb;

const M_SOF13: u8 = 0xcd;
const M_SOF14: u8 = 0xce;
const M_SOF15: u8 = 0xcf;

const M_DHT: u8 = 0xc4;

const M_RST0: u8 = 0xd0;
const M_RST1: u8 = 0xd1;
const M_RST2: u8 = 0xd2;
const M_RST3: u8 = 0xd3;
const M_RST4: u8 = 0xd4;
const M_RST5: u8 = 0xd5;
const M_RST6: u8 = 0xd6;
const M_RST7: u8 = 0xd7;

const M_SOI: u8 = 0xd8;
const M_EOI: u8 = 0xd9;
const M_SOS: u8 = 0xda;
const M_DQT: u8 = 0xdb;
const M_DRI: u8 = 0xdd;

const M_APP0: u8 = 0xe0;

const M_TEM: u8 = 0x01;

//const M_ERROR:u8 = 0x100

const BITS_PER_LONG: u8 = 8 * std::mem::size_of::<i32>() as u8;
const MIN_GET_BITS: u8 = BITS_PER_LONG - 7; // max value for long get_buffer

// bmask[n] is mask for n rightmost bits
const BMASK: [u16; 17] = [
    0x0000, 0x0001, 0x0003, 0x0007, 0x000F, 0x001F, 0x003F, 0x007F, 0x00FF, 0x01FF, 0x03FF, 0x07FF,
    0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF,
];

// Lossless JPEG specifies data precision to be from 2 to 16 bits/sample.
const MIN_PRECISION_BITS: u8 = 2;
const MAX_PRECISION_BITS: u8 = 16;

type ComponentType = u16;
type Mcu = Vec<ComponentType>;

/// A tile
pub struct Tile {
    /// Width of the data
    pub width: u32,
    /// Height of the data
    pub height: u32,
    /// Useful width
    pub u_width: u32,
    /// Useful height
    pub u_height: u32,
    pub bpc: u16,
    pub buf: Vec<u16>,
}

pub struct LJpeg {
    /// Canon-style slices. The format of the slices vector is as follow
    /// * N col1 col2
    /// * N is the number of repeat for col1. The total
    /// number of slices is always N+1
    /// This is for Canon CR2.
    slices: Option<Vec<u32>>,
    cur_row: usize,
    prev_row: usize,
    mcu_row: Vec<Vec<Mcu>>,
    bit_reader: BitReader,
}

impl Default for LJpeg {
    fn default() -> LJpeg {
        LJpeg::new()
    }
}

impl LJpeg {
    pub fn new() -> LJpeg {
        LJpeg {
            slices: None,
            cur_row: 0,
            prev_row: 1,
            mcu_row: Vec::new(),
            bit_reader: BitReader::new(),
        }
    }

    pub fn set_slices(&mut self, slices: &[u32]) {
        let mut v = Vec::new();
        let n = slices[0] as usize;
        v.resize(n + 1, slices[1]);
        v[n] = slices[2];

        self.slices = Some(v);
    }

    /// Decompress the LJPEG stream into a tile.
    /// Pass `true` to `tiled` if it's an actual tiled file.
    pub fn decompress_buffer(&mut self, reader: &mut dyn ReadAndSeek, tiled: bool) -> Result<Tile> {
        let mut dc_info = DecompressInfo::default();

        self.read_file_header(&mut dc_info, reader)?;
        self.read_scan_header(&mut dc_info, reader)?;

        if dc_info.image_width == 0 || dc_info.image_height == 0 {
            return Err(Error::JpegFormat(format!(
                "LJPEG: incorrect dimensions {}x{}",
                dc_info.image_width, dc_info.image_height
            )));
        }
        if dc_info.num_components > 4 {
            return Err(Error::JpegFormat(format!(
                "LJPEG: unsupported number of components {}",
                dc_info.num_components
            )));
        }
        let bpc = dc_info.data_precision;
        let mut output: SlicedBuffer<ComponentType> = SlicedBuffer::new(
            dc_info.image_width as u32 * dc_info.num_components as u32,
            dc_info.image_height as u32,
            self.slices.clone().as_deref(),
        );
        output.reserve(
            dc_info.image_width as usize
                * dc_info.image_height as usize
                * dc_info.num_components as usize,
        );
        log::debug!(
            "dc width = {} dc height = {}",
            dc_info.image_width,
            dc_info.image_height
        );
        let width: u32 = if tiled {
            // Tiled seems to have the actual width.
            dc_info.image_width as u32
        } else {
            // Consistently the real width is the JPEG width * numComponent
            // On CR2 and untiled DNG.
            dc_info.image_width as u32 * dc_info.num_components as u32
        };
        // XXX RawData::set_slices?
        self.decoder_struct_init(&mut dc_info)?;
        self.huff_decoder_init(&mut dc_info)?;
        self.decode_image(&mut dc_info, reader, &mut output)?;

        Ok(Tile {
            height: dc_info.image_height as u32,
            width,
            u_height: dc_info.image_height as u32,
            u_width: width,
            bpc: bpc as u16,
            buf: output.into(),
        })
    }

    /// Decompress the LJPEG stream into a RawData.
    pub fn decompress(&mut self, reader: &mut dyn ReadAndSeek) -> Result<RawData> {
        let tile = self.decompress_buffer(reader, false)?;
        let mut rawdata = RawData::new16(
            tile.width,
            tile.height,
            tile.bpc,
            DataType::Raw,
            tile.buf,
            Pattern::default(),
        );
        let white: u32 = (1 << tile.bpc) - 1;
        rawdata.set_white(white as u16);
        Ok(rawdata)
    }

    fn read_file_header(
        &self,
        dc: &mut DecompressInfo,
        reader: &mut dyn ReadAndSeek,
    ) -> Result<()> {
        // Demand an SOI marker at the start of the file --- otherwise it's
        // probably not a JPEG file at all.
        let c = reader.read_u8()?;
        let c2 = reader.read_u8()?;
        if c != 0xff || c2 != M_SOI {
            return Err(Error::Decompression(format!(
                "LJPEG: Not a JPEG file. Marker is {c:x} {c2:x}"
            )));
        }
        dc.get_soi();
        // Process markers until SOF
        let c = dc.process_tables(reader)?;
        match c {
            M_SOF0 | M_SOF1 | M_SOF3 => dc.get_sof(reader)?,
            _ => {
                log::warn!("Unsupported SOF marker type 0x{:x}", c);
            }
        }
        Ok(())
    }

    fn read_scan_header(
        &self,
        dc: &mut DecompressInfo,
        reader: &mut dyn ReadAndSeek,
    ) -> Result<bool> {
        let c = dc.process_tables(reader)?;

        match c {
            M_SOS => {
                dc.get_sos(reader)?;
                Ok(true)
            }
            M_EOI => Ok(false),
            _ => {
                log::warn!("Unexpected marker Ox{:x}", c);
                Ok(false)
            }
        }
    }

    fn decoder_struct_init(&mut self, dc: &mut DecompressInfo) -> Result<()> {
        // Check sampling factor validity.
        for ci in 0..dc.num_components {
            let comp_ptr = &dc.comp_info[ci as usize];
            if comp_ptr.h_samp_factor != 1 || comp_ptr.v_samp_factor != 1 {
                return Err(Error::JpegFormat(
                    "LJPEG: Downsampling is not supported".into(),
                ));
            }
        }

        // Prepare array describing MCU composition
        if dc.comps_in_scan == 1 {
            dc.mcu_membership[0] = 0
        } else {
            if dc.comps_in_scan > 4 {
                return Err(Error::JpegFormat(
                    "LJPEG: Too many components for interleaved scan".into(),
                ));
            }
            for ci in 0..dc.comps_in_scan {
                dc.mcu_membership[ci as usize] = ci;
            }
        }

        // Initialize mucROW1 and mcuROW2 which buffer two rows of
        // pixels for predictor calculation.
        // XXX Turn this into a single buffer per row.
        // XXX Currently this statically uses 4 components even if
        // XXX our use case is 2.
        self.mcu_row.push(vec![]);
        self.mcu_row[0].resize(dc.image_width as usize, vec![0; 4]);
        self.mcu_row.push(vec![]);
        self.mcu_row[1].resize(dc.image_width as usize, vec![0; 4]);

        Ok(())
    }

    fn huff_decoder_init(&mut self, dc: &mut DecompressInfo) -> Result<()> {
        self.bit_reader.discard_bits(); // just reset it all.

        for ci in 0..dc.comps_in_scan {
            let compptr = &dc.cur_comp_info[ci as usize];
            if compptr.dc_tbl_no as usize >= dc.dc_huff_tbl_ptrs.len() {
                return Err(Error::JpegFormat("LJPEG: invalid dc_tbl_no".into()));
            }
            // Make sure requested tables are present
            if let Some(table) = dc.dc_huff_tbl_ptrs[compptr.dc_tbl_no as usize].as_mut() {
                // Compute derived values for Huffman tables.
                // We may do this more than once for same table, but it's not a
                // big deal
                table.fix()?;
            } else {
                log::error!("LJPEG: Use of undefined Huffman table");
                return Err(Error::Decompression(
                    "LJPEG: Use of undefined Huffman table".into(),
                ));
            }
        }

        // Initialize restart stuff
        dc.restart_in_rows = dc.restart_interval as u32 / dc.image_width as u32;
        dc.restart_rows_to_go = dc.restart_in_rows;
        dc.next_restart_num = 0;

        Ok(())
    }

    fn decode_image(
        &mut self,
        dc: &mut DecompressInfo,
        reader: &mut dyn ReadAndSeek,
        output: &mut SlicedBuffer<ComponentType>,
    ) -> Result<()> {
        let image_width = dc.image_width;
        let num_col = image_width;
        let num_row = dc.image_height;
        let comps_in_scan = dc.comps_in_scan;
        let pt = dc.pt;
        let psv = dc.ss;

        // Decode the first row of image. Output the row and
        // turn this row into a previous row for later predictor
        // calculation.
        self.decode_first_row(dc, reader)?;
        Self::pm_put_row(
            &self.mcu_row[self.cur_row],
            comps_in_scan,
            num_col,
            pt,
            output,
        );
        std::mem::swap(&mut self.cur_row, &mut self.prev_row);

        for _ in 1..num_row {
            // Account for restart interval, process restart marker if needed.
            if dc.restart_in_rows != 0 {
                if dc.restart_rows_to_go == 0 {
                    self.process_restart(dc, reader)?;

                    // Reset predictors at restart
                    self.decode_first_row(dc, reader)?;
                    Self::pm_put_row(
                        &self.mcu_row[self.cur_row],
                        comps_in_scan,
                        num_col,
                        pt,
                        output,
                    );
                    std::mem::swap(&mut self.cur_row, &mut self.prev_row);
                    continue;
                }
                dc.restart_rows_to_go -= 1;
            }

            // The upper neighbors are predictors for the first column.
            for cur_comp in 0..comps_in_scan as usize {
                let ci = dc.mcu_membership[cur_comp];
                let compptr = &dc.cur_comp_info[ci as usize];
                if let Some(ref dctbl) = &dc.dc_huff_tbl_ptrs[compptr.dc_tbl_no as usize] {
                    // Section F.2.2.1: decode the difference
                    let s = self.huff_decode(dctbl, reader)? as u8;
                    let d = if s != 0 {
                        extend(self.bit_reader.get_bits(reader, s)?, s)
                    } else {
                        0
                    };
                    self.mcu_row[self.cur_row][0][cur_comp] =
                        (d + self.mcu_row[self.prev_row][0][cur_comp] as i32) as u16;
                } else {
                    return Err(Error::JpegFormat("Huffman table is None".to_string()));
                }
            }

            // For the rest of the column on this row, predictor
            // calculations are base on PSV.
            for col in 1..num_col {
                for cur_comp in 0..comps_in_scan {
                    let ci = dc.mcu_membership[cur_comp as usize];
                    let compptr = &dc.cur_comp_info[ci as usize];
                    if compptr.dc_tbl_no as usize >= dc.dc_huff_tbl_ptrs.len() {
                        return Err(Error::JpegFormat("LJPEG: invalid dc_tbl_no".into()));
                    }
                    if let Some(ref dctbl) = &dc.dc_huff_tbl_ptrs[compptr.dc_tbl_no as usize] {
                        // Section F.2.2.1: decode the difference
                        let s = self.huff_decode(dctbl, reader)? as u8;
                        let d = if s != 0 {
                            extend(self.bit_reader.get_bits(reader, s)?, s)
                        } else {
                            0
                        };
                        let predictor = self.quick_predict(
                            col as i32,
                            cur_comp as i16,
                            &self.mcu_row[self.cur_row],
                            &self.mcu_row[self.prev_row],
                            psv,
                        );
                        self.mcu_row[self.cur_row][col as usize][cur_comp as usize] =
                            (d + predictor) as u16;
                    } else {
                        return Err(Error::JpegFormat("Huffman table is None".to_string()));
                    }
                }
            }
            Self::pm_put_row(
                &self.mcu_row[self.cur_row],
                comps_in_scan,
                num_col,
                pt,
                output,
            );
            std::mem::swap(&mut self.cur_row, &mut self.prev_row);
        }

        Ok(())
    }

    /// Check for a restart marker & resynchronize decoder.
    fn process_restart(
        &mut self,
        dc: &mut DecompressInfo,
        reader: &mut dyn ReadAndSeek,
    ) -> Result<()> {
        // Throw away any unused bits remaining in bit buffer
        let _nbytes = self.bit_reader.discard_bits();

        // Scan for next JPEG marker
        let mut c = 0;
        while c == 0 {
            // nbytes += 1;
            c = reader.read_u8()?;
            // skip any non-FF bytes
            while c != 0xff {
                // nbytes += 1;
                c = reader.read_u8()?;
            }

            c = reader.read_u8()?;
            // skip any duplicate FFs
            while c == 0xff {
                //  we don't increment nbytes here since extra FFs are legal
                c = reader.read_u8()?;
            }
        } // repeat if it was a stuffed FF/00

        if c != (M_RST0 + dc.next_restart_num) {
            // Uh-oh, the restart markers have been messed up too.
            // Just bail out.
            return Err(Error::JpegFormat(
                "LJPEG: Corrupt JPEG data. Aborting decoding...".into(),
            ));
        }

        // Update restart state
        dc.restart_rows_to_go = dc.restart_in_rows;
        dc.next_restart_num = (dc.next_restart_num + 1) & 7;

        Ok(())
    }

    /// Decode the first raster line of samples at the start of
    /// the scan and at the beginning of each restart interval.
    /// This includes modifying the component value so the real
    /// value, not the difference is returned.
    fn decode_first_row(
        &mut self,
        dc: &mut DecompressInfo,
        reader: &mut dyn ReadAndSeek,
    ) -> Result<()> {
        let pr = dc.data_precision;
        let pt = dc.pt;
        let comps_in_scan = dc.comps_in_scan;
        let num_col = dc.image_width;

        // the start of the scan or at the beginning of restart interval.
        for cur_comp in 0..comps_in_scan {
            let ci = dc.mcu_membership[cur_comp as usize];
            let compptr = &dc.cur_comp_info[ci as usize];
            if let Some(ref dctbl) = dc.dc_huff_tbl_ptrs[compptr.dc_tbl_no as usize] {
                // Section F.2.2.1: decode the difference
                let s = self.huff_decode(dctbl, reader)? as u8;
                let d = if s != 0 {
                    extend(self.bit_reader.get_bits(reader, s)?, s)
                } else {
                    0
                };

                // Add the predictor to the difference.
                let cur_row_buf = &mut self.mcu_row[self.cur_row];
                if pr < pt + 1 {
                    return Err(Error::JpegFormat("LJPEG: Invalid predictors.".into()));
                }
                cur_row_buf[0][cur_comp as usize] = (d + (1 << (pr - pt - 1))) as u16;
            } else {
                return Err(Error::JpegFormat("Huffman table is None".to_string()));
            }
        }

        // the rest of the first row
        for col in 1..num_col {
            for cur_comp in 0..comps_in_scan {
                let ci = dc.mcu_membership[cur_comp as usize];
                let compptr = &dc.cur_comp_info[ci as usize];
                if let Some(ref dctbl) = dc.dc_huff_tbl_ptrs[compptr.dc_tbl_no as usize] {
                    // Section F.2.2.1: decode the difference
                    let s = self.huff_decode(dctbl, reader)? as u8;
                    let d = if s != 0 {
                        extend(self.bit_reader.get_bits(reader, s)?, s)
                    } else {
                        0
                    };
                    // Add the predictor to the difference.
                    let cur_row_buf = &mut self.mcu_row[self.cur_row];
                    cur_row_buf[col as usize][cur_comp as usize] =
                        (d + cur_row_buf[col as usize - 1][cur_comp as usize] as i32) as u16;
                } else {
                    return Err(Error::JpegFormat("Huffman table is None".to_string()));
                }
            }
        }

        if dc.restart_in_rows != 0 {
            dc.restart_rows_to_go -= 1;
        }

        Ok(())
    }

    // Taken from Figure F.16: extract next coded symbol from
    // input stream.
    fn huff_decode(&mut self, htbl: &HuffmanTable, reader: &mut dyn ReadAndSeek) -> Result<u16> {
        let rv: u16;
        // If the huffman code is less than 8 bits, we can use the fast
        // table lookup to get its value.  It's more than 8 bits about
        // 3-4% of the time.
        let mut code = self.bit_reader.show_bits8(reader)? as i32;
        if htbl.numbits[code as usize] != 0 {
            self.bit_reader.flush_bits(htbl.numbits[code as usize]);
            rv = htbl.value[code as usize] as u16;
        } else {
            self.bit_reader.flush_bits(8);
            let mut l: usize = 8;
            while code > htbl.maxcode[l] {
                let temp = self.bit_reader.get_bit(reader)? as i32;
                code = (code << 1) | temp;
                l += 1;
            }

            // With garbage input we may reach the sentinel value l = 17.
            if l > 16 {
                // log::warn!("Corrupt JPEG data: bad huffman code")
                rv = 0;
            } else {
                rv = htbl.huffval[((htbl.valptr[l] as i32) + (code - htbl.mincode[l])) as usize]
                    as u16;
            }
        }

        Ok(rv)
    }

    fn quick_predict(
        &self,
        col: i32,
        cur_comp: i16,
        cur_row_buf: &[Mcu],
        prev_row_buf: &[Mcu],
        psv: u8,
    ) -> i32 {
        let left_col = col - 1;
        let upper = prev_row_buf[col as usize][cur_comp as usize] as i32;
        let left = cur_row_buf[left_col as usize][cur_comp as usize] as i32;
        let diag = prev_row_buf[left_col as usize][cur_comp as usize] as i32;

        match psv {
            0 => 0,
            1 => left,
            2 => upper,
            3 => diag,
            4 => left + upper - diag,
            5 => left + ((upper - diag) >> 1),
            6 => upper + ((left - diag) >> 1),
            7 => (left + upper) >> 1,
            _ => {
                log::warn!("Undefined PSV {}", psv);
                0
            }
        }
    }

    /// Output one row of pixels stored in RowBuf.
    fn pm_put_row(
        row_buf: &[Mcu],
        num_comp: u16,
        num_col: u16,
        pt: u8,
        output: &mut SlicedBuffer<ComponentType>,
    ) {
        for col in 0..num_col {
            output.extend(
                row_buf[col as usize][0..num_comp as usize]
                    .iter()
                    .map(|v| v << pt),
            );
        }
    }
}

/// Bit reader state
struct BitReader {
    bits_left: u8,
    buffer: u32,
}

impl BitReader {
    pub fn new() -> BitReader {
        BitReader {
            bits_left: 0,
            buffer: 0,
        }
    }

    /// Discard current bits and return the number of whole bytes that were
    /// left
    #[inline]
    fn discard_bits(&mut self) -> u8 {
        let nbytes = self.bits_left / 8;
        self.bits_left = 0;

        nbytes
    }

    #[inline]
    fn show_bits8(&mut self, reader: &mut dyn ReadAndSeek) -> Result<u16> {
        if self.bits_left < 8 {
            self.fill_bit_buffer(reader, 8)?;
        }

        Ok(((self.buffer >> (self.bits_left - 8)) & 0xff) as u16)
    }

    #[inline]
    fn flush_bits(&mut self, nbits: u8) {
        self.bits_left -= nbits;
    }

    #[inline]
    fn get_bits(&mut self, reader: &mut dyn ReadAndSeek, nbits: u8) -> Result<u16> {
        if nbits >= 17 {
            return Err(Error::Decompression(format!(
                "LJPEG: Tried to request {nbits} bits (max 16), JPEG is likely corrupt"
            )));
        }
        if self.bits_left < nbits {
            self.fill_bit_buffer(reader, nbits)?;
        }
        self.bits_left -= nbits;
        Ok((self.buffer >> self.bits_left) as u16 & BMASK[nbits as usize])
    }

    #[inline]
    fn get_bit(&mut self, reader: &mut dyn ReadAndSeek) -> Result<u16> {
        if self.bits_left == 0 {
            self.fill_bit_buffer(reader, 1)?;
        }

        self.bits_left -= 1;
        Ok(((self.buffer >> self.bits_left) & 1) as u16)
    }

    // Load up the bit buffer with at least nbits
    // Process any stuffed bytes at this time.
    fn fill_bit_buffer(&mut self, reader: &mut dyn ReadAndSeek, nbits: u8) -> Result<()> {
        while self.bits_left < MIN_GET_BITS {
            let mut c = reader.read_u8()?;
            // If it's 0xFF, check and discard stuffed zero byte
            if c == 0xff {
                let c2 = reader.read_u8()?;
                if c2 != 0 {
                    // Oops, it's actually a marker indicating end of
                    // compressed data.  Better put it back for use later.
                    reader.seek(SeekFrom::Current(-2))?;
                    // There should be enough bits still left in the data
                    // segment; if so, just break out of the while loop.
                    if self.bits_left > nbits {
                        break;
                    }
                    // Uh-oh.  Corrupted data: stuff zeroes into the data
                    // stream, since this sometimes occurs when we are on the
                    // last show_bits(8) during decoding of the Huffman
                    // segment.
                    c = 0;
                }
            }
            // OK, load c into getBuffer
            self.buffer = (self.buffer << 8) | c as u32;
            self.bits_left += 8;
        }

        Ok(())
    }
}

// One of the following structures is created for each huffman coding
// table.  We use the same structure for encoding and decoding, so there
// may be some extra fields for encoding that aren't used in the decoding
// and vice-versa.
struct HuffmanTable {
    // These two fields directly represent the contents of a JPEG DHT
    // marker
    bits: [u8; 17],
    huffval: [u8; 256],

    // The remaining fields are computed from the above to allow more
    // efficient coding and decoding.  These fields should be considered
    // private to the Huffman compression & decompression modules.
    ehufco: [u16; 256],
    ehufsi: [u8; 256],

    mincode: [i32; 17],
    maxcode: [i32; 18],
    valptr: [u16; 17],
    //
    numbits: [u8; 256],
    value: [u8; 256],
}

impl Default for HuffmanTable {
    fn default() -> HuffmanTable {
        HuffmanTable {
            bits: [0; 17],
            huffval: [0; 256],
            ehufco: [0; 256],
            ehufsi: [0; 256],
            mincode: [0; 17],
            maxcode: [0; 18],
            valptr: [0; 17],
            numbits: [0; 256],
            value: [0; 256],
        }
    }
}

impl HuffmanTable {
    const BIT_MASK: [u32; 32] = [
        0xffffffff, 0x7fffffff, 0x3fffffff, 0x1fffffff, 0x0fffffff, 0x07ffffff, 0x03ffffff,
        0x01ffffff, 0x00ffffff, 0x007fffff, 0x003fffff, 0x001fffff, 0x000fffff, 0x0007ffff,
        0x0003ffff, 0x0001ffff, 0x0000ffff, 0x00007fff, 0x00003fff, 0x00001fff, 0x00000fff,
        0x000007ff, 0x000003ff, 0x000001ff, 0x000000ff, 0x0000007f, 0x0000003f, 0x0000001f,
        0x0000000f, 0x00000007, 0x00000003, 0x00000001,
    ];

    pub(crate) fn fix(&mut self) -> Result<()> {
        let mut huffsize = [0_u8; 257];
        let mut huffcode = [0_u16; 257];

        // Figure C.1: make table of Huffman code length for each symbol
        // Note that this is in code-length order.
        let mut p = 0_usize;
        for l in 1..=16_u8 {
            for _ in 1..=self.bits[l as usize] {
                huffsize[p] = l;
                p += 1
            }
        }
        huffsize[p] = 0;
        let lastp = p;

        // Figure C.2: generate the codes themselves
        // Note that this is in code-length order.
        let mut code: u32 = 0; // is u16, but we must check for overflow.
        let mut si = huffsize[0];
        let mut p = 0;
        while huffsize[p] != 0 {
            while huffsize[p] == si {
                huffcode[p] = code as u16;
                p += 1;
                code += 1;
                if code > u16::MAX as u32 {
                    return Err(Error::JpegFormat("LJPEG: Huffman code overflow".into()));
                }
            }
            code <<= 1;
            si += 1;
        }

        // Figure C.3: generate encoding tables
        // These are code and size indexed by symbol value
        // Set any codeless symbols to have code length 0; this allows
        // EmitBits to detect any attempt to emit such symbols.
        self.ehufsi.fill(0);
        for p in 0..lastp {
            self.ehufco[self.huffval[p] as usize] = huffcode[p];
            self.ehufsi[self.huffval[p] as usize] = huffsize[p];
        }

        // Figure F.15: generate decoding tables
        let mut p: u16 = 0;
        for l in 1..=16_usize {
            if self.bits[l] != 0 {
                self.valptr[l] = p;
                self.mincode[l] = huffcode[p as usize] as i32;
                p += self.bits[l] as u16;
                if p > 256 {
                    return Err(Error::JpegFormat("LJPEG: huffcode index overflow".into()));
                }
                self.maxcode[l] = huffcode[p as usize - 1] as i32;
            } else {
                self.maxcode[l] = -1;
            }
        }

        // We put in this value to ensure HuffDecode terminates.
        self.maxcode[17] = 0xfffff;

        // Build the numbits, value lookup tables.
        // These table allow us to gather 8 bits from the bits stream,
        // and immediately lookup the size and value of the huffman codes.
        // If size is zero, it means that more than 8 bits are in the huffman
        // code (this happens about 3-4% of the time).
        self.numbits.fill(0);
        for p in 0..lastp {
            let size = huffsize[p];
            if size <= 8 {
                let value = self.huffval[p];
                code = huffcode[p] as u32;
                let ll: u32 = code << (8 - size);
                let ul = if size < 8 {
                    ll | Self::BIT_MASK[24 + size as usize]
                } else {
                    ll
                };
                if ll as usize >= self.numbits.len() {
                    return Err(Error::JpegFormat(
                        "LJPEG: invalid value in Huffman table".into(),
                    ));
                }
                for i in ll..=ul {
                    self.numbits[i as usize] = size;
                    self.value[i as usize] = value;
                }
            }
        }

        Ok(())
    }
}

// The following structure stores basic information about one component.
#[derive(Clone, Default)]
struct JpegComponentInfo {
    // These values are fixed over the whole image.
    // They are read from the SOF marker.

    // identifier for this component (0..255)
    component_id: u16,
    // its index in SOF or cPtr->compInfo[]
    component_index: u16,

    // Downsampling is not normally used in lossless JPEG, although
    // it is permitted by the JPEG standard (DIS). We set all sampling
    // factors to 1 in this program.

    // horizontal sampling factor
    h_samp_factor: u16,
    // vertical sampling factor
    v_samp_factor: u16,

    // Huffman table selector (0..3). The value may vary
    // between scans. It is read from the SOS marker.
    dc_tbl_no: u16,
}

#[derive(Default)]
struct DecompressInfo {
    // Image width, height, and image data precision (bits/sample)
    // These fields are set by ReadFileHeader or ReadScanHeader
    image_width: u16,
    image_height: u16,
    data_precision: u8,

    // compInfo[i] describes component that appears i'th in SOF
    // numComponents is the # of color components in JPEG image.
    // XXX since it want to share pointers wih cur_comp_info...
    comp_info: Vec<JpegComponentInfo>,
    num_components: u8,

    // *curCompInfo[i] describes component that appears i'th in SOS.
    // comps_in_scan is the # of color components in current scan.
    cur_comp_info: [JpegComponentInfo; 4],
    comps_in_scan: u16,

    // MCUmembership[i] indexes the i'th component of MCU into the
    // cur_comp_info array.
    mcu_membership: [u16; 10],

    // ptrs to Huffman coding tables, or NULL if not defined
    dc_huff_tbl_ptrs: [Option<HuffmanTable>; 4],

    // prediction seletion value (PSV) and point transform parameter (Pt)
    ss: u8,
    pt: u8,

    // In lossless JPEG, restart interval shall be an integer
    // multiple of the number of MCU in a MCU row.

    /* MCUs per restart interval, 0 = no restart */
    restart_interval: u16,
    /*if > 0, MCU rows per restart interval; 0 = no restart*/
    restart_in_rows: u32,

    // these fields are private data for the entropy decoder

    // MCUs rows left in this restart interval
    restart_rows_to_go: u32,
    // # of next RSTn marker (0..7)
    next_restart_num: u8,
}

impl DecompressInfo {
    fn get_soi(&mut self) {
        self.restart_interval = 0;
    }

    fn process_tables(&mut self, reader: &mut dyn ReadAndSeek) -> Result<u8> {
        loop {
            let c = self.next_marker(reader)?;
            match c {
                M_SOF0 | M_SOF1 | M_SOF2 | M_SOF3 | M_SOF5 | M_SOF6 | M_SOF7 | M_JPG | M_SOF9
                | M_SOF10 | M_SOF11 | M_SOF13 | M_SOF14 | M_SOF15 | M_SOI | M_EOI | M_SOS => {
                    return Ok(c)
                }
                M_DHT => self.get_dht(reader)?,
                M_DQT => log::warn!("Not a lossless JPEG file."),
                M_DRI => self.get_dri(reader)?,
                M_APP0 => self.get_app0(reader)?,
                // these are all parameterless
                M_RST0 | M_RST1 | M_RST2 | M_RST3 | M_RST4 | M_RST5 | M_RST6 | M_RST7 | M_TEM => {
                    log::warn!("Unexpected marker: 0x{:x}", c)
                }
                _ => self.skip_variable(reader)?,
            }
        }
    }

    fn get_sof(&mut self, reader: &mut dyn ReadAndSeek) -> Result<()> {
        let length = reader.read_u16::<BigEndian>()?;
        self.data_precision = reader.read_u8()?;
        self.image_height = reader.read_u16::<BigEndian>()?;
        self.image_width = reader.read_u16::<BigEndian>()?;
        self.num_components = reader.read_u8()?;

        // We don't support files in which the image height is initially
        // specified as 0 and is later redefined by DNL.  As long as we
        // have to check that, might as well have a general sanity check.
        if self.image_height == 0 || self.image_width == 0 || self.num_components == 0 {
            return Err(Error::JpegFormat(
                "LJPEG: Empty JPEG image (DNL not supported)".into(),
            ));
        }

        if self.data_precision < MIN_PRECISION_BITS || self.data_precision > MAX_PRECISION_BITS {
            return Err(Error::JpegFormat(
                "LJPEG: Unsupported JPEG data precision".into(),
            ));
        }

        if length != (self.num_components as u16 * 3 + 8) {
            return Err(Error::JpegFormat("LJPEG: Bogus SOF length".into()));
        }

        self.comp_info
            .resize(self.num_components as usize, JpegComponentInfo::default());

        for ci in 0..self.num_components {
            let compptr = &mut self.comp_info[ci as usize];
            compptr.component_index = ci as u16;
            compptr.component_id = reader.read_u8()? as u16;
            let c = reader.read_u8()?;
            compptr.h_samp_factor = ((c >> 4) & 15) as u16;
            compptr.v_samp_factor = (c & 15) as u16;
            reader.read_u8()?; // skip Tq
        }

        Ok(())
    }

    fn get_dht(&mut self, reader: &mut dyn ReadAndSeek) -> Result<()> {
        // length as a signed i32 is safer
        let mut length = reader.read_u16::<BigEndian>()? as i32 - 2;
        while length > 0 {
            let index = reader.read_u8()?;

            if index >= 4 {
                return Err(Error::JpegFormat(format!("LJPEG: Bogus DHT index {index}")));
            }

            if self.dc_huff_tbl_ptrs[index as usize].is_none() {
                self.dc_huff_tbl_ptrs[index as usize] = Some(HuffmanTable::default());
            }
            let mut htblptr = self.dc_huff_tbl_ptrs[index as usize].as_mut().unwrap();
            htblptr.bits[0] = 0;
            let mut count = 0_u16;
            for i in 1..=16 {
                let b = reader.read_u8()?;
                htblptr.bits[i] = b;
                count += b as u16;
            }

            if count > 256 {
                return Err(Error::JpegFormat("LJPEG: Bogus DHT counts".into()));
            }
            for i in 0_usize..count as usize {
                htblptr.huffval[i] = reader.read_u8()?;
            }
            length -= 1 + 16 + count as i32;
        }
        Ok(())
    }

    fn skip_variable(&self, reader: &mut dyn ReadAndSeek) -> Result<()> {
        let length = reader.read_u16::<BigEndian>()?;
        if length < 2 {
            return Err(Error::JpegFormat("LJPEG: invalid variable length".into()));
        }
        let length = length - 2;
        reader.seek(SeekFrom::Current(length as i64))?;

        Ok(())
    }

    fn next_marker(&self, reader: &mut dyn ReadAndSeek) -> Result<u8> {
        let mut c;

        loop {
            // skip any non-FF bytes
            c = reader.read_u8()?;
            while c != 0xff {
                c = reader.read_u8()?;
            }

            c = reader.read_u8()?;
            while c == 0xff {
                c = reader.read_u8()?;
            }
            if c != 0 {
                break;
            }
        }

        Ok(c)
    }

    fn get_dri(&mut self, reader: &mut dyn ReadAndSeek) -> Result<()> {
        if reader.read_u16::<BigEndian>()? != 4 {
            return Err(Error::JpegFormat("LJPEG: Invalid DRI length.".into()));
        }
        self.restart_interval = reader.read_u16::<BigEndian>()?;
        Ok(())
    }

    /// Process APP0 marker.
    fn get_app0(&self, reader: &mut dyn ReadAndSeek) -> Result<()> {
        let length = reader.read_u16::<BigEndian>()?;
        if length < 2 {
            return Err(Error::JpegFormat("LJPEG: Invalid APP0 length.".into()));
        }
        let length = length - 2;
        reader.seek(SeekFrom::Current(length as i64))?;
        Ok(())
    }

    fn get_sos(&mut self, reader: &mut dyn ReadAndSeek) -> Result<()> {
        let mut length = reader.read_u16::<BigEndian>()?;
        if length < 3 {
            return Err(Error::JpegFormat("LJPEG: invalid SOS length".into()));
        }

        let n = reader.read_u8()?;
        self.comps_in_scan = n as u16;
        length -= 3;

        if length != (n as u16 * 2 + 3) || !(1..=4).contains(&n) {
            return Err(Error::JpegFormat(format!("LJPEG: Bogus SOS length {n}")));
        }

        for i in 0..n {
            let cc = reader.read_u8()? as u16;
            let c = reader.read_u8()?;
            length -= 2;

            let mut ci = 0;
            while ci < self.num_components {
                if cc == self.comp_info[ci as usize].component_id {
                    break;
                }
                ci += 1;
            }
            if ci >= self.num_components {
                return Err(Error::JpegFormat(
                    "LJPEG: Invalid component number in SOS".into(),
                ));
            }

            let compptr = &mut self.comp_info[ci as usize];
            compptr.dc_tbl_no = ((c >> 4) & 15) as u16;
            // XXX do we want to clone or to share?
            self.cur_comp_info[i as usize] = compptr.clone();
        }

        self.ss = reader.read_u8()?;
        reader.read_u8()?;
        let c = reader.read_u8()?;
        self.pt = c & 0x0f;

        Ok(())
    }
}

#[inline]
/// F.2.2.1 Code and table for Figure F.12: extend sign bit
fn extend(x: u16, s: u8) -> i32 {
    let vt = 1 << (s as u16 - 1);

    if x < vt {
        x as i32 + (-1 << s as i16) + 1
    } else {
        x as i32
    }
}

#[cfg(test)]
mod test {
    use super::LJpeg;
    use super::{BitReader, BITS_PER_LONG, MIN_GET_BITS};

    #[test]
    fn test_bit_reader() {
        // Note: don't use 0b1111_1111 for a byte as fill_bit_buffer
        // will use it to mark the end of the stream
        let bits = vec![
            0b1010_1010,
            0b0101_0101,
            0b1101_1011,
            0b0011_0011,
            0b1010_1010,
            0b0101_0101,
            0b1101_1011,
            0b0011_0011,
            0b1010_1010,
            0b0101_0101,
            0b1101_1011,
            0b0011_0011,
            0b1010_1010,
            0b0101_0101,
            0b1101_1011,
            0b0011_0011,
        ];

        let mut io = std::io::Cursor::new(bits);
        let mut br = BitReader::new();

        assert_eq!(BITS_PER_LONG, 32);
        assert_eq!(MIN_GET_BITS, 25);

        assert_eq!(br.buffer, 0);
        assert_eq!(br.bits_left, 0);
        assert_eq!(br.discard_bits(), 0);
        assert_eq!(br.buffer, 0);
        assert_eq!(br.bits_left, 0);

        assert!(matches!(br.show_bits8(&mut io), Ok(0b1010_1010)));
        assert_eq!(br.bits_left, 32);
        assert_eq!(br.buffer, 0b1010_1010_0101_0101_1101_1011_0011_0011);
        assert!(matches!(br.show_bits8(&mut io), Ok(0b1010_1010)));

        assert_eq!(br.discard_bits(), 4);
        // this doesn't clear the buffer
        assert_eq!(br.bits_left, 0);

        assert!(matches!(br.fill_bit_buffer(&mut io, 8), Ok(())));
        assert_eq!(br.bits_left, 32);
        assert_eq!(br.buffer, 0b1010_1010_0101_0101_1101_1011_0011_0011);
        assert!(matches!(br.show_bits8(&mut io), Ok(0b1010_1010)));

        assert!(matches!(br.get_bits(&mut io, 8), Ok(0b1010_1010)));
        assert_eq!(br.bits_left, 24);
        assert_eq!(br.buffer, 0b1010_1010_0101_0101_1101_1011_0011_0011);

        assert!(matches!(br.get_bit(&mut io), Ok(0)));
        assert_eq!(br.bits_left, 23);
        assert!(matches!(br.get_bit(&mut io), Ok(1)));
        assert_eq!(br.bits_left, 22);

        // XXX test fill_bit_buffer encountering 0xff
    }

    #[test]
    fn test_jpeg() {
        let mut decompressor = LJpeg::new();

        let io = std::fs::File::open("test/ljpegtest1.jpg");
        assert!(io.is_ok());
        let io = io.unwrap();
        let mut buffered = std::io::BufReader::new(io);
        let rawdata = decompressor.decompress(&mut buffered);

        assert!(rawdata.is_ok());
        let rawdata = rawdata.unwrap();

        fn raw_checksum(buf: &[u8]) -> u16 {
            // This is the same algorithm as used in the C++ implementation
            let crc = crc::Crc::<u16>::new(&crc::CRC_16_IBM_3740);
            let mut digest = crc.digest();
            digest.update(buf);

            digest.finalize()
        }

        let buf = rawdata.data16_as_u8();
        assert!(buf.is_some());
        let buf = buf.unwrap();
        let crc = raw_checksum(buf);

        assert_eq!(crc, 0x20cc);
    }
}
