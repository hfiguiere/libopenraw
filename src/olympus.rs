// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - olympus.rs
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

//! Olympus ORF support

pub mod decompress;
mod matrices;

use std::collections::HashMap;
use std::rc::Rc;

use once_cell::unsync::OnceCell;
use rayon::prelude::*;

use crate::bitmap::Bitmap;
use crate::container;
use crate::container::{Endian, RawContainer};
use crate::io::Viewer;
use crate::rawfile::{RawFileHandleType, ThumbnailStorage};
use crate::tiff;
use crate::tiff::{exif, Ifd};
use crate::tiff::{IfdType, LoaderFixup};
use crate::utils;
use crate::{
    DataType, Dump, Error, RawFile, RawFileHandle, RawFileImpl, RawImage, Rect, Result, Type,
    TypeId,
};
pub use tiff::exif::generated::MNOTE_OLYMPUS_TAG_NAMES as MNOTE_TAG_NAMES;
use tiff::exif::generated::{
    MNOTE_OLYMPUS_CS_TAG_NAMES, MNOTE_OLYMPUS_EQ_TAG_NAMES, MNOTE_OLYMPUS_FI_TAG_NAMES,
    MNOTE_OLYMPUS_IP_TAG_NAMES, MNOTE_OLYMPUS_RD2_TAG_NAMES, MNOTE_OLYMPUS_RD_TAG_NAMES,
    MNOTE_OLYMPUS_RI_TAG_NAMES,
};

use decompress::decompress_olympus;
use matrices::MATRICES;

#[macro_export]
macro_rules! olympus {
    ($id:expr, $model:ident) => {
        (
            $id,
            TypeId(
                $crate::camera_ids::vendor::OLYMPUS,
                $crate::camera_ids::olympus::$model,
            ),
        )
    };
    ($model:ident) => {
        TypeId(
            $crate::camera_ids::vendor::OLYMPUS,
            $crate::camera_ids::olympus::$model,
        )
    };
}

lazy_static::lazy_static! {

    static ref MAKE_TO_ID_MAP: tiff::MakeToIdMap = HashMap::from([
        olympus!("E-1             ", E1),
        olympus!("E-10        "    , E10),
        olympus!("E-3             ", E3),
        olympus!("E-30            ", E30),
        olympus!("E-5             ", E5),
        olympus!("E-300           ", E300),
        olympus!("E-330           ", E330),
        olympus!("E-400           ", E400),
        olympus!("E-410           ", E410),
        olympus!("E-420           ", E420),
        olympus!("E-450           ", E450),
        olympus!("E-500           ", E500),
        olympus!("E-510           ", E510),
        olympus!("E-520           ", E520),
        olympus!("E-600           ", E600),
        olympus!("E-620           ", E620),
        olympus!("SP350"           , SP350),
        olympus!("SP500UZ"         , SP500UZ),
        olympus!("SP510UZ"         , SP510UZ),
        olympus!("SP550UZ                ", SP550UZ),
        olympus!("SP565UZ                ", SP565UZ),
        olympus!("SP570UZ                ", SP570UZ),
        olympus!("E-P1            ", EP1),
        olympus!("E-P2            ", EP2),
        olympus!("E-P3            ", EP3),
        olympus!("E-P5            ", EP5),
        olympus!("E-P7            ", EP7),
        olympus!("E-PL1           ", EPL1),
        olympus!("E-PL2           ", EPL2),
        olympus!("E-PL3           ", EPL3),
        olympus!("E-PL5           ", EPL5),
        olympus!("E-PL6           ", EPL6),
        olympus!("E-PL7           ", EPL7),
        olympus!("E-PL8           ", EPL8),
        olympus!("E-PL9           ", EPL9),
        olympus!("E-PL10          ", EPL10),
        olympus!("E-PM1           ", EPM1),
        olympus!("E-PM2           ", EPM2),
        olympus!("XZ-1            ", XZ1),
        olympus!("XZ-10           ", XZ10),
        olympus!("XZ-2            ", XZ2),
        olympus!("E-M5            ", EM5),
        olympus!("E-M5MarkII      ", EM5II),
        olympus!("E-M5MarkIII     ", EM5III),
        olympus!("E-M1            ", EM1),
        olympus!("E-M1MarkII      ", EM1II),
        olympus!("E-M1MarkIII     ", EM1III),
        olympus!("E-M1X           ", EM1X),
        olympus!("E-M10           ", EM10),
        olympus!("E-M10MarkII     ", EM10II),
        olympus!("E-M10 Mark III  ", EM10III),
        olympus!("E-M10MarkIIIS   ", EM10IIIS),
        olympus!("E-M10MarkIV     ", EM10IV),
        olympus!("OM-1            ", OM1),
        olympus!("OM-1MarkII      ", OM1II),
        olympus!("OM-5            ", OM5),
        olympus!("STYLUS1         ", STYLUS1),
        olympus!("STYLUS1,1s      ", STYLUS1_1S),
        olympus!("PEN-F           ", PEN_F),
        olympus!("SH-2            ", SH2),
        olympus!("TG-4            ", TG4),
        olympus!("TG-5            ", TG5),
        olympus!("TG-6            ", TG6),
        olympus!("TG-7            ", TG7),
        olympus!("C5060WZ", C5060WZ),
    ]);

    static ref MNOTE_TAG_TO_DIRID: HashMap<u16, &'static str> = HashMap::from([
        (exif::ORF_TAG_CAMERA_SETTINGS, "Exif.OlympusCs"),
        (exif::ORF_TAG_IMAGE_PROCESSING, "Exif.OlympusIp"),
        (exif::ORF_TAG_RAW_DEVELOPMENT, "Exif.OlympusRd"),
        (exif::ORF_TAG_RAW_DEVELOPMENT2, "Exif.OlympusRd2"),
        (exif::ORF_TAG_EQUIPMENT, "Exif.OlympusEq"),
        (exif::ORF_TAG_FOCUS_INFO, "Exif.OlympusFi"),
        (exif::ORF_TAG_RAW_INFO, "Exif.OlympusRi"),
    ]);
}

