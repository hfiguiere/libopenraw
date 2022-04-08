/*
 * libopenraw - epson.rs
 *
 * Copyright (C) 2022 Hubert Figuière
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

use crate::bitmap;
use crate::camera_ids;
use crate::camera_ids::vendor;
use crate::container::GenericContainer;
use crate::io::Viewer;
use crate::rawfile::ReadAndSeek;
use crate::thumbnail;
use crate::tiff;
use crate::tiff::{exif, Ifd};
use crate::{DataType, Dump, Error, RawData, RawFile, RawFileImpl, Result, Type, TypeId};

/// The MakerNote tag names. It's actually the same as Olympus.
pub(crate) use crate::olympus::MNOTE_TAG_NAMES;

use crate::colour::BuiltinMatrix;

lazy_static::lazy_static! {
    /// EPSON built-in colour matrices
    static ref MATRICES: [BuiltinMatrix; 2] = [
        BuiltinMatrix::new(
            TypeId(vendor::EPSON, camera_ids::epson::RD1), 0, 0,
            [ 6827, -1878, -732, -8429, 16012, 2564, -704, 592, 7145 ]
        ),
        BuiltinMatrix::new(
            TypeId(vendor::EPSON, camera_ids::epson::RD1S), 0, 0,
            [ 6827, -1878, -732, -8429, 16012, 2564, -704, 592, 7145 ]
        ),
    ];
}

lazy_static::lazy_static! {
    /// Make to TypeId map for ERF files.
    static ref MAKE_TO_ID_MAP: tiff::MakeToIdMap = HashMap::from([
        ( "R-D1", TypeId(vendor::EPSON, camera_ids::epson::RD1) ),
        ( "R-D1s", TypeId(vendor::EPSON, camera_ids::epson::RD1S) ),
    ]);
}

/// ERF RAW file support
pub(crate) struct ErfFile {
    reader: Rc<Viewer>,
    container: OnceCell<tiff::Container>,
    thumbnails: OnceCell<Vec<(u32, thumbnail::ThumbDesc)>>,
    cfa: OnceCell<Option<Rc<tiff::Dir>>>,
}

impl ErfFile {
    pub fn factory(reader: Box<dyn ReadAndSeek>) -> Box<dyn RawFile> {
        let viewer = Viewer::new(reader);
        Box::new(ErfFile {
            reader: viewer,
            container: OnceCell::new(),
            thumbnails: OnceCell::new(),
            cfa: OnceCell::new(),
        })
    }

    /// Return the CFA dir
    fn cfa_dir(&self) -> Option<&Rc<tiff::Dir>> {
        self.cfa
            .get_or_init(|| {
                self.container();
                tiff::tiff_locate_cfa_ifd(self.container.get().unwrap())
            })
            .as_ref()
    }
}

impl RawFileImpl for ErfFile {
    fn identify_id(&self) -> TypeId {
        self.container();
        let container = self.container.get().unwrap();
        tiff::identify_with_exif(container, &MAKE_TO_ID_MAP).unwrap_or(TypeId(0, 0))
    }

    /// Return a lazily loaded `tiff::Container`
    fn container(&self) -> &dyn GenericContainer {
        self.container.get_or_init(|| {
            // XXX we should be faillible here.
            let view = Viewer::create_view(&self.reader, 0).expect("Created view");
            let mut container = tiff::Container::new(view, vec![], self.type_());
            container.load().expect("IFD container error");
            container
        })
    }

    fn thumbnails(&self) -> &Vec<(u32, thumbnail::ThumbDesc)> {
        self.thumbnails.get_or_init(|| {
            self.container();
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

            thumbnails
        })
    }

    fn ifd(&self, ifd_type: tiff::Type) -> Option<Rc<tiff::Dir>> {
        // XXX todo
        match ifd_type {
            tiff::Type::Cfa => self.cfa_dir().cloned(),
            tiff::Type::Exif => {
                self.container();
                self.container.get().unwrap().exif_dir()
            }
            tiff::Type::MakerNote => {
                self.container();
                self.container.get().unwrap().mnote_dir(self.type_())
            }
            _ => None,
        }
    }

    fn load_rawdata(&self) -> Result<RawData> {
        self.ifd(tiff::Type::Cfa)
            .ok_or_else(|| {
                log::error!("CFA not found");
                Error::NotFound
            })
            .and_then(|ref ifd| {
                self.container();
                tiff::tiff_get_rawdata(self.container.get().unwrap(), ifd).map(|mut rawdata| {
                    self.maker_note_ifd().and_then(|mnote| {
                        mnote
                            .entry_cloned(
                                exif::MNOTE_EPSON_SENSORAREA,
                                &mut self.container().borrow_view_mut(),
                            )
                            // the data type is `Undefined`
                            .and_then(|e| e.value_array::<u16>(mnote.endian()))
                            .or_else(|| {
                                log::error!("Failed to read sensor area");
                                None
                            })
                            .and_then(|a| {
                                if a.len() >= 4 {
                                    rawdata.set_active_area(Some(bitmap::Rect {
                                        x: a[0] as u32,
                                        y: a[1] as u32,
                                        width: a[2] as u32,
                                        height: a[3] as u32,
                                    }));
                                    Some(())
                                } else {
                                    None
                                }
                            })
                    });
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

impl RawFile for ErfFile {
    fn type_(&self) -> Type {
        Type::Erf
    }
}

impl Dump for ErfFile {
    fn print_dump(&self, indent: u32) {
        dump_println!(indent, "<Epson ERF File>");
        {
            let indent = indent + 1;
            self.container().print_dump(indent);
        }
        dump_println!(indent, "</Epson ERF File>");
    }
}
