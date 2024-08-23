// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - nikon.rs
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

//! Nikon specific code.

mod diffiterator;
mod huffman;
mod matrices;

use std::collections::HashMap;
use std::io::{Seek, SeekFrom};
use std::rc::Rc;

use byteorder::{BigEndian, ByteOrder, LittleEndian, ReadBytesExt};
use once_cell::unsync::OnceCell;

use crate::bitmap::Bitmap;
use crate::container::RawContainer;
use crate::decompress;
use crate::io::Viewer;
use crate::rawfile::{RawFileHandleType, ThumbnailStorage};
use crate::tiff;
use crate::tiff::exif;
use crate::tiff::{Dir, Ifd};
use crate::utils;
use crate::{
    DataType, Dump, Error, RawFile, RawFileHandle, RawFileImpl, RawImage, Result, Type, TypeId,
};

use diffiterator::{CfaIterator, DiffIterator};
use matrices::MATRICES;

#[macro_export]
macro_rules! nikon {
    ($id:expr, $model:ident) => {
        (
            $id,
            TypeId(
                $crate::camera_ids::vendor::NIKON,
                $crate::camera_ids::nikon::$model,
            ),
        )
    };
    ($model:ident) => {
        TypeId(
            $crate::camera_ids::vendor::NIKON,
            $crate::camera_ids::nikon::$model,
        )
    };
}

/// Nikon2 MakerNote tag names
pub use tiff::exif::generated::MNOTE_NIKON2_TAG_NAMES as MNOTE_TAG_NAMES_2;
/// Nikon1 MakerNote tag names
pub use tiff::exif::generated::MNOTE_NIKON_TAG_NAMES as MNOTE_TAG_NAMES;

lazy_static::lazy_static! {
    static ref MAKE_TO_ID_MAP: tiff::MakeToIdMap = HashMap::from([
        nikon!("NIKON D1 ", D1),
        nikon!("NIKON D100 ", D100),
        nikon!("NIKON D1H", D1H),
        nikon!("NIKON D1X", D1X),
        nikon!("NIKON D200", D200),
        nikon!("NIKON D2H", D2H),
        nikon!("NIKON D2Hs", D2HS),
        nikon!("NIKON D2X", D2X),
        nikon!("NIKON D2Xs", D2XS ),
        nikon!("NIKON D3", D3),
        nikon!("NIKON D3S", D3S),
        nikon!("NIKON D3X", D3X),
        nikon!("NIKON D300", D300),
        nikon!("NIKON D300S", D300S),
        nikon!("NIKON D3000", D3000),
        nikon!("NIKON D3100", D3100),
        nikon!("NIKON D3200", D3200),
        nikon!("NIKON D3300", D3300),
        nikon!("NIKON D3400", D3400),
        nikon!("NIKON D3500", D3500),
        nikon!("NIKON D4", D4),
        nikon!("NIKON D4S", D4S),
        nikon!("NIKON D40", D40),
        nikon!("NIKON D40X", D40X),
        nikon!("NIKON D5", D5),
        nikon!("NIKON D50", D50),
        nikon!("NIKON D500", D500),
        nikon!("NIKON D5000", D5000),
        nikon!("NIKON D5100", D5100),
        nikon!("NIKON D5200", D5200),
        nikon!("NIKON D5300", D5300),
        nikon!("NIKON D5500", D5500),
        nikon!("NIKON D5600", D5600),
        nikon!("NIKON D6", D6),
        nikon!("NIKON D60",   D60),
        nikon!("NIKON D600", D600),
        nikon!("NIKON D610", D610),
        nikon!("NIKON D70", D70),
        nikon!("NIKON D70s", D70S),
        nikon!("NIKON D700", D700),
        nikon!("NIKON D7000", D7000),
        nikon!("NIKON D7100", D7100),
        nikon!("NIKON D7200", D7200),
        nikon!("NIKON D750", D750),
        nikon!("NIKON D7500", D7500),
        nikon!("NIKON D780", D780),
        nikon!("NIKON D80", D80),
        nikon!("NIKON D800", D800),
        nikon!("NIKON D800E", D800E),
        nikon!("NIKON D810", D810),
        nikon!("NIKON D850", D850),
        nikon!("NIKON D90", D90),
        nikon!("NIKON Df", DF),
        nikon!("NIKON Z 30", Z30),
        nikon!("NIKON Z 6", Z6),
        nikon!("NIKON Z 6_2", Z6_2),
        nikon!("NIKON Z6_3", Z6_3),
        nikon!("NIKON Z 7", Z7),
        nikon!("NIKON Z 7_2", Z7_2),
        nikon!("NIKON Z 50", Z50),
        nikon!("NIKON Z 5", Z5),
        nikon!("NIKON Z 8", Z8),
        nikon!("NIKON Z 9", Z9),
        nikon!("NIKON Z fc", ZFC),
        nikon!("NIKON Z f", ZF),
        nikon!("E5400", E5400),
        nikon!("E5700", E5700),
        nikon!("E8400", E8400),
        nikon!("E8800", E8800),
        nikon!("COOLPIX B700", COOLPIX_B700),
        nikon!("COOLPIX P330", COOLPIX_P330),
        nikon!("COOLPIX P340", COOLPIX_P340),
        nikon!("COOLPIX P950", COOLPIX_P950),
        nikon!("COOLPIX P1000", COOLPIX_P1000),
        nikon!("COOLPIX P6000", COOLPIX_P6000),
        nikon!("COOLPIX P7000", COOLPIX_P7000),
        nikon!("COOLPIX P7100", COOLPIX_P7100),
        nikon!("COOLPIX P7700", COOLPIX_P7700),
        nikon!("COOLPIX P7800", COOLPIX_P7800),
        nikon!("COOLPIX A", COOLPIX_A),
        nikon!("COOLPIX A1000", COOLPIX_A1000),
        nikon!("NIKON 1 J1", NIKON1_J1),
        nikon!("NIKON 1 J2", NIKON1_J2),
        nikon!("NIKON 1 J3", NIKON1_J3),
        nikon!("NIKON 1 J4", NIKON1_J4),
        nikon!("NIKON 1 J5", NIKON1_J5),
        nikon!("NIKON 1 V1", NIKON1_V1),
        nikon!("NIKON 1 V2", NIKON1_V2),
        nikon!("NIKON 1 V3", NIKON1_V3),
        nikon!("NIKON 1 S1", NIKON1_S1),
        nikon!("NIKON 1 S2", NIKON1_S2),
        nikon!("NIKON 1 AW1", NIKON1_AW1),
    ]);
}