struct OrfFixup {}

impl LoaderFixup for OrfFixup {
    fn check_magic_header(&self, buf: &[u8]) -> Result<container::Endian> {
        OrfFile::is_magic_header(buf)
    }
}

#[derive(Debug)]
pub(crate) struct OrfFile {
    reader: Rc<Viewer>,
    type_id: OnceCell<TypeId>,
    container: OnceCell<tiff::Container>,
    thumbnails: OnceCell<ThumbnailStorage>,
    #[cfg(feature = "probe")]
    probe: Option<crate::Probe>,
}

impl OrfFile {
    pub fn factory(reader: Rc<Viewer>) -> RawFileHandle {
        RawFileHandleType::new(OrfFile {
            reader,
            type_id: OnceCell::new(),
            container: OnceCell::new(),
            thumbnails: OnceCell::new(),
            #[cfg(feature = "probe")]
            probe: None,
        })
    }

    /// Return the CFA dir
    fn cfa_dir(&self) -> Option<&tiff::Dir> {
        self.container();
        self.container.get().unwrap().directory(0)
    }

    /// Will identify the magic header for Olympus and return the endian
    /// Olympus slightly change over the standard TIFF header.
    fn is_magic_header(buf: &[u8]) -> Result<container::Endian> {
        if buf.len() < 4 {
            log::error!("Olympus magic header buffer too small: {} bytes", buf.len());
            return Err(Error::BufferTooSmall);
        }
        // the subtype is 'O' or 'S' and doesn't seem to be used.
        // 'S' seems to be for the C5060WZ
        if &buf[0..3] == b"IIR" && (buf[3] == b'O' || buf[3] == b'S') {
            Ok(container::Endian::Little)
        } else if &buf[0..2] == b"MM" && buf[3] == b'R' && (buf[2] == b'O' || buf[2] == b'S') {
            Ok(container::Endian::Big)
        } else {
            log::error!("Incorrect Olympus IFD magic: {:?}", buf);
            Err(Error::FormatError)
        }
    }

