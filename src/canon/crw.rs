// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - canon/crw.rs
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

//! Canon CRW format, the 2nd generation of Canon RAW format, based on
//! CIFF.

pub(crate) mod ciff;
mod decompress;

use std::collections::HashMap;
use std::io::{Read, Seek, SeekFrom};
use std::rc::Rc;

use once_cell::unsync::OnceCell;

use crate::bitmap;
use crate::canon;
use crate::canon::SensorInfo;
use crate::container::RawContainer;
use crate::io::Viewer;
use crate::jpeg;
use crate::mosaic::Pattern;
use crate::rawfile::{RawFileHandleType, ThumbnailStorage};
use crate::thumbnail;
use crate::thumbnail::{Data, ThumbDesc};
use crate::tiff;
use crate::tiff::exif;
use crate::tiff::Dir;
use crate::{
    DataType, Dump, Error, RawFile, RawFileHandle, RawFileImpl, RawImage, Result, Type, TypeId,
};

use super::matrices::MATRICES;
use decompress::Decompress;

lazy_static::lazy_static! {
    static ref MAKE_TO_ID_MAP: tiff::MakeToIdMap = HashMap::from([
        canon!("Canon EOS D30" , EOS_D30),
        canon!("Canon EOS D60" , EOS_D60),
        canon!("Canon EOS 10D" , EOS_10D),
        canon!("Canon EOS DIGITAL REBEL", DIGITAL_REBEL),
        canon!("Canon EOS 300D DIGITAL", EOS_300D),
        canon!("Canon PowerShot G1", G1),
        canon!("Canon PowerShot G2", G2),
        canon!("Canon PowerShot G3", G3),
        canon!("Canon PowerShot G5", G5),
        canon!("Canon PowerShot G6", G6),
        // G7 is CHDK, So remove from the list from now.
        //    canon!("Canon PowerShot G7", G7),
        canon!("Canon PowerShot Pro1", PRO1),
        canon!("Canon PowerShot Pro70", PRO70),
        canon!("Canon PowerShot Pro90 IS", PRO90),
        canon!("Canon PowerShot S30", S30),
        canon!("Canon PowerShot S40", S40),
        canon!("Canon PowerShot S45", S45),
        canon!("Canon PowerShot S50", S50),
        canon!("Canon PowerShot S60", S60),
        canon!("Canon PowerShot S70", S70),
    ]);
}

#[derive(Debug)]
/// Canon CRW File
pub(crate) struct CrwFile {
    reader: Rc<Viewer>,
    type_id: OnceCell<TypeId>,
    container: OnceCell<ciff::Container>,
    thumbnails: OnceCell<ThumbnailStorage>,
}

impl CrwFile {
    pub(crate) fn factory(reader: Rc<Viewer>) -> RawFileHandle {
        RawFileHandleType::new(CrwFile {
            reader,
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
                .unwrap_or(canon!(UNKNOWN))
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

    fn thumbnails(&self) -> &ThumbnailStorage {
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
            ThumbnailStorage::with_thumbnails(thumbnails)
        })
    }

    fn ifd(&self, ifd_type: tiff::IfdType) -> Option<&Dir> {
        self.container();
        let container = self.container.get().unwrap();
        match ifd_type {
            tiff::IfdType::Main => container.main_ifd(),
            tiff::IfdType::Exif => container.exif_ifd(),
            _ => None,
        }
    }

    fn load_rawdata(&self, skip_decompression: bool) -> Result<RawImage> {
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
                                let mut rawdata = RawImage::with_data8(
                                    x,
                                    y,
                                    component_bit_depth as u16,
                                    DataType::CompressedRaw,
                                    data,
                                    Pattern::Rggb,
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
                                        rawdata.set_whites([(1 << 10) - 1; 4]);
                                        rawdata.set_active_area(sensor_info);
                                        rawdata.set_mosaic_pattern(Pattern::Rggb);
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
    fn write_dump<W: std::io::Write + ?Sized>(&self, out: &mut W, indent: u32) {
        dump_writeln!(out, indent, "<Canon CRW File>");
        {
            let indent = indent + 1;
            self.container();
            self.container.get().unwrap().write_dump(out, indent);
        }
        dump_writeln!(out, indent, "</Canon CRW File>");
    }
}

dumpfile_impl!(CrwFile);
