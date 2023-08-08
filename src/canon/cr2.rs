// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - canon/cr2.rs
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

//! Canon CR2 format, the 2nd generation of Canon RAW format, based on
//! TIFF.

use std::rc::Rc;

use once_cell::unsync::OnceCell;

use crate::bitmap::{self, Bitmap};
use crate::canon;
use crate::container::RawContainer;
use crate::decompress;
use crate::io::Viewer;
use crate::mosaic::Pattern;
use crate::rawfile::RawFileHandleType;
use crate::rawfile::ThumbnailStorage;
use crate::tiff;
use crate::tiff::{exif, Dir, Ifd};
use crate::{
    DataType, Dump, Error, RawFile, RawFileHandle, RawFileImpl, RawImage, Result, Type, TypeId,
};

use super::matrices::MATRICES;

#[derive(Debug)]
/// Canon CR2 File
pub(crate) struct Cr2File {
    reader: Rc<Viewer>,
    type_id: OnceCell<TypeId>,
    container: OnceCell<tiff::Container>,
    thumbnails: OnceCell<ThumbnailStorage>,
}

impl Cr2File {
    pub(crate) fn factory(reader: Rc<Viewer>) -> RawFileHandle {
        RawFileHandleType::new(Cr2File {
            reader,
            type_id: OnceCell::new(),
            container: OnceCell::new(),
            thumbnails: OnceCell::new(),
        })
    }

    fn is_cr2(&self) -> bool {
        // XXX todo
        true
    }

    /// Get the raw bytes.
    fn get_raw_bytes(
        &self,
        width: u32,
        height: u32,
        offset: u64,
        byte_len: u64,
        slices: &[u32],
        skip_decompress: bool,
    ) -> Result<RawImage> {
        let data = self.container().load_buffer8(offset, byte_len);
        if (data.len() as u64) != byte_len {
            log::warn!("Size mismatch for data. Moving on");
        }

        if skip_decompress {
            Ok(RawImage::with_data8(
                width,
                height,
                8,
                DataType::CompressedRaw,
                data,
                Pattern::default(),
            ))
        } else {
            let mut decompressor = decompress::LJpeg::new();
            // in fact on Canon CR2 files slices either do not exists
            // or is 3.
            if slices.len() > 1 {
                decompressor.set_slices(slices);
            }

            let mut io = std::io::Cursor::new(data);
            decompressor.decompress(&mut io).map(|buffer| {
                RawImage::with_image_buffer(buffer, DataType::Raw, Pattern::default())
            })
        }
    }

    /// Load the `RawImage` for actual CR2 files.
    fn load_cr2_rawdata(&self, skip_decompress: bool) -> Result<RawImage> {
        self.container();
        let container = self.container.get().unwrap();

        let cfa_ifd = self.ifd(tiff::IfdType::Raw).ok_or_else(|| {
            log::debug!("CFA IFD not found");
            Error::NotFound
        })?;
        let offset = cfa_ifd
            .value::<u32>(exif::EXIF_TAG_STRIP_OFFSETS)
            .ok_or_else(|| {
                log::debug!("offset not found");
                Error::NotFound
            })?;
        let byte_len = cfa_ifd
            .value::<u32>(exif::EXIF_TAG_STRIP_BYTE_COUNTS)
            .ok_or_else(|| {
                log::debug!("byte len not found");
                Error::NotFound
            })?;
        let slices = cfa_ifd
            .entry(exif::CR2_TAG_SLICE)
            .or_else(|| {
                log::debug!("CR2 slice not found");
                None
            })
            .and_then(|entry| entry.uint_value_array(container.endian()))
            .or_else(|| {
                log::debug!("CR2 slice value not found");
                None
            })
            .unwrap_or_default();

        // The tags exif::EXIF_TAG_PIXEL_X_DIMENSION
        // and exif::EXIF_TAG_PIXEL_Y_DIMENSION from the Exif IFD
        // contain X & Y but we don't need them right now.
        // We'll use the active area and the JPEG stream.
        // But we need this if we skip decompression.
        let width = cfa_ifd
            .uint_value(exif::EXIF_TAG_PIXEL_X_DIMENSION)
            .unwrap_or_default();
        let height = cfa_ifd
            .uint_value(exif::EXIF_TAG_PIXEL_Y_DIMENSION)
            .unwrap_or_default();

        let mut rawdata = self.get_raw_bytes(
            width,
            height,
            offset as u64,
            byte_len as u64,
            &slices,
            skip_decompress,
        )?;

        let sensor_info = self
            .ifd(tiff::IfdType::MakerNote)
            .and_then(super::SensorInfo::new)
            .map(|sensor_info| bitmap::Rect {
                x: sensor_info.0[0],
                y: sensor_info.0[1],
                width: sensor_info.0[2],
                height: sensor_info.0[3],
            });
        rawdata.set_active_area(sensor_info);
        // XXX they are not all RGGB.
        // XXX but I don't seem to see where this is encoded.
        rawdata.set_mosaic_pattern(Pattern::Rggb);

        // Get the black and white point from the built-in matrices.
        let bpc = rawdata.bpc();
        let (black, white) = MATRICES
            .iter()
            .find(|m| m.camera == self.type_id())
            .map(|m| {
                (
                    m.black,
                    if m.white == 0 {
                        // A 0 value for white isn't valid.
                        let white: u32 = (1 << bpc) - 1;
                        white as u16
                    } else {
                        m.white
                    },
                )
            })
            .unwrap_or_else(|| {
                let white: u32 = (1 << bpc) - 1;
                (0, white as u16)
            });
        rawdata.set_black(black);
        rawdata.set_white(white);

        Ok(rawdata)
    }
}

