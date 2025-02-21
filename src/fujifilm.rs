// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - fujifilm.rs
 *
 * Copyright (C) 2022-2025 Hubert Figuière
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

//! Fujifilm RAF format

mod decompress;
mod matrices;
mod raf;

use std::collections::HashMap;
use std::convert::{TryFrom, TryInto};
use std::io::{Seek, SeekFrom};
use std::rc::Rc;

use once_cell::unsync::OnceCell;

use crate::camera_ids::fujifilm;
use crate::container::{Endian, RawContainer};
use crate::decompress as unpack;
use crate::decompress::bit_reader::BitReaderLe32;
use crate::io::Viewer;
use crate::mosaic::Pattern;
use crate::rawfile::{RawFileHandleType, ThumbnailStorage};
use crate::thumbnail;
use crate::thumbnail::{Data, DataOffset};
use crate::tiff;
use crate::tiff::{exif, Ifd};
use crate::utils;
use crate::{
    AspectRatio, DataType, Dump, Error, Point, RawFile, RawFileHandle, RawFileImpl, RawImage, Rect,
    Result, Size, Type, TypeId,
};

use matrices::MATRICES;

pub(crate) const RAF_MAGIC: &[u8] = b"FUJIFILMCCD-RAW ";

#[macro_export]
macro_rules! fuji {
    ($id:expr, $model:ident) => {
        (
            $id,
            TypeId(
                $crate::camera_ids::vendor::FUJIFILM,
                $crate::camera_ids::fujifilm::$model,
            ),
        )
    };
    ($model:ident) => {
        TypeId(
            $crate::camera_ids::vendor::FUJIFILM,
            $crate::camera_ids::fujifilm::$model,
        )
    };
}

pub use tiff::exif::generated::MNOTE_FUJIFILM_RAWIFD_TAG_NAMES;
pub use tiff::exif::generated::MNOTE_FUJIFILM_TAG_NAMES as MNOTE_TAG_NAMES;

lazy_static::lazy_static! {
    /// Make to TypeId map for RAF files.
    static ref MAKE_TO_ID_MAP: tiff::MakeToIdMap = HashMap::from([
        fuji!("GFX 50S", GFX50S),
        fuji!("GFX50S II", GFX50S_II),
        fuji!("GFX 50R", GFX50R),
        fuji!("GFX 100", GFX100),
        fuji!("GFX100 II", GFX100_II),
        fuji!("GFX100S", GFX100S),
        fuji!("FinePix F550EXR", F550EXR),
        fuji!("FinePix F700  ", F700),
        fuji!("FinePix F810   ", F810),
        fuji!("FinePix E900   ", E900),
        fuji!("FinePixS2Pro", S2PRO),
        fuji!("FinePix S3Pro  ", S3PRO),
        fuji!("FinePix S5Pro  ", S5PRO),
        fuji!("FinePix S5000 ", S5000),
        fuji!("FinePix S5600  ", S5600),
        fuji!("FinePix S6000fd", S6000FD),
        fuji!("FinePix S6500fd", S6500FD),
        fuji!("FinePix S9500  ", S9500),
        fuji!("FinePix SL1000", SL1000),
        fuji!("FinePix HS10 HS11", HS10),
        fuji!("FinePix HS30EXR", HS30EXR),
        fuji!("FinePix HS33EXR", HS33EXR),
        fuji!("FinePix HS50EXR", HS50EXR),
        fuji!("FinePix S100FS ", S100FS),
        fuji!("FinePix S200EXR", S200EXR),
        fuji!("FinePix X100", X100),
        fuji!("X10", X10),
        fuji!("X20", X20),
        fuji!("X30", X30),
        fuji!("X70", X70),
        fuji!("X-Pro1", XPRO1),
        fuji!("X-Pro2", XPRO2),
        fuji!("X-Pro3", XPRO3),
        fuji!("X-S1", XS1),
        fuji!("X-S10", XS10),
        fuji!("X-S20", XS20),
        fuji!("X-A1", XA1),
        fuji!("X-A10", XA10),
        fuji!("X-A2", XA2),
        fuji!("X-A3", XA3),
        fuji!("X-A5", XA5),
        fuji!("X-A7", XA7),
        fuji!("XQ1", XQ1),
        fuji!("XQ2", XQ2),
        fuji!("X-E1", XE1),
        fuji!("X-E2", XE2),
        fuji!("X-E2S", XE2S),
        fuji!("X-E3", XE3),
        fuji!("X-E4", XE4),
        fuji!("X-M1", XM1),
        fuji!("X-M5", XM5),
        fuji!("X-T1", XT1),
        fuji!("X-T10", XT10),
        fuji!("X-T100", XT100),
        fuji!("X-T2", XT2),
        fuji!("X-T20", XT20),
        fuji!("X-T200", XT200),
        fuji!("X-T3", XT3),
        fuji!("X-T30", XT30),
        fuji!("X-T30 II", XT30_II),
        fuji!("X-T4", XT4),
        fuji!("X-T5", XT5),
        fuji!("X-T50", XT50),
        fuji!("XF1", XF1),
        fuji!("XF10", XF10),
        fuji!("X100S", X100S),
        fuji!("X100T", X100T),
        fuji!("X100F", X100F),
        fuji!("X100V", X100V),
        fuji!("X100VI", X100VI),
        fuji!("X-H1", XH1),
        fuji!("X-H2", XH2),
        fuji!("X-H2S", XH2S),
    ]);
}