struct CompressionInfo {
    vpred: [[u16; 2]; 2],
    curve: Vec<u16>,
    huffman: Option<&'static [huffman::HuffmanNode]>,
}

impl CompressionInfo {
    fn new() -> Self {
        Self {
            vpred: [[0; 2]; 2],
            curve: vec![0; 0x8000],
            huffman: None,
        }
    }
}

#[derive(Debug)]
pub(crate) struct NefFile {
    reader: Rc<Viewer>,
    type_id: OnceCell<TypeId>,
    container: OnceCell<tiff::Container>,
    thumbnails: OnceCell<ThumbnailStorage>,
    probe: Option<crate::Probe>,
}

impl NefFile {
    pub(crate) fn factory(reader: Rc<Viewer>) -> RawFileHandle {
        RawFileHandleType::new(NefFile {
            reader,
            type_id: OnceCell::new(),
            container: OnceCell::new(),
            thumbnails: OnceCell::new(),
            probe: None,
        })
    }

    fn is_nrw(&self) -> bool {
        // XXX cache?
        self.ifd(tiff::IfdType::MakerNote)
            .and_then(|mnote| {
                mnote
                    .value::<String>(exif::MNOTE_NIKON_QUALITY)
                    .map(|value| value.trim() == "NRW")
                    .map(|value| {
                        if value {
                            probe!(self.probe, "nef.is_nrw", "true");
                        }
                        value
                    })
            })
            .unwrap_or(false)
    }

    /// The Raw file is from a D100.
    fn is_d100(&self) -> bool {
        let is_d100 = self.type_id() == nikon!(D100);
        probe!(self.probe, "nef.is_d100", is_d100);
        is_d100
    }

