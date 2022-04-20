/*
 * libopenraw - canon/cr3.rs
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

//! Canon CR3 format, the 3rd generation of Canon RAW format, based on
//! the ISOMedia (MP4) container format.

use std::rc::Rc;

use once_cell::unsync::OnceCell;

use crate::camera_ids::{canon, vendor};
use crate::container::GenericContainer;
use crate::io::Viewer;
use crate::mp4;
use crate::rawfile::ReadAndSeek;
use crate::thumbnail;
use crate::tiff;
use crate::tiff::Dir;
use crate::{DataType, Dump, Error, RawData, RawFile, RawFileImpl, Rect, Result, Type, TypeId};

use crate::colour::BuiltinMatrix;

lazy_static::lazy_static! {
    static ref MATRICES: [BuiltinMatrix; 16] = [
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_M200),
      0,
      0,
      [ 10463, -2173, -1437, -4856, 12635, 2482, -1216, 2915, 7237 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_M50),
      0,
      0,
      [ 8532, -701, -1167, -4095, 11879, 2508, -797, 2424, 7010 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_M50MKII),
      0,
      0,
      [ 10463, -2173, -1437, -4856, 12635, 2482, -1216, 2915, 7237 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_M6MKII),
      0,
      0,
      [ 11498, -3759, -1516, -5073, 12954, 2349, -892, 1867, 6118 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_90D),
      0,
      0,
      [ 11498, -3759, -1516, -5073, 12954, 2349, -892, 1867, 6118 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_R),
      0,
      0,
      [ 6446, -366, -864, -4436, 12204, 2513, -952, 2496, 6348 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_RP),
      0,
      0,
      [ 8608, -2097, -1178, -5425, 13265, 2383, -1149, 2238, 5680 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_R3),
      0,
      0,
      [ 9423, -2839, -1195, -4532, 12377, 2415, -483, 1374, 5276 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_R5),
      0,
      0,
      [ 9766, -2953, -1254, -4276, 12116, 2433, -437, 1336, 5131 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_R6),
      0,
      0,
      [ 8293, -1611, -1132, -4759, 12711, 2275, -1013, 2415, 5509 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::SX70_HS),
      0,
      0,
      [ 18285, -8907, -1951, -1845, 10688, 1323, 364, 1101, 5139 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_250D),
      0,
      0,
      [ 9079, -1923, -1236, -4677, 12454, 2492, -922, 2319, 5565 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_850D),
      0,
      0,
      [ 9079, -1923, -1236, -4677, 12454, 2492, -922, 2319, 5565 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::G5XMKII),
      0,
      0,
      [ 11629, -5713, -914, -2706, 11090, 1842, -206, 1225, 5515 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::G7XMKIII),
      0,
      0,
      [ 11629, -5713, -914, -2706, 11090, 1842, -206, 1225, 5515 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_1DXMKIII),
      0,
      0,
      [ 8971, -2022, -1242, -5405, 13249, 2380, -1280, 2483, 6072 ] ),
    ];
}

/// Canon CR3 File
pub struct Cr3File {
    reader: Rc<Viewer>,
    container: OnceCell<mp4::Container>,
    thumbnails: OnceCell<Vec<(u32, thumbnail::ThumbDesc)>>,
}

impl Cr3File {
    pub fn factory(reader: Box<dyn ReadAndSeek>) -> Box<dyn RawFile> {
        let viewer = Viewer::new(reader, 0);
        Box::new(Cr3File {
            reader: viewer,
            container: OnceCell::new(),
            thumbnails: OnceCell::new(),
        })
    }
}

impl RawFileImpl for Cr3File {
    fn identify_id(&self) -> TypeId {
        if let Some(maker_note) = self.maker_note_ifd() {
            super::identify_from_maker_note(maker_note)
        } else {
            log::error!("MakerNote not found");
            TypeId(vendor::CANON, 0)
        }
    }

    /// Return a lazily loaded `mp4::Container`
    fn container(&self) -> &dyn GenericContainer {
        self.container.get_or_init(|| {
            // XXX we should be faillible here.
            let view = Viewer::create_view(&self.reader, 0).expect("Created view");
            let mut container = mp4::Container::new(view, self.type_());
            container.load().expect("MP4 container error");
            container
        })
    }

    /// Return a lazily loaded set of thumbnails
    fn thumbnails(&self) -> &Vec<(u32, thumbnail::ThumbDesc)> {
        self.thumbnails.get_or_init(|| {
            use thumbnail::{Data, DataOffset};

            self.container();
            let container = self.container.get().unwrap();
            let mut thumbnails = Vec::new();
            if let Ok(craw_header) = container.craw_header() {
                let x = craw_header.thumbnail.width;
                let y = craw_header.thumbnail.height;
                let dim = std::cmp::max(x, y) as u32;
                if dim > 0 {
                    let desc = thumbnail::ThumbDesc {
                        width: x as u32,
                        height: y as u32,
                        data_type: DataType::Jpeg,
                        data: Data::Bytes(craw_header.thumbnail.data.clone()),
                    };
                    log::debug!(
                        "Found thumbnail: {}x{} len: {}",
                        x,
                        y,
                        craw_header.thumbnail.data.len()
                    );
                    thumbnails.push((dim, desc));
                }
            }

            let track_count = container.track_count().unwrap_or(0);
            for i in 0..track_count {
                if !container.is_track_video(i).unwrap_or(false) {
                    continue;
                }
                if let Ok(raw_track) = container.raw_track(i) {
                    if raw_track.is_jpeg {
                        let dim =
                            std::cmp::max(raw_track.image_width, raw_track.image_height) as u32;
                        let desc = thumbnail::ThumbDesc {
                            width: raw_track.image_width as u32,
                            height: raw_track.image_height as u32,
                            data_type: DataType::Jpeg,
                            data: Data::Offset(DataOffset {
                                offset: raw_track.offset,
                                len: raw_track.len,
                            }),
                        };
                        log::debug!(
                            "Found thumbnail: {}x{} @{} len: {}",
                            raw_track.image_width,
                            raw_track.image_height,
                            raw_track.offset,
                            raw_track.len
                        );
                        thumbnails.push((dim, desc));
                    }
                }
            }

            if let Ok(desc) = container.preview_desc() {
                let dim = std::cmp::max(desc.width, desc.height);
                thumbnails.push((dim, desc));
            }
            thumbnails
        })
    }

    fn ifd(&self, ifd_type: tiff::Type) -> Option<Rc<Dir>> {
        self.container();
        let container = self.container.get().unwrap();
        match ifd_type {
            tiff::Type::Main => container.metadata_block(0).and_then(|c| c.1.directory(0)),
            tiff::Type::Exif => container.metadata_block(1).and_then(|c| c.1.directory(0)),
            tiff::Type::MakerNote => container.metadata_block(2).and_then(|c| c.1.directory(0)),
            _ => None,
        }
    }

    /// Load the RawData and return it.
    fn load_rawdata(&self) -> Result<RawData> {
        self.container();
        let container = self.container.get().unwrap();

        if !container.is_track_video(2).unwrap_or(false) {
            log::error!("Video track not found");
            return Err(Error::NotFound);
        }
        if let Ok(raw_track) = container.raw_track(2) {
            if raw_track.is_jpeg {
                log::error!("RAW track is JPEG");
                return Err(Error::NotFound);
            }

            let width = raw_track.image_width;
            let height = raw_track.image_height;
            let byte_len = raw_track.len;
            let offset = raw_track.offset;
            let data = container.load_buffer8(offset, byte_len);

            let mut rawdata = RawData::new8(
                width as u32,
                height as u32,
                8,
                DataType::CompressedRaw,
                data,
            );

            let sensor_info = self
                .maker_note_ifd()
                .and_then(super::SensorInfo::new)
                .map(|s| Rect {
                    x: s.0[0],
                    y: s.0[1],
                    width: s.0[2],
                    height: s.0[3],
                });
            rawdata.set_active_area(sensor_info);

            Ok(rawdata)
        } else {
            log::error!("Raw track not found");
            Err(Error::NotFound)
        }
    }

    fn get_builtin_colour_matrix(&self) -> Result<Vec<f64>> {
        MATRICES
            .iter()
            .find(|m| m.camera == self.type_id())
            .map(|m| Vec::from(m.matrix))
            .ok_or(Error::NotFound)
    }

    fn white(&self) -> u16 {
        MATRICES
            .iter()
            .find(|m| m.camera == self.type_id())
            .map(|m| m.white)
            .unwrap_or(0xffff)
    }

    fn black(&self) -> u16 {
        MATRICES
            .iter()
            .find(|m| m.camera == self.type_id())
            .map(|m| m.black)
            .unwrap_or(0)
    }
}

impl RawFile for Cr3File {
    fn type_(&self) -> Type {
        Type::Cr3
    }
}

impl Dump for Cr3File {
    #[cfg(feature = "dump")]
    fn print_dump(&self, indent: u32) {
        dump_println!(indent, "<Canon CR3 File>");
        // dump container
        {
            let indent = indent + 1;
            self.container().print_dump(indent);
        }
        dump_println!(indent, "</Canon CR3 File>");
    }
}
