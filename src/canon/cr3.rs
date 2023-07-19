// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - canon/cr3.rs
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

//! Canon CR3 format, the 3rd generation of Canon RAW format, based on
//! the ISOMedia (MP4) container format.
//!
#![doc = include_str!("../../doc/cr3.md")]

use std::rc::Rc;

use once_cell::unsync::OnceCell;

use crate::canon;
use crate::container::RawContainer;
use crate::io::Viewer;
use crate::mosaic::Pattern;
use crate::mp4;
use crate::rawfile::ReadAndSeek;
use crate::rawfile::ThumbnailStorage;
use crate::thumbnail;
use crate::tiff;
use crate::tiff::Dir;
use crate::{DataType, Dump, Error, RawData, RawFile, RawFileImpl, Rect, Result, Type, TypeId};

use super::matrices::MATRICES;

/// Canon CR3 File
pub struct Cr3File {
    reader: Rc<Viewer>,
    type_id: OnceCell<TypeId>,
    container: OnceCell<mp4::Container>,
    thumbnails: OnceCell<ThumbnailStorage>,
}

impl Cr3File {
    pub fn factory(reader: Box<dyn ReadAndSeek>) -> Box<dyn RawFile> {
        let viewer = Viewer::new(reader, 0);
        Box::new(Cr3File {
            reader: viewer,
            type_id: OnceCell::new(),
            container: OnceCell::new(),
            thumbnails: OnceCell::new(),
        })
    }
}

impl RawFileImpl for Cr3File {
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

    /// Returns a lazily loaded [`mp4::Container`].
    fn container(&self) -> &dyn RawContainer {
        self.container.get_or_init(|| {
            // XXX we should be faillible here.
            let view = Viewer::create_view(&self.reader, 0).expect("Created view");
            let mut container = mp4::Container::new(view, self.type_());
            container.load().expect("MP4 container error");
            container
        })
    }

    /// Returns a lazily loaded set of [thumbnails][thumbnail::ThumbDesc].
    fn thumbnails(&self) -> &ThumbnailStorage {
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
            ThumbnailStorage::with_thumbnails(thumbnails)
        })
    }

    fn ifd(&self, ifd_type: tiff::IfdType) -> Option<&Dir> {
        self.container();
        let container = self.container.get().unwrap();
        match ifd_type {
            tiff::IfdType::Main => container.metadata_block(0).and_then(|c| c.1.directory(0)),
            tiff::IfdType::Exif => container.metadata_block(1).and_then(|c| c.1.directory(0)),
            tiff::IfdType::MakerNote => container.metadata_block(2).and_then(|c| c.1.directory(0)),
            _ => None,
        }
    }

    /// Load the [`RawData`] and return it.
    fn load_rawdata(&self, _skip_decompression: bool) -> Result<RawData> {
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
                Pattern::default(),
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
}

impl RawFile for Cr3File {
    fn type_(&self) -> Type {
        Type::Cr3
    }
}

impl Dump for Cr3File {
    #[cfg(feature = "dump")]
    fn write_dump<W: std::io::Write + ?Sized>(&self, out: &mut W, indent: u32) {
        dump_writeln!(out, indent, "<Canon CR3 File>");
        // dump container
        {
            let indent = indent + 1;
            self.container();
            self.container.get().unwrap().write_dump(out, indent);
        }
        dump_writeln!(out, indent, "</Canon CR3 File>");
    }
}

dumpfile_impl!(Cr3File);