    // Inspired from rawloader::decoders::packed::decode_12be_interlaced
    //  https://github.com/pedrocr/rawloader/blob/master/src/decoders/packed.rs#L182
    fn decode_12be_olympus_interlaced(buf: &[u8], width: usize, height: usize) -> Vec<u16> {
        let half = (height + 1) >> 1;
        // Interlaced data seems to be 2048 bytes aligned
        let interlaced_offset = (((half * width * 3 / 2) >> 11) + 1) << 11;
        let interlaced = &buf[interlaced_offset..];

        let mut out: Vec<u16> = vec![0; width * height];
        out.par_chunks_mut(width)
            .enumerate()
            .for_each(|(row, outb)| {
                let off = row / 2 * width * 12 / 8;
                let inb = if (row % 2) == 0 {
                    &buf[off..]
                } else {
                    &interlaced[off..]
                };

                let mut i = 0;
                let mut o = 0;
                while o < outb.len() {
                    let in_slice = &inb[i..];
                    if in_slice.len() < 3 {
                        // If read short, it's the end.
                        break;
                    }
                    let out_slice = &mut outb[o..];
                    out_slice[0] = ((in_slice[0] as u16) << 4) | ((in_slice[1] as u16) >> 4);
                    out_slice[1] = ((in_slice[1] as u16 & 0x0f) << 8) | in_slice[2] as u16;
                    i += 3;
                    o += 2;
                }
            });

        out
    }

    // Inspired from rawloader::decoders::packed::decode_12be_msb32
    //  https://github.com/pedrocr/rawloader/blob/master/src/decoders/packed.rs#L111
    fn decode_12be_olympus(buf: &[u8], width: usize, height: usize) -> Vec<u16> {
        let mut out: Vec<u16> = vec![0; width * height];

        let mut i = 0;
        let mut o = 0;
        while i < buf.len() {
            let in_slice = &buf[i..];
            let out_slice = &mut out[o..];

            out_slice[0] = (in_slice[3] as u16) << 4 | in_slice[2] as u16 >> 4;
            out_slice[1] = (in_slice[2] as u16 & 0x0f) << 8 | in_slice[1] as u16;
            out_slice[2] = (in_slice[0] as u16) << 4 | in_slice[7] as u16 >> 4;
            out_slice[3] = (in_slice[7] as u16 & 0x0f) << 8 | in_slice[6] as u16;
            out_slice[4] = (in_slice[5] as u16) << 4 | in_slice[4] as u16 >> 4;
            out_slice[5] = (in_slice[4] as u16 & 0x0f) << 8 | in_slice[11] as u16;
            out_slice[6] = (in_slice[10] as u16) << 4 | in_slice[9] as u16 >> 4;
            out_slice[7] = (in_slice[9] as u16 & 0x0f) << 8 | in_slice[8] as u16;

            i += 12;
            o += 8;
        }

        out
    }

    /// Decompress the Olympus RawData. Could be unpack.
    fn decompress(&self, mut data: RawImage) -> RawImage {
        let width = data.width();
        let height = data.height();

        if let Some(d) = data
            .data8()
            .or_else(|| data.data16_as_u8())
            .and_then(|data8| {
                if data8.len() == (height * ((width * 12 / 8) + ((width + 2) / 10))) as usize {
                    probe!(self.probe, "orf.decompress.unpack", "true");
                    let mut buf = data8;
                    crate::decompress::unpack_from_reader(
                        &mut buf,
                        width,
                        height,
                        12,
                        tiff::Compression::Olympus,
                        data8.len(),
                        // Olympus is little endian.
                        Endian::Little,
                    )
                    .ok()
                } else if data8.len() == (height * width * 3 / 2) as usize {
                    // It seems that this is the only way™.
                    // Some decoder like rawloader use 3500,
                    // but SP565UZ is 3664 and interlaced.
                    if width < 3700 {
                        probe!(self.probe, "orf.decompress.interlaced", "true");
                        Some(Self::decode_12be_olympus_interlaced(
                            data8,
                            width as usize,
                            height as usize,
                        ))
                    } else {
                        probe!(self.probe, "orf.decompress.12be", "true");
                        Some(Self::decode_12be_olympus(
                            data8,
                            width as usize,
                            height as usize,
                        ))
                    }
                } else {
                    // Olympus decompression
                    probe!(self.probe, "orf.decompress.olympus", "true");
                    decompress_olympus(data8, width as usize, height as usize)
                        .map_err(|err| {
                            log::error!("Decompression failed {}", err);
                            err
                        })
                        .ok()
                }
            })
        {
            data.set_data16(d);
            data.set_compression(tiff::Compression::None);
            data.set_data_type(DataType::Raw);
        }
        data
    }

