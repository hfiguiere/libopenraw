// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - jpeg/container.rs
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

//! JPEG container

use std::cell::{RefCell, RefMut};
use std::rc::Rc;

use jpeg_decoder::{Decoder, ImageInfo};
use once_cell::unsync::OnceCell;

use crate::container;
use crate::io::{View, Viewer};
use crate::metadata;
use crate::tiff;
use crate::Dump;
use crate::Type as RawType;

/// JFIF Container to just read a JPEG image.
pub(crate) struct Container {
    /// The `io::View`.
    view: RefCell<View>,
    /// JPEG image info
    image_info: OnceCell<Option<ImageInfo>>,
    /// JPEG decoder
    decoder: OnceCell<RefCell<Decoder<View>>>,
    /// Exif IFD
    exif: OnceCell<Option<(tiff::Container, Rc<Viewer>)>>,
    /// The RawType this belong to
    raw_type: RawType,
}

impl container::RawContainer for Container {
    fn endian(&self) -> container::Endian {
        container::Endian::Big
    }

    fn borrow_view_mut(&self) -> RefMut<'_, View> {
        self.view.borrow_mut()
    }

    fn raw_type(&self) -> RawType {
        self.raw_type
    }

    fn dir_iterator(&self) -> metadata::Iterator {
        self.exif()
            .map(|exif| exif.dirs().iter().into())
            .unwrap_or_default()
    }
}

impl Container {
    pub(crate) fn new(view: View, raw_type: RawType) -> Self {
        Self {
            view: RefCell::new(view),
            image_info: OnceCell::new(),
            decoder: OnceCell::new(),
            exif: OnceCell::new(),
            raw_type,
        }
    }

    /// Initialize the JPEG decoder.
    fn decoder(&self) -> &RefCell<Decoder<View>> {
        self.decoder.get_or_init(|| {
            let view = &*self.view.borrow_mut();
            RefCell::new(Decoder::new(view.clone()))
        })
    }

    pub fn exif_dir(&self) -> Option<&tiff::Dir> {
        self.exif().and_then(|container| container.exif_dir())
    }

    pub fn mnote_dir(&self) -> Option<&tiff::Dir> {
        self.exif().and_then(|container| container.mnote_dir())
    }

    pub fn exif(&self) -> Option<&tiff::Container> {
        self.exif
            .get_or_init(|| {
                let decoder = self.decoder();
                decoder
                    .borrow_mut()
                    .read_info()
                    .map_err(|err| {
                        log::error!("JPEG decoding error: {}", err);
                        err
                    })
                    .ok()?;
                decoder
                    .borrow()
                    .exif_data()
                    .and_then(|data| {
                        let data = Vec::from(data);
                        let length = data.len();
                        let io = Box::new(std::io::Cursor::new(data));
                        let viewer = Viewer::new(io, length as u64);
                        let view = Viewer::create_view(&viewer, 0)
                            .map_err(|err| {
                                log::error!("Failed to create view {}", err);
                                err
                            })
                            .ok()?;
                        let mut exif = tiff::Container::new(
                            view,
                            vec![tiff::IfdType::Main, tiff::IfdType::Other],
                            self.raw_type,
                        );
                        exif.load(None)
                            .map(|_| (exif, viewer))
                            .map_err(|err| {
                                log::warn!("Failed to load Exif: {}", err);
                                err
                            })
                            .ok()
                    })
                    .or_else(|| {
                        log::warn!("Error loading exif (likely there is none)");
                        None
                    })
            })
            .as_ref()
            .map(|m| &m.0)
    }

    /// Load the image info.
    fn image_info(&self) -> &Option<ImageInfo> {
        self.image_info.get_or_init(|| {
            let decoder = self.decoder();
            decoder
                .borrow_mut()
                .read_info()
                .map_err(|err| {
                    log::error!("JPEG decoding error: {}", err);
                    err
                })
                .ok()?;
            decoder.borrow().info()
        })
    }

    /// Return the height of the JPEG image.
    pub fn height(&self) -> u16 {
        if let Some(info) = self.image_info() {
            info.height
        } else {
            0
        }
    }

    /// Return the width of the JPEG image.
    pub fn width(&self) -> u16 {
        if let Some(info) = self.image_info() {
            info.width
        } else {
            0
        }
    }
}

impl Dump for Container {
    #[cfg(feature = "dump")]
    fn write_dump<W: std::io::Write + ?Sized>(&self, out: &mut W, indent: u32) {
        dump_writeln!(
            out,
            indent,
            "<JPEG Container @{}>",
            self.view.borrow().offset()
        );
        {
            let indent = indent + 1;
            dump_writeln!(
                out,
                indent,
                "Width = {} Height = {}",
                self.width(),
                self.height()
            );
            if let Some(exif) = self.exif() {
                dump_writeln!(out, indent, "Exif: ");
                exif.write_dump(out, indent);
            } else {
                dump_writeln!(out, indent, "No Exif");
            }
        }
        dump_writeln!(out, indent, "</JPEG Container>");
    }
}