    /// Unpack Nikon.
    fn unpack_nikon(&self, rawdata: RawImage) -> Result<RawImage> {
        let mut width = rawdata.width();
        if self.is_d100() {
            width += 6;
        }
        probe!(self.probe, "nef.nikon_packed_raw", "true");
        let height = rawdata.height();
        let bpc = rawdata.bpc();
        let block_size: usize = match bpc {
            12 => ((width / 2 * 3) + width / 10) as usize,
            _ => {
                log::warn!("Invalid BPC {}", bpc);
                return Err(Error::InvalidFormat);
            }
        };
        log::debug!("block_size {} width {} ", block_size, width);

        let data = rawdata.data8().ok_or(Error::NotFound)?;

        let out_size = width as usize * height as usize;
        let mut out_data = Vec::with_capacity(out_size);
        let mut fetched = 0_usize;
        let mut written = 0_usize;

        let byte_len = std::cmp::min(data.len(), block_size * height as usize);
        while fetched < byte_len {
            let block = &data[fetched..fetched + block_size];
            fetched += block.len();

            written +=
                decompress::unpack_be12to16(block, &mut out_data, tiff::Compression::NikonPack)?;
        }
        log::debug!("Unpacked {} pixels", written);

        let mut rawdata = rawdata.replace_data(out_data);
        rawdata.set_data_type(DataType::Raw);

        Ok(rawdata)
    }

    fn get_compression_curve(&self, rawdata: &mut RawImage) -> Result<CompressionInfo> {
        self.ifd(tiff::IfdType::MakerNote)
            .ok_or_else(|| {
                log::error!("No MakerNote");
                Error::NotFound
            })
            .and_then(|mnote| {
                let curve_entry =
                    mnote
                        .entry(exif::MNOTE_NIKON_NEFDECODETABLE2)
                        .ok_or_else(|| {
                            log::error!("DecodeTable2 not found");
                            Error::NotFound
                        })?;
                let pos = curve_entry.offset().ok_or(Error::NotFound)? + mnote.mnote_offset;
                let bpc = rawdata.bpc();

                let container = self.container.get().unwrap();
                let mut view = container.borrow_view_mut();
                view.seek(SeekFrom::Start(pos as u64))?;
                let header0 = view.read_u8()?;
                let header1 = view.read_u8()?;

                probe!(self.probe, "nef.compression_curve", "true");

                probe!(
                    self.probe,
                    "nef.compression_curve.header0",
                    &format!("0x{header0:x}")
                );
                probe!(
                    self.probe,
                    "nef.compression_curve.header1",
                    &format!("0x{header1:x}")
                );
                if header0 == 0x49 {
                    // some interesting stuff at 2110
                    // XXX we need to implement this.
                    log::warn!("NEF: header0 is 0x49 - case not yet handled.");
                    view.seek(SeekFrom::Current(2110))?;
                }

                let mut curve = CompressionInfo::new();
                for i in 0..2 {
                    for j in 0..2 {
                        curve.vpred[i][j] = view.read_endian_u16(container.endian())?;
                    }
                }

                let mut header_ok = false;
                // header0 == 0x44 || 0x49 -> lossy
                // header0 == 0x46 -> lossless
                if header0 == 0x44 || header0 == 0x49 {
                    if bpc == 12 {
                        probe!(self.probe, "nef.compression_curve.huffman", "lossy_12");
                        curve.huffman = Some(&diffiterator::LOSSY_12BIT);
                        log::debug!("12 bits lossy {}", bpc);
                        header_ok = true;
                    } else if bpc == 14 {
                        probe!(self.probe, "nef.compression_curve.huffman", "lossy_14");
                        curve.huffman = Some(&diffiterator::LOSSY_14BIT);
                        log::debug!("14 bits lossy {}", bpc);
                        header_ok = true;
                    }
                } else if header0 == 0x46 {
                    if bpc == 14 {
                        probe!(self.probe, "nef.compression_curve.huffman", "lossless_14");
                        curve.huffman = Some(&diffiterator::LOSSLESS_14BIT);
                        log::debug!("14 bits lossless");
                        header_ok = true;
                    } else if bpc == 12 {
                        probe!(self.probe, "nef.compression_curve.huffman", "lossless_12");
                        // curve.huffman = Some(&diffiterator::LOSSLESS_12BIT);
                        log::debug!("12 bits lossless");
                        log::error!("12 bits lossless isn't yet supported");
                        // header_ok = true;
                        return Err(Error::NotSupported);
                    }
                }
                if !header_ok {
                    log::error!("Wrong header, found {}-{}", header0, header1);
                    return Err(Error::FormatError);
                }

                let nelems = view.read_endian_u16(container.endian()).unwrap_or(0);
                log::debug!("Num elems {}", nelems);

                let mut ceiling: u16 = 1 << bpc & 0x7fff;
                let mut step = 0_usize;
                if nelems > 1 {
                    step = (ceiling / (nelems - 1)) as usize;
                }
                log::debug!("ceiling {}, step {}", ceiling, step);

                if header0 == 0x44 && header1 == 0x20 && step > 0 {
                    for i in 0..nelems {
                        let value = view.read_endian_u16(container.endian())?;
                        curve.curve[i as usize * step] = value;
                    }
                    for i in 0..ceiling as usize {
                        curve.curve[i] = ((curve.curve[i - i % step] as usize * (step - i % step)
                            + curve.curve[i - i % step + step] as usize * (i % step))
                            / step) as u16;
                    }
                    probe!(self.probe, "nef.compression_curve.computed", "true");
                    // split flag is at offset 562.
                    // XXX
                } else if header0 != 0x46 && nelems <= 0x4001 {
                    let num_read =
                        container.read_u16_array(&mut view, &mut curve.curve, nelems as usize)?;
                    // XXX It's likly not possible for the short read to appear.
                    // As an error might be thrown first.
                    if num_read < nelems as usize {
                        log::error!("NEF short read of {} elems, expected {}", num_read, nelems);
                        return Err(Error::UnexpectedEOF);
                    }
                    ceiling = nelems;
                    probe!(self.probe, "nef.compression_curve.direct", "true");
                }

                let black = curve.curve[0];
                let white = curve.curve[ceiling as usize - 1];
                for i in ceiling..0x8000 {
                    // XXX there is a more rusty way to do that
                    curve.curve[i as usize] = white;
                }

                rawdata.set_whites([white; 4]);
                rawdata.set_blacks([black; 4]);

                Ok(curve)
            })
    }