    fn olympus_cs_ifd(&self, mnote: &tiff::Dir) -> Option<tiff::Dir> {
        let dirid = MNOTE_TAG_TO_DIRID.get(&exif::ORF_TAG_CAMERA_SETTINGS);
        mnote.ifd_in_entry(
            self.container.get().unwrap(),
            exif::ORF_TAG_CAMERA_SETTINGS,
            dirid.cloned(),
            Some(&MNOTE_OLYMPUS_CS_TAG_NAMES),
        )
    }

    fn olympus_ip_ifd(&self, mnote: &tiff::Dir) -> Option<tiff::Dir> {
        let dirid = MNOTE_TAG_TO_DIRID.get(&exif::ORF_TAG_IMAGE_PROCESSING);
        mnote.ifd_in_entry(
            self.container.get().unwrap(),
            exif::ORF_TAG_IMAGE_PROCESSING,
            dirid.cloned(),
            Some(&MNOTE_OLYMPUS_IP_TAG_NAMES),
        )
    }

    fn olympus_rd_ifd(&self, mnote: &tiff::Dir) -> Option<tiff::Dir> {
        let dirid = MNOTE_TAG_TO_DIRID.get(&exif::ORF_TAG_RAW_DEVELOPMENT);
        mnote.ifd_in_entry(
            self.container.get().unwrap(),
            exif::ORF_TAG_RAW_DEVELOPMENT,
            dirid.cloned(),
            Some(&MNOTE_OLYMPUS_RD_TAG_NAMES),
        )
    }

    fn olympus_rd2_ifd(&self, mnote: &tiff::Dir) -> Option<tiff::Dir> {
        let dirid = MNOTE_TAG_TO_DIRID.get(&exif::ORF_TAG_RAW_DEVELOPMENT2);
        mnote.ifd_in_entry(
            self.container.get().unwrap(),
            exif::ORF_TAG_RAW_DEVELOPMENT2,
            dirid.cloned(),
            Some(&MNOTE_OLYMPUS_RD2_TAG_NAMES),
        )
    }

    fn olympus_fi_ifd(&self, mnote: &tiff::Dir) -> Option<tiff::Dir> {
        let dirid = MNOTE_TAG_TO_DIRID.get(&exif::ORF_TAG_FOCUS_INFO);
        mnote.ifd_in_entry(
            self.container.get().unwrap(),
            exif::ORF_TAG_FOCUS_INFO,
            dirid.cloned(),
            Some(&MNOTE_OLYMPUS_FI_TAG_NAMES),
        )
    }

    fn olympus_eq_ifd(&self, mnote: &tiff::Dir) -> Option<tiff::Dir> {
        let dirid = MNOTE_TAG_TO_DIRID.get(&exif::ORF_TAG_EQUIPMENT);
        mnote.ifd_in_entry(
            self.container.get().unwrap(),
            exif::ORF_TAG_EQUIPMENT,
            dirid.cloned(),
            Some(&MNOTE_OLYMPUS_EQ_TAG_NAMES),
        )
    }

    fn olympus_ri_ifd(&self, mnote: &tiff::Dir) -> Option<tiff::Dir> {
        let dirid = MNOTE_TAG_TO_DIRID.get(&exif::ORF_TAG_RAW_INFO);
        mnote.ifd_in_entry(
            self.container.get().unwrap(),
            exif::ORF_TAG_RAW_INFO,
            dirid.cloned(),
            Some(&MNOTE_OLYMPUS_RI_TAG_NAMES),
        )
    }
}

impl RawFileImpl for OrfFile {
    #[cfg(feature = "probe")]
    probe_imp!();

    fn identify_id(&self) -> TypeId {
        *self.type_id.get_or_init(|| {
            self.container();
            let container = self.container.get().unwrap();
            tiff::identify_with_exif(container, &MAKE_TO_ID_MAP).unwrap_or(olympus!(UNKNOWN))
        })
    }

