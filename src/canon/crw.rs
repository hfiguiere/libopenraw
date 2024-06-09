// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - canon/crw.rs
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

//! Canon CRW format, the 2nd generation of Canon RAW format, based on
//! CIFF.

pub(crate) mod ciff;
mod decompress;

use std::collections::HashMap;
use std::io::{Read, Seek, SeekFrom};
use std::rc::Rc;

use once_cell::unsync::OnceCell;

use crate::bitmap;
use crate::camera_ids::canon as canon_id;
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
    DataType, Dump, Error, RawFile, RawFileHandle, RawFileImpl, RawImage, Rect, Result, Size, Type,
    TypeId,
};

use super::matrices::MATRICES;
use ciff::Heap;
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

    /// Extract the sensor info `Rect` and raw image size `Size`.
    fn get_sensor_info(&self, heap: &Heap) -> Option<(Size, Rect)> {
        let container = self.container.get().unwrap();

        heap.records().get(&ciff::Tag::SensorInfo).and_then(|r| {
            if let ciff::Record::InHeap((pos, _)) = r.data {
                let mut view = container.borrow_view_mut();
                view.seek(SeekFrom::Start(pos as u64)).ok()?;
                let mut sensor_info = vec![0_u16; 9];
                view.read_endian_u16_array(&mut sensor_info, container.endian())
                    .ok()?;
                let dim = Size {
                    width: sensor_info[1] as u32,
                    height: sensor_info[2] as u32,
                };
                SensorInfo::parse(sensor_info).map(|sensor_info| {
                    (
                        dim,
                        bitmap::Rect {
                            x: sensor_info.0[0],
                            y: sensor_info.0[1],
                            width: sensor_info.0[2],
                            height: sensor_info.0[3],
                        },
                    )
                })
            } else {
                None
            }
        })
    }

    const CRW_WB_DEFAULT_OFFSET: u64 = 120;

    fn crw_wb_offset(&self) -> u64 {
        match self.type_id().1 {
            canon_id::PRO1 | canon_id::G6 | canon_id::S60 | canon_id::S70 => 96,
            _ => Self::CRW_WB_DEFAULT_OFFSET,
        }
    }

    fn get_white_balance(&self, heap: &Heap) -> Option<Vec<f64>> {
        let container = self.container.get().unwrap();

        // Check ColourInfo2 first. A G2 also has ColourInfo1.
        let wb = heap.records().get(&ciff::Tag::ColourInfo2).and_then(|r| {
            if let ciff::Record::InHeap((pos, _)) = r.data {
                let mut view = container.borrow_view_mut();
                view.seek(SeekFrom::Start(pos as u64)).ok()?;
                let magic = view.read_endian_u16(container.endian()).unwrap_or(0);
                if magic > 512 {
                    view.seek(SeekFrom::Start(pos as u64 + 120)).ok()?;
                    let mut wb_data = [0_u16; 4];
                    view.read_endian_u16_array(&mut wb_data, container.endian())
                        .ok();
                    let g = wb_data[0] as f64;
                    Some(vec![
                        g / wb_data[2] as f64,
                        g / wb_data[3] as f64,
                        1.0,
                        g / wb_data[1] as f64,
                    ])
                } else if magic != 276 {
                    // G2, S30, S40
                    view.seek(SeekFrom::Start(pos as u64 + 100)).ok()?;
                    let mut wb_data = [0_u16; 4];
                    view.read_endian_u16_array(&mut wb_data, container.endian())
                        .ok();
                    let g = (wb_data[0] + wb_data[3]) as f64 / 2.0;
                    Some(vec![g / wb_data[1] as f64, 1.0, g / wb_data[2] as f64])
                } else {
                    log::warn!("Unexpected ColourInfo2 magic {magic}");
                    None
                }
            } else {
                None
            }
        });
        if wb.is_some() {
            return wb;
        }

        // D30, G, S.
        let wb = heap.records().get(&ciff::Tag::ColourInfo1).and_then(|r| {
            if let ciff::Record::InHeap((pos, len)) = r.data {
                let mut view = container.borrow_view_mut();
                match len {
                    768 => {
                        // D30 values are RGGB
                        view.seek(SeekFrom::Start(pos as u64 + 72)).ok()?;
                        let mut wb_values = [0_u16; 4];
                        view.read_endian_u16_array(&mut wb_values, container.endian())
                            .ok()?;
                        let g = (1024.0 / wb_values[1] as f64 + 1024.0 / wb_values[2] as f64) / 2.0;
                        let wb = vec![
                            g / (1024.0 / wb_values[0] as f64),
                            1.0,
                            g / (1024.0 / wb_values[3] as f64),
                        ];
                        Some(wb)
                    }
                    769.. => {
                        // Pro, G6, S60, S70, S45, S50, G3, G5
                        let offset = self.crw_wb_offset();
                        // magic values from rawspeed if not the default offset.
                        // Pro, G6, S60, S70
                        let key = if offset != Self::CRW_WB_DEFAULT_OFFSET {
                            (0x410, 0x45f3)
                        } else {
                            (0_u16, 0_u16)
                        };

                        view.seek(SeekFrom::Start(pos as u64 + offset)).ok()?;
                        let mut wb_data = [0_u16; 3];
                        view.read_endian_u16_array(&mut wb_data, container.endian())
                            .ok();
                        let r = (wb_data[1] ^ key.1) as f64;
                        let g = (wb_data[0] ^ key.0) as f64;
                        let b = (wb_data[2] ^ key.0) as f64;
                        Some(vec![g / r, 1.0, g / b])
                    }
                    _ => None,
                }
            } else {
                None
            }
        });
        if wb.is_some() {
            return wb;
        }

        let wb_idx = heap.records().get(&ciff::Tag::ShotInfo).and_then(|r| {
            if let ciff::Record::InHeap((pos, _)) = r.data {
                let mut view = container.borrow_view_mut();
                view.seek(SeekFrom::Start(pos as u64 + 14)).ok()?;
                view.read_endian_u16(container.endian()).ok()
            } else {
                None
            }
        });
        // For D60, 10D, 300D. Need wb_idx.
        if let Some(wb_idx) = wb_idx {
            let r = heap.records().get(&ciff::Tag::WhiteBalanceTable)?;
            if let ciff::Record::InHeap((pos, _)) = r.data {
                // Some dark magic lifted from both rawspeed and dcraw
                let wb_offset = 1 + (b"0134567028"[wb_idx as usize] - b'0') as u64 * 4;
                let mut view = container.borrow_view_mut();
                view.seek(SeekFrom::Start(pos as u64 + wb_offset * 2))
                    .ok()?;
                let mut wb_data = [0_u16; 4];
                view.read_endian_u16_array(&mut wb_data, container.endian())
                    .ok()?;
                let g = wb_data[1] as f64;
                return Some(vec![g / wb_data[0] as f64, 1.0, g / wb_data[3] as f64]);
            };
        }
        None
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
                let sensor_info = self.get_sensor_info(heap);
                if let Some(ref sensor_info) = sensor_info {
                    cfa_x = sensor_info.0.width;
                    cfa_y = sensor_info.0.height;
                }
                let wb = self.get_white_balance(heap);
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
                                if let Some(wb) = wb {
                                    rawdata.set_as_shot_neutral(&wb);
                                }
                                rawdata.set_active_area(sensor_info.map(|v| v.1));
                                Ok(rawdata)
                            } else {
                                let mut decompressor =
                                    Decompress::new(decoder_table as usize, cfa_x, cfa_y);
                                let mut view = Viewer::create_subview(
                                    &container.borrow_view_mut(),
                                    pos as u64,
                                )?;
                                decompressor
                                    .decompress(&mut view)
                                    .map(|mut rawdata| {
                                        rawdata.set_whites([(1 << 10) - 1; 4]);
                                        rawdata.set_active_area(sensor_info.map(|v| v.1));
                                        if let Some(wb) = wb {
                                            rawdata.set_as_shot_neutral(&wb);
                                        }
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