    fn decompress_nikon_quantized(&self, mut rawdata: RawImage) -> Result<RawImage> {
        let new_data = self
            .get_compression_curve(&mut rawdata)
            .map_err(|err| {
                log::error!("Get compression curve failed {}", err);
                err
            })
            .and_then(|curve| {
                probe!(self.probe, "nef.compress_quantized", "true");
                let rows = rawdata.height() as usize;
                let raw_columns = rawdata.width() as usize;
                // XXX not always true
                let columns = raw_columns - 1;

                let data8 = rawdata.data8();
                if data8.is_none() {
                    return Err(Error::FormatError);
                }
                let diffs =
                    DiffIterator::new(curve.huffman.unwrap(), rawdata.data8().as_ref().unwrap());
                let mut iter = CfaIterator::new(diffs, raw_columns, curve.vpred);

                // Using uninit_vec! here is slower.
                let mut new_data = vec![0; rows * columns];
                for i in 0..rows {
                    for j in 0..raw_columns {
                        let t = iter.get().map_err(|err| {
                            log::error!("Error get");
                            err
                        })?;
                        if j < columns {
                            let shift: u16 = 16 - rawdata.bpc();
                            new_data[i * columns + j] = curve.curve[t as usize & 0x3fff] << shift;
                        }
                    }
                }
                rawdata.set_width(columns as u32);
                rawdata.set_whites([(1 << rawdata.bpc()) - 1; 4]);
                Ok(new_data)
            });
        match new_data {
            // This is not supported yet, just return the compressed data.
            Err(Error::NotSupported) | Err(Error::NotFound) => return Ok(rawdata),
            Err(e) => return Err(e),
            _ => {}
        }

        new_data.map(|new_data| {
            let mut rawdata = rawdata.replace_data(new_data);
            rawdata.set_data_type(DataType::Raw);
            rawdata
        })
    }