    /// Return a lazily loaded `tiff::Container`
    fn container(&self) -> &dyn RawContainer {
        self.container.get_or_init(|| {
            // XXX we should be faillible here.
            let view = Viewer::create_view(&self.reader, 0).expect("Created view");
            let mut container =
                tiff::Container::new(view, vec![(IfdType::Main, None)], self.type_());
            container
                .load(Some(Box::new(OrfFixup {})))
                .expect("Olympus IFD container error");
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
            self.maker_note_ifd().and_then(|mnote| {
                mnote.entry(exif::ORF_TAG_THUMBNAIL_IMAGE).map(|e| {
                    container.add_thumbnail_from_entry(e, mnote.mnote_offset, &mut thumbnails)
                });
                self.olympus_cs_ifd(mnote).map(|dir| {
                    if dir
                        .value::<u32>(exif::ORF_TAG_CS_PREVIEW_IMAGE_VALID)
                        .unwrap_or(0)
                        != 0
                    {
                        let start = dir
                            .value::<u32>(exif::ORF_TAG_CS_PREVIEW_IMAGE_START)
                            .unwrap_or(0)
                            + mnote.mnote_offset;
                        let len = dir
                            .value::<u32>(exif::ORF_TAG_CS_PREVIEW_IMAGE_LENGTH)
                            .unwrap_or(0);
                        if start != 0 && len != 0 {
                            let _ =
                                container.add_thumbnail_from_stream(start, len, &mut thumbnails);
                        } else {
                            log::error!(
                                "ORF thumbnail valid but invalid start {} or len {}",
                                start,
                                len
                            );
                        }
                    }
                })
            });

            ThumbnailStorage::with_thumbnails(thumbnails)
        })
    }

    fn ifd(&self, ifd_type: tiff::IfdType) -> Option<&tiff::Dir> {
        self.container();
        let container = self.container.get().unwrap();
        match ifd_type {
            tiff::IfdType::Main => container.directory(0),
            tiff::IfdType::Raw => self.cfa_dir(),
            tiff::IfdType::Exif => container.exif_dir(),
            tiff::IfdType::MakerNote => container.mnote_dir(),
            _ => None,
        }
    }

