// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - jpeg.rs
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

//! JPEG support, except ljpeg.

mod container;

pub(crate) use container::Container;

use std::rc::Rc;

use once_cell::unsync::OnceCell;

use crate::camera_ids::vendor;
use crate::container::RawContainer;
use crate::io::Viewer;
use crate::rawfile::ThumbnailStorage;
use crate::thumbnail;
use crate::tiff;
use crate::tiff::{exif, Dir, Ifd};
use crate::{DataType, Dump, Error, RawFile, RawFileImpl, RawImage, Result, Type, TypeId};

#[derive(Debug)]
/// JPEG file
pub(crate) struct JpegFile {
    reader: Rc<Viewer>,
    type_id: OnceCell<TypeId>,
    container: OnceCell<Container>,
    thumbnails: OnceCell<ThumbnailStorage>,
}

impl JpegFile {
    pub(crate) fn factory(reader: Rc<Viewer>) -> Rc<dyn RawFile> {
        Rc::new(JpegFile {
            reader,
            type_id: OnceCell::new(),
            container: OnceCell::new(),
            thumbnails: OnceCell::new(),
        })
    }
}

impl RawFileImpl for JpegFile {
    fn identify_id(&self) -> TypeId {
        *self.type_id.get_or_init(|| TypeId(vendor::JPEG, 0))
    }

    fn container(&self) -> &dyn RawContainer {
        self.container.get_or_init(|| {
            // XXX we should be faillible here.
            let view = Viewer::create_view(&self.reader, 0).expect("Created view");
            Container::new(view, Type::Jpeg)
        })
    }

    fn thumbnails(&self) -> &ThumbnailStorage {
        self.thumbnails.get_or_init(|| {
            let mut thumbnails = vec![];
            self.container();
            let container = self.container.get().unwrap();
            container.exif().and_then(|exif| {
                let dir = exif.directory(1)?;
                let len = dir.value::<u32>(exif::EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LENGTH)? as u64;
                let offset = dir.value::<u32>(exif::EXIF_TAG_JPEG_INTERCHANGE_FORMAT)? as u64;
                // XXX this +12 should be "calculated"
                let offset = offset + 12;
                // XXX as a shortcut we assume it's Exif 160x120
                thumbnails.push((
                    160,
                    thumbnail::ThumbDesc {
                        width: 160,
                        height: 120,
                        data_type: DataType::Jpeg,
                        data: thumbnail::Data::Offset(thumbnail::DataOffset { offset, len }),
                    },
                ));

                Some(())
            });
            ThumbnailStorage::with_thumbnails(thumbnails)
        })
    }

    fn ifd(&self, ifd_type: tiff::IfdType) -> Option<&Dir> {
        self.container();
        let container = self.container.get().unwrap();
        match ifd_type {
            tiff::IfdType::Main | tiff::IfdType::Raw => None,
            tiff::IfdType::Exif => container.exif().and_then(|exif| exif.directory(0)),
            tiff::IfdType::MakerNote => container.exif().and_then(|exif| exif.mnote_dir()),
            _ => None,
        }
    }

    fn load_rawdata(&self, _: bool) -> Result<RawImage> {
        Err(Error::NotSupported)
    }

    fn get_builtin_colour_matrix(&self) -> Result<Vec<f64>> {
        Err(Error::NotSupported)
    }
}

impl RawFile for JpegFile {
    fn type_(&self) -> Type {
        Type::Jpeg
    }
}

impl Dump for JpegFile {
    #[cfg(feature = "dump")]
    fn write_dump<W: std::io::Write + ?Sized>(&self, out: &mut W, indent: u32) {
        dump_writeln!(out, indent, "<JPEG File>");
        {
            let indent = indent + 1;
            self.container();
            self.container.get().unwrap().write_dump(out, indent);
        }
        dump_writeln!(out, indent, "</JPEG File>");
    }
}

dumpfile_impl!(JpegFile);