#[derive(Debug)]
pub(crate) struct RafFile {
    reader: Rc<Viewer>,
    container: OnceCell<raf::RafContainer>,
    thumbnails: OnceCell<ThumbnailStorage>,
    #[cfg(feature = "probe")]
    probe: Option<crate::Probe>,
}

impl RafFile {
    pub fn factory(reader: Rc<Viewer>) -> RawFileHandle {
        RawFileHandleType::new(RafFile {
            reader,
            container: OnceCell::new(),
            thumbnails: OnceCell::new(),
            #[cfg(feature = "probe")]
            probe: None,
        })
    }
}

impl RawFileImpl for RafFile {
    #[cfg(feature = "probe")]
    probe_imp!();

    fn identify_id(&self) -> TypeId {
        self.container();
        let container = self.container.get().unwrap();
        let model = container.get_model();
        MAKE_TO_ID_MAP
            .get(&model)
            .copied()
            .unwrap_or(fuji!(UNKNOWN))
    }

    fn container(&self) -> &dyn RawContainer {
        self.container.get_or_init(|| {
            // XXX we should be faillible here.
            let view = Viewer::create_view(&self.reader, 0).expect("Created view");
            let mut container = raf::RafContainer::new(view);
            container.load().expect("Raf container error");
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
            let mut thumbnails = Vec::new();
            self.container();
            let container = self.container.get().unwrap();
            if let Some(jpeg) = container.jpeg_preview() {
                let width = jpeg.width();
                let height = jpeg.height();
                let dim = std::cmp::max(width, height) as u32;

                thumbnails.push((
                    dim,
                    thumbnail::ThumbDesc {
                        width: width as u32,
                        height: height as u32,
                        data_type: DataType::Jpeg,
                        data: Data::Offset(DataOffset {
                            offset: container.jpeg_offset() as u64,
                            len: container.jpeg_len() as u64,
                        }),
                    },
                ));

                jpeg.exif()
                    .and_then(|exif| {
                        exif.directory(1).and_then(|dir| {
                            let offset =
                                dir.value::<u32>(exif::EXIF_TAG_JPEG_INTERCHANGE_FORMAT)?;
                            let len =
                                dir.value::<u32>(exif::EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LENGTH)?;
                            let bytes = exif.load_buffer8(offset as u64, len as u64);
                            let mut byte_slice = bytes.as_slice();
                            let mut decoder = jpeg_decoder::Decoder::new(&mut byte_slice);
                            decoder
                                .read_info()
                                .map_err(|e| {
                                    log::error!("JPEG decoding error {}", e);
                                })
                                .ok()?;
                            let (width, height) =
                                decoder.info().map(|info| (info.width, info.height))?;
                            let dim = std::cmp::max(width, height) as u32;
                            thumbnails.push((
                                dim,
                                thumbnail::ThumbDesc {
                                    width: width as u32,
                                    height: height as u32,
                                    data_type: DataType::Jpeg,
                                    data: Data::Bytes(bytes),
                                },
                            ));
                            probe!(self.probe, "raf.thumbnail.exif", true);
                            Some(())
                        })
                    })
                    .or_else(|| {
                        log::error!("Failed to get thumbnail from Exif.");
                        None
                    });
            }
            ThumbnailStorage::with_thumbnails(thumbnails)
        })
    }

