// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - canon/crw.rs
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

//! Canon CRW format, the 2nd generation of Canon RAW format, based on
//! CIFF.

mod ciff;
mod decompress;

use std::collections::HashMap;
use std::io::{Read, Seek, SeekFrom};
use std::rc::Rc;

use once_cell::unsync::OnceCell;

use crate::bitmap;
use crate::camera_ids::{canon, vendor};
use crate::canon::SensorInfo;
use crate::container::RawContainer;
use crate::io::Viewer;
use crate::jpeg;
use crate::rawfile::ReadAndSeek;
use crate::thumbnail;
use crate::thumbnail::{Data, ThumbDesc};
use crate::tiff;
use crate::tiff::exif;
use crate::tiff::Dir;
use crate::{DataType, Dump, Error, RawData, RawFile, RawFileImpl, Result, Type, TypeId};

use super::matrices::MATRICES;
use decompress::Decompress;

lazy_static::lazy_static! {
    static ref MAKE_TO_ID_MAP: tiff::MakeToIdMap = HashMap::from([
        ( "Canon EOS D30" , TypeId(vendor::CANON, canon::EOS_D30) ),
        ( "Canon EOS D60" , TypeId(vendor::CANON, canon::EOS_D60) ),
        ( "Canon EOS 10D" , TypeId(vendor::CANON, canon::EOS_10D) ),
        ( "Canon EOS DIGITAL REBEL", TypeId(vendor::CANON, canon::DIGITAL_REBEL) ),
        ( "Canon EOS 300D DIGITAL", TypeId(vendor::CANON, canon::EOS_300D) ),
        ( "Canon PowerShot G1", TypeId(vendor::CANON, canon::G1) ),
        ( "Canon PowerShot G2", TypeId(vendor::CANON, canon::G2) ),
        ( "Canon PowerShot G3", TypeId(vendor::CANON, canon::G3) ),
        ( "Canon PowerShot G5", TypeId(vendor::CANON, canon::G5) ),
        ( "Canon PowerShot G6", TypeId(vendor::CANON, canon::G6) ),
        // G7 is CHDK, So remove from the list from now.
        //    ( "Canon PowerShot G7", TypeId(vendor::CANON, canon::G7) ),
        ( "Canon PowerShot Pro1", TypeId(vendor::CANON, canon::PRO1) ),
        ( "Canon PowerShot Pro70", TypeId(vendor::CANON, canon::PRO70) ),
        ( "Canon PowerShot Pro90 IS", TypeId(vendor::CANON, canon::PRO90) ),
        ( "Canon PowerShot S30", TypeId(vendor::CANON, canon::S30) ),
        ( "Canon PowerShot S40", TypeId(vendor::CANON, canon::S40) ),
        ( "Canon PowerShot S45", TypeId(vendor::CANON, canon::S45) ),
        ( "Canon PowerShot S50", TypeId(vendor::CANON, canon::S50) ),
        ( "Canon PowerShot S60", TypeId(vendor::CANON, canon::S60) ),
        ( "Canon PowerShot S70", TypeId(vendor::CANON, canon::S70) ),
    ]);
}

/// Canon CRW File
pub struct CrwFile {
    reader: Rc<Viewer>,
    type_id: OnceCell<TypeId>,
    container: OnceCell<ciff::Container>,
    thumbnails: OnceCell<Vec<(u32, ThumbDesc)>>,
}

impl CrwFile {
    pub fn factory(reader: Box<dyn ReadAndSeek>) -> Box<dyn RawFile> {
        let viewer = Viewer::new(reader, 0);
        Box::new(CrwFile {
            reader: viewer,
            type_id: OnceCell::new(),
            container: OnceCell::new(),
            thumbnails: OnceCell::new(),
        })
    }
}

impl RawFileImpl for CrwFile {
    fn identify_id(&self) -> TypeId {
        *self.type_id.get_or_init(|| {
            self.container();
            let container = self.container.get().unwrap();
            container
                .make_or_model(exif::EXIF_TAG_MODEL)
                .and_then(|model| MAKE_TO_ID_MAP.get(model.as_str()).copied())
                .unwrap_or(TypeId(vendor::CANON, 0))
        })
    }

    /// Return a lazily loaded `tiff::Container`
    fn container(&self) -> &dyn RawContainer {
        self.container.get_or_init(|| {
            // XXX we should be faillible here.
            let view = Viewer::create_view(&self.reader, 0).expect("Created view");
            ciff::Container::new(view)
        })
    }

