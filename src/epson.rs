// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - epson.rs
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

//! Epson ERF support.

use std::collections::HashMap;
use std::rc::Rc;

use once_cell::unsync::OnceCell;

use crate::camera_ids;
use crate::camera_ids::vendor;
use crate::colour::BuiltinMatrix;
use crate::container::RawContainer;
use crate::io::Viewer;
use crate::rawfile::RawFileHandleType;
use crate::rawfile::ThumbnailStorage;
use crate::thumbnail;
use crate::tiff;
use crate::tiff::{exif, Ifd, IfdType};
use crate::utils;
use crate::{
    Context, DataType, Dump, Error, RawFile, RawFileHandle, RawFileImpl, RawImage, Result, Type,
    TypeId,
};

/// The MakerNote tag names. It's actually the same as Olympus.
pub(crate) use crate::olympus::MNOTE_TAG_NAMES;

macro_rules! epson {
    ($id:expr, $model:ident) => {
        ($id, TypeId(vendor::EPSON, camera_ids::epson::$model))
    };
    ($model:ident) => {
        TypeId(vendor::EPSON, camera_ids::epson::$model)
    };
}

lazy_static::lazy_static! {
    /// EPSON built-in colour matrices
    static ref MATRICES: [BuiltinMatrix; 3] = [
        BuiltinMatrix::new(
            epson!(RD1), 0, 0,
            [ 6827, -1878, -732, -8429, 16012, 2564, -704, 592, 7145 ]
        ),
        BuiltinMatrix::new(
            epson!(RD1S), 0, 0,
            [ 6827, -1878, -732, -8429, 16012, 2564, -704, 592, 7145 ]
        ),
        BuiltinMatrix::new(
            epson!(RD1X), 0, 0,
            [ 6827, -1878, -732, -8429, 16012, 2564, -704, 592, 7145 ]
        ),
    ];

    /// Make to TypeId map for ERF files.
    static ref MAKE_TO_ID_MAP: tiff::MakeToIdMap = HashMap::from([
        epson!("R-D1", RD1),
        epson!("R-D1s", RD1S),
        epson!("R-D1x", RD1X),
    ]);
}

#[derive(Debug)]
/// ERF RAW file support
pub(crate) struct ErfFile {
    reader: Rc<Viewer>,
    type_id: OnceCell<TypeId>,
    container: OnceCell<Box<tiff::Container>>,
    thumbnails: OnceCell<ThumbnailStorage>,
    #[cfg(feature = "probe")]
    probe: Option<crate::Probe>,
}

impl ErfFile {
    pub fn factory(reader: Rc<Viewer>) -> RawFileHandle {
        RawFileHandleType::new(ErfFile {
            reader,
            type_id: OnceCell::new(),
            container: OnceCell::new(),
            thumbnails: OnceCell::new(),
            #[cfg(feature = "probe")]
            probe: None,
        })
    }
}

impl RawFileImpl for ErfFile {
    #[cfg(feature = "probe")]
    probe_imp!();

    fn identify_id(&self) -> Result<TypeId> {
        self.type_id
            .get_or_try_init(|| {
                self.container()?;
                let container = self.container.get().unwrap();
                Ok(tiff::identify_with_exif(container, &MAKE_TO_ID_MAP).unwrap_or(epson!(UNKNOWN)))
            })
            .copied()
    }

    /// Return a lazily loaded `tiff::Container`
    fn container(&self) -> Result<&dyn RawContainer> {
        self.container
            .get_or_try_init(|| {
                let view = Viewer::create_view(&self.reader, 0).context("Error creating view")?;
                let mut container =
                    tiff::Container::new(view, vec![(IfdType::Main, None)], self.type_());
                container.load(None).context("IFD container error")?;
                Ok(Box::new(container))
            })
            .map(|b| b.as_ref() as &dyn RawContainer)
    }