    fn encrypted_white_balance(&self, mnote: &Dir, entry: &tiff::Entry) -> Option<[f64; 3]> {
        probe!(self.probe, "nef.wb.encrypted", "true");
        let data = entry.data();
        let version = &data[0..4];
        let endian = mnote.endian();
        probe!(
            self.probe,
            "nef.wb.encrypted.version",
            &String::from_utf8_lossy(version)
        );
        if version == b"0100" && data.len() >= 80 {
            let r = endian.read_u16(&data[72..]) as f64;
            let b = endian.read_u16(&data[74..]) as f64;
            let g = endian.read_u16(&data[76..]) as f64;
            return Some([g / r, 1.0, g / b]);
        } else if version == b"0103" && data.len() >= 26 {
            let r = endian.read_u16(&data[20..]) as f64;
            let g = endian.read_u16(&data[22..]) as f64;
            let b = endian.read_u16(&data[24..]) as f64;
            return Some([g / r, 1.0, g / b]);
        }
        log::error!("Encrypted white balance is not supported yet. {version:?}");
        None
    }

    /// Extract the while balance mostly from NRW files.
    fn nrw_white_balance(&self, entry: &tiff::Entry) -> Option<[f64; 3]> {
        let data = entry.data();
        if data.len() == 2560 {
            probe!(self.probe, "nef.wb.nrw2560", "true");
            let data = &data[1248..];
            let r = BigEndian::read_u16(data) as f64;
            let b = BigEndian::read_u16(&data[2..]) as f64;
            Some([256.0 / r, 1.0, 256.0 / b])
        } else if &data[0..4] == b"NRW " {
            probe!(self.probe, "nef.wb.nrw", "true");
            let offset = if &data[4..8] != b"0100" && entry.count > 72 {
                56
            } else if entry.count > 1572 {
                1556
            } else {
                log::error!("ColorBalanceA format unknown");
                return None;
            };
            probe!(self.probe, "nef.wb.nrw.offset", offset);
            let data = &data[offset..];
            let r = (LittleEndian::read_u32(data) << 2) as f64;
            let g =
                (LittleEndian::read_u32(&data[4..]) + LittleEndian::read_u32(&data[8..])) as f64;
            let b = (LittleEndian::read_u32(&data[12..]) << 2) as f64;
            Some([g / r, 1.0, b / r])
        } else {
            log::error!("ColorBalanceA format unknown");
            None
        }
    }

    /// Extract the while balance
    fn white_balance(&self) -> Option<[f64; 3]> {
        self.maker_note_ifd().and_then(|mnote| {
            if let Some(entry) = mnote.entry(exif::MNOTE_NIKON_WB_RB_LEVELS) {
                // Some NRW have this tag, so take it before
                // `ColorBalanceA` (see [`nrw_white_balance`])
                if entry.count >= 4 {
                    probe!(self.probe, "nef.wb.nikon", "true");
                    entry.float_value_array(mnote.endian()).map(|v| {
                        let mut g = v[2];
                        if g <= 0.0 {
                            g = 1.0;
                        }
                        [1.0 / v[0], g, 1.0 / v[1]]
                    })
                } else {
                    probe!(self.probe, "nef.wb.nikon", "false");
                    None
                }
            } else if let Some(entry) = mnote.entry(exif::MNOTE_NIKON_COLOR_BALANCE) {
                self.encrypted_white_balance(mnote, entry)
            } else if let Some(entry) = mnote.entry(exif::MNOTE_NIKON_COLOR_BALANCE_A) {
                self.nrw_white_balance(entry)
            } else {
                None
            }
        })
    }
}

impl RawFileImpl for NefFile {
    #[cfg(feature = "probe")]
    probe_imp!();

    fn identify_id(&self) -> TypeId {
        *self.type_id.get_or_init(|| {
            self.container();
            let container = self.container.get().unwrap();
            tiff::identify_with_exif(container, &MAKE_TO_ID_MAP).unwrap_or(nikon!(UNKNOWN))
        })
    }

    fn container(&self) -> &dyn RawContainer {
        self.container.get_or_init(|| {
            // XXX we should be faillible here.
            let view = Viewer::create_view(&self.reader, 0).expect("Created view");
            let mut container =
                tiff::Container::new(view, vec![(tiff::IfdType::Main, None)], self.type_());
            container.load(None).expect("NEF container error");
            probe!(
                self.probe,
                "raw.container.endian",
                &format!("{:?}", container.endian())
            );
            container
        })
    }