    fn load_rawdata(&self, skip_decompress: bool) -> Result<RawImage> {
        self.ifd(IfdType::Raw)
            .ok_or(Error::NotFound)
            .and_then(|cfa| {
                self.container();
                tiff::tiff_get_rawdata(self.container.get().unwrap(), cfa, self.type_())
            })
            .map(|mut data| {
                let width = data.width();
                let height = data.height();
                let compression = if data.data_size() < width as usize * height as usize * 2 {
                    data.set_compression(tiff::Compression::Olympus);
                    data.set_data_type(DataType::CompressedRaw);
                    tiff::Compression::Olympus
                } else {
                    data.compression()
                };
                let mut data = match compression {
                    tiff::Compression::Olympus => {
                        if !skip_decompress {
                            self.decompress(data)
                        } else {
                            data
                        }
                    }
                    _ => data,
                };
                data.set_photometric_interpretation(exif::PhotometricInterpretation::CFA);
                data.set_bpc(12);
                data.set_active_area(Some(Rect {
                    x: 0,
                    y: 0,
                    width,
                    height,
                }));
                data.set_whites([(1 << 12) - 1_u16; 4]);

                let mut wb: Option<[f64; 4]> = None;
                if let Some(mnote) = self.ifd(IfdType::MakerNote) {
                    let wbr = mnote.uint_value(exif::ORF_TAG_RED_MULTIPLIER);
                    let wbb = mnote.uint_value(exif::ORF_TAG_BLUE_MULTIPLIER);
                    if let Some(wbr) = wbr {
                        if let Some(wbb) = wbb {
                            probe!(self.probe, "orf.ip.wb.mutlipliers", "true");
                            wb = Some([256.0 / wbr as f64, 1.0, 256.0 / wbb as f64, f64::NAN]);
                        }
                    }
                    // We are guaranted that container was created
                    if let Some(ip_dir) = self.olympus_ip_ifd(mnote) {
                        if wb.is_none() {
                            if let Some(wb_rb) = ip_dir
                                .float_value_array(exif::ORF_TAG_IP_WHITE_BALANCE_RB)
                                .filter(|v| v.len() == 2 || v.len() == 4)
                            {
                                probe!(self.probe, "orf.ip.wb.rb", "true");
                                wb = Some([256.0 / wb_rb[0], 1.0, 256.0 / wb_rb[1], f64::NAN]);
                            }
                        }

                        if let Some(blacks) = ip_dir.uint_value_array(exif::ORF_TAG_IP_BLACK_LEVEL2)
                        {
                            probe!(self.probe, "orf.ip.black_level2", blacks.len());
                            if blacks.len() == 1 {
                                data.set_blacks([blacks[0] as u16; 4]);
                            } else if blacks.len() == 4 {
                                data.set_blacks(utils::to_quad(&blacks));
                            }
                        }
                        let active_area = Some(Rect::default()).and_then(|_| {
                            // Note: in a DNG, it sets the active area to
                            // 0, 0, width, height.
                            probe!(self.probe, "orf.ip.active_area", "true");
                            let y = ip_dir.uint_value(exif::ORF_TAG_IP_CROP_TOP)?;
                            let x = ip_dir.uint_value(exif::ORF_TAG_IP_CROP_LEFT)?;
                            let width = ip_dir.uint_value(exif::ORF_TAG_IP_CROP_WIDTH)?;
                            let height = ip_dir.uint_value(exif::ORF_TAG_IP_CROP_HEIGHT)?;
                            Some(Rect {
                                x,
                                y,
                                width,
                                height,
                            })
                        });
                        if active_area.is_some() {
                            data.set_active_area(active_area.clone());
                        }
                        let user_crop = Some(Rect::default()).and_then(|_| {
                            probe!(self.probe, "orf.ip.user_crop", "true");
                            let values = ip_dir.uint_value_array(exif::ORF_TAG_IP_ASPECT_FRAME)?;
                            if values.len() < 4 {
                                return None;
                            }
                            let x = values[0];
                            let y = values[1];
                            let mut crop = Rect {
                                x,
                                y,
                                width: values[2] + 1 - x,
                                height: values[3] + 1 - y,
                            };
                            if let Some(active_area) = active_area {
                                crop.x += active_area.x;
                                crop.y += active_area.y;
                            }

                            Some(crop)
                        });
                        data.set_user_crop(user_crop, None);
                    }
                }
                if let Some(wb) = wb {
                    data.set_as_shot_neutral(&wb);
                }
                data
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

impl RawFile for OrfFile {
    fn type_(&self) -> Type {
        Type::Orf
    }
}

impl Dump for OrfFile {
    #[cfg(feature = "dump")]
    fn write_dump<W: std::io::Write + ?Sized>(&self, out: &mut W, indent: u32) {
        dump_writeln!(out, indent, "<Olympus ORF File>");
        {
            let indent = indent + 1;
            self.container();
            self.container.get().unwrap().write_dump(out, indent);
            if let Some(mnote) = self.maker_note_ifd() {
                if let Some(dir) = self.olympus_cs_ifd(mnote) {
                    dir.write_dump(out, indent);
                }
                if let Some(dir) = self.olympus_ip_ifd(mnote) {
                    dir.write_dump(out, indent);
                }
                if let Some(dir) = self.olympus_rd_ifd(mnote) {
                    dir.write_dump(out, indent);
                }
                if let Some(dir) = self.olympus_rd2_ifd(mnote) {
                    dir.write_dump(out, indent);
                }
                if let Some(dir) = self.olympus_fi_ifd(mnote) {
                    dir.write_dump(out, indent);
                }
                if let Some(dir) = self.olympus_eq_ifd(mnote) {
                    dir.write_dump(out, indent);
                }
                if let Some(dir) = self.olympus_ri_ifd(mnote) {
                    dir.write_dump(out, indent);
                }
            }
        }
        dump_writeln!(out, indent, "</Olympus ORF File>");
    }
}

dumpfile_impl!(OrfFile);
