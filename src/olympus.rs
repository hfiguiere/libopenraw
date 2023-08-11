// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - olympus.rs
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

//! Olympus ORF support

pub mod decompress;
mod matrices;

use std::collections::HashMap;
use std::rc::Rc;

use once_cell::unsync::OnceCell;

use crate::bitmap::{Bitmap, Rect};
use crate::container;
use crate::container::RawContainer;
use crate::io::Viewer;
use crate::rawfile::{RawFileHandleType, ThumbnailStorage};
use crate::tiff;
use crate::tiff::IfdType;
use crate::tiff::{exif, Ifd};
use crate::{
    DataType, Dump, Error, RawFile, RawFileHandle, RawFileImpl, RawImage, Result, Type, TypeId,
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
        olympus!("OM-5            ", OM5),
        olympus!("STYLUS1         ", STYLUS1),
        olympus!("STYLUS1,1s      ", STYLUS1_1S),
        olympus!("PEN-F           ", PEN_F),
        olympus!("SH-2            ", SH2),
        olympus!("TG-4            ", TG4),
        olympus!("TG-5            ", TG5),
        olympus!("TG-6            ", TG6),
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

#[derive(Debug)]
pub(crate) struct OrfFile {
    reader: Rc<Viewer>,
    type_id: OnceCell<TypeId>,
    container: OnceCell<tiff::Container>,
    thumbnails: OnceCell<ThumbnailStorage>,
}

impl OrfFile {
    pub fn factory(reader: Rc<Viewer>) -> RawFileHandle {
        RawFileHandleType::new(OrfFile {
            reader,
            type_id: OnceCell::new(),
            container: OnceCell::new(),
            thumbnails: OnceCell::new(),
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

    /// Decompress the Olympus RawData. Could be unpack.
    fn decompress(&self, mut data: RawImage) -> RawImage {
        let width = data.width();
        let height = data.height();

        if let Some(d) = data
            .data8()
            .or_else(|| data.data16_as_u8())
            .and_then(|data8| {
                if data8.len() == (height * ((width * 12 / 8) + ((width + 2) / 10))) as usize {
                    let mut buf = data8;
                    crate::decompress::unpack_from_reader(
                        &mut buf,
                        width,
                        height,
                        12,
                        tiff::Compression::Olympus,
                        data8.len(),
                    )
                    .ok()
                } else {
                    // Olympus decompression
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
            let mut container = tiff::Container::new(view, vec![IfdType::Main], self.type_());
            container
                .load(Some(OrfFile::is_magic_header))
                .expect("Olympus IFD container error");
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
                data.set_bpc(12);
                data.set_active_area(Some(Rect {
                    x: 0,
                    y: 0,
                    width,
                    height,
                }));
                data.set_whites([(1 << 12) - 1_u16; 4]);

                if let Some(mnote) = self.ifd(IfdType::MakerNote) {
                    // We are guaranted that container was created
                    if let Some(ip_dir) = self.olympus_ip_ifd(mnote) {
                        let active_area = Some(Rect::default()).and_then(|_| {
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
                            data.set_active_area(active_area);
                        }
                    }
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