    fn thumbnails(&self) -> &ThumbnailStorage {
        self.thumbnails.get_or_init(|| {
            self.container();
            let container = self.container.get().unwrap();
            let mut thumbnails = tiff::tiff_thumbnails(container);

            // Get the preview in the makernote
            if let Some(mnote) = self.ifd(tiff::IfdType::MakerNote) {
                // XXX is this the right id.
                // XXX add tag names
                mnote
                    .ifd_in_entry(
                        container,
                        exif::MNOTE_NIKON_PREVIEW_IFD,
                        Some("Nikon.Preview"),
                        None,
                    )
                    .and_then(|dir| {
                        let start = dir.value::<u32>(exif::MNOTE_NIKON_PREVIEWIFD_START)?
                            + mnote.mnote_offset;
                        let len = dir.value::<u32>(exif::MNOTE_NIKON_PREVIEWIFD_LENGTH)?;
                        container
                            .add_thumbnail_from_stream(start, len, &mut thumbnails)
                            .ok()
                    });
            }
            ThumbnailStorage::with_thumbnails(thumbnails)
        })
    }

    fn ifd(&self, ifd_type: tiff::IfdType) -> Option<&Dir> {
        self.container();
        let container = self.container.get().unwrap();
        match ifd_type {
            tiff::IfdType::Main => container.directory(0),
            tiff::IfdType::Raw => tiff::tiff_locate_raw_ifd(container),
            tiff::IfdType::Exif => container.exif_dir(),
            tiff::IfdType::MakerNote => container.mnote_dir(),
            _ => None,
        }
    }

    fn load_rawdata(&self, skip_decompress: bool) -> Result<RawImage> {
        self.ifd(tiff::IfdType::Raw)
            .ok_or_else(|| {
                log::error!("CFA not found");
                Error::NotFound
            })
            .and_then(|dir| {
                tiff::tiff_get_rawdata(self.container.get().unwrap(), dir, self.type_())
                    .map_err(|err| {
                        log::error!("NEF get rawdata failed {}", err);
                        err
                    })
                    .and_then(|rawdata| {
                        let compression = rawdata.compression();
                        probe!(self.probe, "nef.compression", &format!("{compression:?}"));
                        if self.is_d100() {
                            self.unpack_nikon(rawdata)
                        } else if compression == tiff::Compression::None {
                            Ok(rawdata)
                        } else if compression == tiff::Compression::NikonQuantized {
                            if !skip_decompress {
                                log::debug!("Nikon quantized");
                                self.decompress_nikon_quantized(rawdata).map_err(|err| {
                                    log::error!("NEF quantized {}", err);
                                    err
                                })
                            } else {
                                Ok(rawdata)
                            }
                        } else if self.is_nrw() {
                            // XXX decompression not yet supported
                            // is this a thing?
                            probe!(self.probe, "nef.is_nrw_compression", "true");
                            log::error!("NRW compression unsupported");
                            Ok(rawdata)
                        } else {
                            log::error!("Invalid compression {:?}", compression);
                            Ok(rawdata)
                        }
                    })
                    .map(|mut rawdata| {
                        if let Some(blacks) = self
                            .ifd(tiff::IfdType::MakerNote)
                            .and_then(|dir| dir.uint_value_array(exif::MNOTE_NIKON_BLACK_LEVEL))
                        {
                            probe!(self.probe, "nef.blacks.mnote", true);
                            rawdata.set_blacks(utils::to_quad(&blacks));
                        }
                        if let Some(wb) = self.white_balance() {
                            rawdata.set_as_shot_neutral(&wb);
                        }
                        rawdata
                    })
            })
    }

    fn get_builtin_colour_matrix(&self) -> Result<Vec<f64>> {
        MATRICES
            .iter()
            .find(|m| m.camera == self.type_id())
            .map(|m| Vec::from(m.matrix))
            .ok_or(Error::NotFound)
    }
}

impl RawFile for NefFile {
    fn type_(&self) -> Type {
        Type::Nef
    }
}

impl Dump for NefFile {
    #[cfg(feature = "dump")]
    fn write_dump<W: std::io::Write + ?Sized>(&self, out: &mut W, indent: u32) {
        dump_writeln!(out, indent, "<Nikon NEF File>");
        {
            let indent = indent + 1;
            self.container();
            self.container.get().unwrap().write_dump(out, indent);
        }
        dump_writeln!(out, indent, "</Nikon NEF File>");
    }
}

dumpfile_impl!(NefFile);