    fn ifd(&self, ifd_type: tiff::IfdType) -> Option<&tiff::Dir> {
        self.container();
        let raw_container = self.container.get().unwrap();
        match ifd_type {
            tiff::IfdType::Main => raw_container
                .jpeg_preview()
                .and_then(|jpeg| jpeg.exif())
                .and_then(|exif| exif.directory(0)),
            tiff::IfdType::Exif => raw_container
                .jpeg_preview()
                .and_then(|jpeg| jpeg.exif())
                .and_then(|exif| exif.exif_dir()),
            tiff::IfdType::MakerNote => raw_container
                .jpeg_preview()
                .and_then(|jpeg| jpeg.exif())
                .and_then(|exif| exif.mnote_dir()),
            _ => None,
        }
    }

    fn load_rawdata(&self, skip_decompress: bool) -> Result<RawImage> {
        self.container();
        let raw_container = self.container.get().unwrap();
        raw_container
            .meta_container()
            .ok_or(Error::NotFound)
            .and_then(|container| {
                // Dimensions are encapsulated into two u16 with an u32
                let raw_size = container
                    .value(raf::TAG_SENSOR_DIMENSION)
                    // Fujifilm HS10 doesn't have sensor dimension
                    .or_else(|| container.value(raf::TAG_IMG_HEIGHT_WIDTH))
                    .and_then(|v| Size::try_from(v).ok())
                    .ok_or_else(|| {
                        log::error!("Wrong RAF dimensions.");
                        Error::FormatError
                    })?;
                let active_area = container
                    .value(raf::TAG_IMG_TOP_LEFT)
                    .and_then(|topleft| Point::try_from(topleft).ok())
                    .and_then(|topleft| {
                        container
                            .value(raf::TAG_IMG_HEIGHT_WIDTH)
                            .and_then(|size| Size::try_from(size).ok())
                            .map(|size| Rect::new(topleft, size))
                    });
                let aspect_ratio = container
                    .value(raf::TAG_IMG_ASPECT_RATIO)
                    .and_then(|aspect_ratio| AspectRatio::try_from(aspect_ratio).ok());
                let crop = aspect_ratio.and_then(|aspect_ratio| {
                    active_area
                        .as_ref()
                        .map(|active_area| aspect_ratio.crop_into(active_area))
                });
                let raw_props = container
                    .value(raf::TAG_RAW_INFO)
                    .and_then(|v| match v {
                        raf::Value::Int(props) => Some(props),
                        _ => {
                            log::error!("Wrong RAF raw props");
                            None
                        }
                    })
                    .ok_or(Error::FormatError)?;

                // It's an XTrans if there is a `TAG_CFA_PATTERN` and it has 36 elements.
                let pattern = container.value(raf::TAG_CFA_PATTERN).and_then(|v| match v {
                    raf::Value::Bytes(pattern) => {
                        probe!(self.probe, "raf.cfa_pattern", true);
                        Some(pattern)
                    }
                    _ => None,
                });

                let mut wb = container.value(raf::TAG_WB_OLD).and_then(|v| match v {
                    raf::Value::Bytes(wb) => {
                        // XXX Unsure of the format here.
                        probe!(self.probe, "raf.wb.old", true);
                        let g = u16::from_be_bytes(wb[0..2].try_into().ok()?) as f64;
                        let r = u16::from_be_bytes(wb[2..4].try_into().ok()?) as f64;
                        let b = u16::from_be_bytes(wb[6..8].try_into().ok()?) as f64;
                        Some([g / r, 1.0, g / b, f64::NAN])
                    }
                    _ => None,
                });

                let mosaic = if let Some(pattern) = pattern {
                    // In the RAF file the pattern is inverted.
                    Pattern::try_from(
                        pattern
                            .iter()
                            .rev()
                            .copied()
                            .collect::<Vec<u8>>()
                            .as_slice(),
                    )
                    .map_err(|_| Error::FormatError)?
                } else {
                    match self.type_id().1 {
                        fujifilm::X10 | fujifilm::XF1 | fujifilm::XS1 => {
                            probe!(self.probe, "raf.cfa.bggr", true);
                            Pattern::Bggr
                        }
                        _ => Pattern::Rggb,
                    }
                };

                log::debug!("RAF raw props {:x}", raw_props);
                let layout = (raw_props & 0xff000000) >> 24 >> 7;
                probe!(self.probe, "raf.layout", layout);
                // This is unclear how significant it is as on X-Trans
                // compressed this is 0.
                let compression = ((raw_props & 0xff0000) >> 18) & 8;
                probe!(self.probe, "raf.compression", compression);
                let compressed = compression != 0;
                log::debug!("compressed {compressed}");

                let mut cfa_offset: u64 = 0;
                let mut cfa_len: u64 = 0;
                let mut bps: u16 = 12;
                let mut blacks = vec![0_u32; 4];

                if let Some(cfa_container) = raw_container.cfa_container() {
                    if let Some(dir) = cfa_container.directory(0).and_then(|dir| {
                        dir.ifd_in_entry(
                            cfa_container,
                            raf::FUJI_TAG_RAW_SUBIFD,
                            Some("Raw.Fujifilm"),
                            Some(&MNOTE_FUJIFILM_RAWIFD_TAG_NAMES),
                        )
                    }) {
                        cfa_offset = dir
                            .value::<u32>(raf::FUJI_TAG_RAW_OFFSET)
                            .map(|v| v + raw_container.cfa_offset())
                            .unwrap_or(0) as u64;
                        cfa_len = dir.value::<u32>(raf::FUJI_TAG_RAW_BYTE_LEN).unwrap_or(0) as u64;
                        bps = dir.value::<u32>(raf::FUJI_TAG_RAW_BPS).unwrap_or(0) as u16;
                        blacks = dir
                            .uint_value_array(raf::FUJI_TAG_RAW_BLACK_LEVEL_GRB)
                            .unwrap_or_else(|| vec![0_u32; 4]);
                        if wb.is_none() {
                            let wb_grb = dir.uint_value_array(raf::FUJI_TAG_RAW_WB_GRB).map(|v| {
                                probe!(self.probe, "raf.wb.grb", true);
                                let g = v[0] as f64;
                                [g / v[1] as f64, 1.0, g / v[2] as f64, f64::NAN]
                            });
                            wb = wb_grb;
                        }
                    }
                } else {
                    cfa_offset = raw_container.cfa_offset() as u64 + 2048;
                    cfa_len = raw_container.cfa_len() as u64 - 2048;
                    // XXX likely 12 is incorrect for 14.
                    bps = if compressed { 12 } else { 16 };
                }
                // Invalid value.
                if cfa_offset == 0 || cfa_len == 0 {
                    return Err(Error::NotFound);
                }
                probe!(self.probe, "raf.raw.bps", bps);
                let compressed =
                    cfa_len < (bps as u64 * raw_size.width as u64 * raw_size.height as u64 / 8);
                probe!(self.probe, "raf.compressed", compressed);
                let mut rawdata = if !compressed {
                    if cfa_len == (2 * raw_size.width as u64 * raw_size.height as u64) {
                        let buffer = raw_container.load_buffer16_le(cfa_offset, cfa_len);
                        RawImage::with_data16(
                            raw_size.width,
                            raw_size.height,
                            16,
                            DataType::Raw,
                            buffer,
                            mosaic,
                        )
                    } else {
                        let mut view = raw_container.borrow_view_mut();
                        let unpacked = if bps == 14 {
                            let mut unpacked = Vec::with_capacity(
                                raw_size.width as usize * raw_size.height as usize,
                            );
                            let view = crate::io::Viewer::create_subview(&view, cfa_offset)?;
                            let mut reader = BitReaderLe32::new(view);
                            unpack::unpack_14to16(
                                &mut reader,
                                raw_size.width as usize * raw_size.height as usize,
                                &mut unpacked,
                            )
                            .map_err(|err| {
                                log::error!("RAF failed to unpack 14 bits {err}");
                                err
                            })?;
                            unpacked
                        } else {
                            view.seek(SeekFrom::Start(cfa_offset))?;
                            unpack::unpack_from_reader(
                                &mut *view,
                                raw_size.width,
                                raw_size.height,
                                bps,
                                tiff::Compression::None,
                                cfa_len as usize,
                                Endian::Little,
                            )
                            .map_err(|err| {
                                log::error!("RAF failed to unpack {}", err);
                                err
                            })?
                        };
                        RawImage::with_data16(
                            raw_size.width,
                            raw_size.height,
                            16,
                            DataType::Raw,
                            unpacked,
                            mosaic,
                        )
                    }
                } else {
                    let mut raw = raw_container.load_buffer8(cfa_offset, cfa_len);
                    // The decompressor needs some extra bytes. We add 16.
                    raw.extend(&[0; 16]);
                    let mut rawbuffer = None;
                    if !skip_decompress {
                        rawbuffer = decompress::decompress_fuji(
                            &raw,
                            raw_size.width as usize,
                            raw_size.height as usize,
                            &mosaic,
                        )
                        .ok();
                    }

                    if let Some(rawbuffer) = rawbuffer {
                        RawImage::with_image_buffer(rawbuffer, DataType::Raw, mosaic)
                    } else {
                        RawImage::with_data8(
                            raw_size.width,
                            raw_size.height,
                            bps,
                            DataType::CompressedRaw,
                            raw,
                            mosaic,
                        )
                    }
                };

                if let Some(v) = container.value(raf::TAG_RAF_DATA) {
                    use crate::fujifilm::raf::Value;
                    if let Value::U32s(v) = v {
                        // The content of the tag is undocumented.
                        // But it is assumed that as long as a value is
                        // bigger than width then it's not right.
                        let mut idx = 0_usize;
                        while v[idx] >= raw_size.width && idx < v.len() {
                            idx += 1;
                        }
                        // We definitely want the index in probe in case
                        // it's out of bounds.
                        probe!(self.probe, "raf.raw.out.index", idx);
                        if idx + 1 < v.len() {
                            let output_w = v[idx];
                            let output_h = v[idx + 1];
                            probe!(self.probe, "raf.raw.out.width", output_w);
                            probe!(self.probe, "raf.raw.out.height", output_h);
                            rawdata.set_output_size(output_w, output_h);
                        }
                    }
                };
                rawdata.set_blacks(utils::to_quad(&blacks));
                rawdata.set_whites([((1 << bps as u32) - 1) as u16; 4]);
                rawdata.set_active_area(active_area);
                rawdata.set_user_crop(crop, aspect_ratio);
                if let Some(wb) = wb {
                    rawdata.set_as_shot_neutral(&wb);
                }

                Ok(rawdata)
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

impl RawFile for RafFile {
    fn type_(&self) -> Type {
        Type::Raf
    }
}

impl Dump for RafFile {
    #[cfg(feature = "dump")]
    fn write_dump<W: std::io::Write + ?Sized>(&self, out: &mut W, indent: u32) {
        dump_writeln!(out, indent, "<Fujifilm RAF File>");
        self.container();
        self.container.get().unwrap().write_dump(out, indent + 1);
        dump_writeln!(out, indent, "</Fujfilm RAF File>");
    }
}

dumpfile_impl!(RafFile);