    fn thumbnails(&self) -> Result<&ThumbnailStorage> {
        self.thumbnails.get_or_try_init(|| {
            self.container()?;
            let container = self.container.get().unwrap();
            let mut thumbnails = tiff::tiff_thumbnails(container);
            self.maker_note_ifd().and_then(|mnote| {
                mnote.entry(exif::ERF_TAG_PREVIEW_IMAGE).map(|e| {
                    let mut data = Vec::from(e.data());
                    // The data start by 0xee instead of 0xff for a JPEG. Not sure why.
                    data[0] = 0xff;
                    let desc = thumbnail::ThumbDesc {
                        // It is 640x424 (3:2 aspect ratio)
                        width: 640,
                        height: 424,
                        data_type: DataType::Jpeg,
                        data: thumbnail::Data::Bytes(data),
                    };
                    thumbnails.push((640, desc));
                })
            });

            Ok(ThumbnailStorage::with_thumbnails(thumbnails))
        })
    }

    fn ifd(&self, ifd_type: tiff::IfdType) -> Option<&tiff::Dir> {
        self.container().ok()?;
        let container = self.container.get().unwrap();
        match ifd_type {
            tiff::IfdType::Main => container.directory(0),
            tiff::IfdType::Raw => tiff::tiff_locate_raw_ifd(container),
            tiff::IfdType::Exif => container.exif_dir(),
            tiff::IfdType::MakerNote => container.mnote_dir(),
            _ => None,
        }
    }

    fn load_rawdata(&self, _skip_decompression: bool) -> Result<RawImage> {
        self.ifd(tiff::IfdType::Raw)
            .ok_or_else(|| {
                log::error!("CFA not found");
                Error::NotFound
            })
            .and_then(|ifd| {
                self.container()?;
                tiff::tiff_get_rawdata(self.container.get().unwrap(), ifd, self.type_()).map(
                    |mut rawdata| {
                        self.maker_note_ifd().and_then(|mnote| {
                            mnote
                                .entry(exif::MNOTE_EPSON_SENSORAREA)
                                // the data type is `Undefined`
                                .and_then(|e| e.value_array::<u16>(mnote.endian()))
                                .or_else(|| {
                                    log::error!("Failed to read sensor area");
                                    None
                                })
                                .and_then(|a| {
                                    if a.len() >= 4 {
                                        rawdata.set_active_area(Some(crate::Rect {
                                            x: a[0] as u32,
                                            y: a[1] as u32,
                                            width: a[2] as u32,
                                            height: a[3] as u32,
                                        }));
                                        Some(())
                                    } else {
                                        None
                                    }
                                });
                            if let Some(blacks) =
                                mnote.uint_value_array(exif::MNOTE_EPSON_BLACK_LEVEL)
                            {
                                rawdata.set_blacks(utils::to_quad(&blacks));
                            }
                            if let Some(wb_values) = mnote.u16_value_array(exif::MNOTE_EPSON_WB) {
                                if wb_values.len() != 128 {
                                    log::error!("EPSON white balance len {}", wb_values.len());
                                }
                                // These values are taken directly from dcraw.
                                rawdata.set_as_shot_neutral(&[
                                    (0x10000 as f64) / ((wb_values[24] as f64) * 508.0 * 1.078),
                                    1.0,
                                    (0x10000 as f64) / ((wb_values[25] as f64) * 382.0 * 1.173),
                                ]);
                            }
                            None::<()>
                        });
                        rawdata
                    },
                )
            })
    }

    fn get_builtin_colour_matrix(&self) -> Result<Vec<f64>> {
        self.builtin_colour_matrix(&*MATRICES)
    }
}

impl RawFile for ErfFile {
    fn type_(&self) -> Type {
        Type::Erf
    }
}

impl Dump for ErfFile {
    #[cfg(feature = "dump")]
    fn write_dump<W: std::io::Write + ?Sized>(&self, out: &mut W, indent: u32) {
        dump_writeln!(out, indent, "<Epson ERF File>");
        {
            let indent = indent + 1;
            let _ = self.container();
            self.container.get().unwrap().write_dump(out, indent);
        }
        dump_writeln!(out, indent, "</Epson ERF File>");
    }
}

dumpfile_impl!(ErfFile);