    fn thumbnails(&self) -> &Vec<(u32, ThumbDesc)> {
        self.thumbnails.get_or_init(|| {
            let mut thumbnails = Vec::new();
            self.container();
            let container = self.container.get().unwrap();
            let heap = container.heap();
            heap.records().get(&ciff::Tag::JpegImage).and_then(|r| {
                if let ciff::Record::InHeap((pos, len)) = r.data {
                    Viewer::create_subview(&container.borrow_view_mut(), pos as u64)
                        .map(|view| {
                            let jpeg = jpeg::Container::new(view, Type::Crw);
                            let x = jpeg.width();
                            let y = jpeg.height();
                            let dim = std::cmp::max(x, y) as u32;
                            if dim > 0 {
                                let desc = ThumbDesc {
                                    width: x as u32,
                                    height: y as u32,
                                    data_type: DataType::Jpeg,
                                    data: Data::Offset(thumbnail::DataOffset {
                                        offset: pos as u64,
                                        len: len as u64,
                                    }),
                                };
                                thumbnails.push((dim, desc));
                            }

                            jpeg
                        })
                        .ok()
                } else {
                    None
                }
            });
            thumbnails
        })
    }

    fn ifd(&self, _ifd_type: tiff::IfdType) -> Option<Rc<Dir>> {
        None
    }

    fn load_rawdata(&self, skip_decompression: bool) -> Result<RawData> {
        self.container();
        let container = self.container.get().unwrap();

        let mut component_bit_depth = 12;
        let (x, y) = container
            .image_spec()
            .map(|spec| {
                component_bit_depth = spec.component_bit_depth;
                (spec.image_width, spec.image_height)
            })
            .ok_or(Error::NotFound)?;

        container
            .exif_info()
            .ok_or(Error::NotFound)
            .and_then(|heap| {
                let decoder_table = heap
                    .records()
                    .get(&ciff::Tag::DecoderTable)
                    .ok_or(Error::NotFound)
                    .and_then(|r| {
                        if let ciff::Record::InHeap((pos, _)) = r.data {
                            let mut view = container.borrow_view_mut();
                            view.seek(SeekFrom::Start(pos as u64))?;
                            let table = view.read_endian_u32(container.endian())?;
                            Ok(table)
                        } else {
                            Err(Error::NotFound)
                        }
                    })?;
                let mut cfa_x = 0;
                let mut cfa_y = 0;
                let sensor_info = heap.records().get(&ciff::Tag::SensorInfo).and_then(|r| {
                    if let ciff::Record::InHeap((pos, _)) = r.data {
                        let mut view = container.borrow_view_mut();
                        view.seek(SeekFrom::Start(pos as u64)).ok()?;
                        let mut sensor_info = vec![0_u16; 9];
                        view.read_endian_u16_array(&mut sensor_info, container.endian())
                            .ok()?;
                        cfa_x = sensor_info[1];
                        cfa_y = sensor_info[2];
                        SensorInfo::parse(sensor_info).map(|sensor_info| bitmap::Rect {
                            x: sensor_info.0[0],
                            y: sensor_info.0[1],
                            width: sensor_info.0[2],
                            height: sensor_info.0[3],
                        })
                    } else {
                        None
                    }
                });
                container
                    .raw_data_record()
                    .ok_or(Error::NotFound)
                    .and_then(|r| {
                        if let ciff::Record::InHeap((pos, len)) = r.data {
                            // XXX pattern is RGGB
                            if skip_decompression {
                                let mut view = container.borrow_view_mut();
                                view.seek(SeekFrom::Start(pos as u64))?;
                                let mut data = vec![0; len as usize];
                                view.read_exact(&mut data)?;
                                let mut rawdata = RawData::new8(
                                    x,
                                    y,
                                    component_bit_depth as u16,
                                    DataType::CompressedRaw,
                                    data,
                                );
                                rawdata.set_active_area(sensor_info);
                                Ok(rawdata)
                            } else {
                                let mut decompressor = Decompress::new(
                                    decoder_table as usize,
                                    cfa_x as u32,
                                    cfa_y as u32,
                                );
                                let mut view = Viewer::create_subview(
                                    &container.borrow_view_mut(),
                                    pos as u64,
                                )?;
                                decompressor
                                    .decompress(&mut view)
                                    .map(|mut rawdata| {
                                        rawdata.set_white((1 << 10) - 1);
                                        rawdata.set_active_area(sensor_info);
                                        rawdata
                                    })
                                    .map_err(|err| {
                                        log::error!("CRW error: {:?}", err);
                                        err
                                    })
                            }
                        } else {
                            Err(Error::NotFound)
                        }
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

impl RawFile for CrwFile {
    fn type_(&self) -> Type {
        Type::Crw
    }
}

impl Dump for CrwFile {
    #[cfg(feature = "dump")]
    fn print_dump(&self, indent: u32) {
        dump_println!(indent, "<Canon CRW File>");
        {
            let indent = indent + 1;
            self.container().print_dump(indent);
        }
        dump_println!(indent, "</Canon CRW File>");
    }
}