impl RawFileImpl for Cr2File {
    fn identify_id(&self) -> TypeId {
        *self.type_id.get_or_init(|| {
            if let Some(maker_note) = self.maker_note_ifd() {
                super::identify_from_maker_note(maker_note)
            } else {
                log::error!("MakerNote not found");
                canon!(UNKNOWN)
            }
        })
    }

    /// Return a lazily loaded `tiff::Container`
    fn container(&self) -> &dyn RawContainer {
        self.container.get_or_init(|| {
            // XXX we should be faillible here.
            let view = Viewer::create_view(&self.reader, 0).expect("Created view");
            let mut container = tiff::Container::new(
                // XXX non CR2 have a different layout
                view,
                vec![
                    tiff::IfdType::Main,
                    tiff::IfdType::Other,
                    tiff::IfdType::Other,
                    tiff::IfdType::Raw,
                ],
                self.type_(),
            );
            container.load(None).expect("TIFF container error");
            container
        })
    }

    fn thumbnails(&self) -> &ThumbnailStorage {
        self.thumbnails.get_or_init(|| {
            ThumbnailStorage::with_thumbnails(if self.is_cr2() {
                self.container();
                let container = self.container.get().unwrap();
                tiff::tiff_thumbnails(container)
            } else {
                // XXX todo non CR2 files
                vec![]
            })
        })
    }

    fn ifd(&self, ifd_type: tiff::IfdType) -> Option<&Dir> {
        self.container();
        let container = self.container.get().unwrap();
        match ifd_type {
            tiff::IfdType::Raw => {
                if !self.is_cr2() {
                    self.ifd(tiff::IfdType::MakerNote)
                } else {
                    // XXX todo set the IFD to type Cfa
                    container.directory(3)
                }
            }
            tiff::IfdType::Main =>
            // XXX todo set the IFD to type Main
            {
                container.directory(0)
            }
            tiff::IfdType::Exif => container.exif_dir(),
            tiff::IfdType::MakerNote => container.mnote_dir(),
            _ => None,
        }
    }

    fn load_rawdata(&self, skip_decompress: bool) -> Result<RawImage> {
        if self.is_cr2() {
            return self.load_cr2_rawdata(skip_decompress);
        }
        Err(Error::NotSupported)
    }

    fn get_builtin_colour_matrix(&self) -> Result<Vec<f64>> {
        MATRICES
            .iter()
            .find(|m| m.camera == self.type_id())
            .map(|m| Vec::from(m.matrix))
            .ok_or(Error::NotFound)
    }
}

impl RawFile for Cr2File {
    fn type_(&self) -> Type {
        Type::Cr2
    }
}

impl Dump for Cr2File {
    #[cfg(feature = "dump")]
    fn write_dump<W: std::io::Write + ?Sized>(&self, out: &mut W, indent: u32) {
        dump_writeln!(out, indent, "<Canon CR2 File>");
        {
            let indent = indent + 1;
            self.container();
            self.container.get().unwrap().write_dump(out, indent);
        }
        dump_writeln!(out, indent, "</Canon CR2 File>");
    }
}

dumpfile_impl!(Cr2File);
