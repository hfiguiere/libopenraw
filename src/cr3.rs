/*
 * libopenraw - cr3.rs
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

use std::collections::HashMap;
use std::rc::Rc;

use once_cell::unsync::OnceCell;

use crate::container::Container;
use crate::ifd;
use crate::io::Viewer;
use crate::mp4;
use crate::rawfile::ReadAndSeek;
use crate::thumbnail;
use crate::thumbnail::Thumbnail;
use crate::{DataType, Error, RawFile, RawFileImpl, Result, Type, TypeId};

/// Canon CR3 File
pub struct Cr3File {
    reader: Rc<Viewer>,
    container: OnceCell<mp4::Container>,
    thumbnails: OnceCell<HashMap<u32, thumbnail::ThumbDesc>>,
}

impl Cr3File {
    pub fn factory(reader: Box<dyn ReadAndSeek>) -> Box<dyn RawFile> {
        let viewer = Viewer::new(reader);
        Box::new(Cr3File {
            reader: viewer,
            container: OnceCell::new(),
            thumbnails: OnceCell::new(),
        })
    }

    /// Return a lazily loaded `mp4::Container`
    fn container(&self) -> &mp4::Container {
        self.container.get_or_init(|| {
            // XXX we should be faillible here.
            let view = Viewer::create_view(&self.reader, 0).expect("Created view");
            let mut container = mp4::Container::new(view);
            container.load().expect("MP4 container error");
            container
        })
    }

    /// Return a lazily loaded set of thumbnails
    fn thumbnails(&self) -> &HashMap<u32, thumbnail::ThumbDesc> {
        self.thumbnails.get_or_init(|| {
            use thumbnail::{Data, DataOffset};

            let container = self.container();
            let mut thumbnails = HashMap::new();
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
                    thumbnails.insert(dim, desc);
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
                        thumbnails.insert(dim, desc);
                    }
                }
            }

            if let Ok(desc) = container.preview_desc() {
                let dim = std::cmp::max(desc.width, desc.height);
                thumbnails.insert(dim, desc);
            }
            thumbnails
        })
    }
}

impl RawFileImpl for Cr3File {
    fn identify_id(&self) -> TypeId {
        0
    }

    fn thumbnail_for_size(&self, size: u32) -> Result<Thumbnail> {
        let thumbnails = self.thumbnails();
        if let Some(desc) = thumbnails.get(&size) {
            self.container().make_thumbnail(desc)
        } else {
            Err(Error::NotFound)
        }
    }

    fn list_thumbnail_sizes(&self) -> Vec<u32> {
        let thumbnails = self.thumbnails();

        // XXX shall we cache this?
        let mut sizes: Vec<u32> = thumbnails.keys().copied().collect();
        sizes.sort_unstable();
        sizes
    }

    fn ifd(&self, ifd_type: ifd::Type) -> Option<Rc<ifd::Dir>> {
        match ifd_type {
            ifd::Type::Main => self.container().metadata_block(0),
            ifd::Type::Exif => self.container().metadata_block(1),
            ifd::Type::MakerNote => self.container().metadata_block(2),
            _ => None,
        }
        .and_then(|c| c.1.directory(0))
    }
}

impl RawFile for Cr3File {
    fn type_(&self) -> Type {
        super::Type::Cr3
    }
}
