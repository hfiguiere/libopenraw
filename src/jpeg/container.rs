/*
 * libopenraw - jpeg/container.rs
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

//! JPEG container

use std::cell::{RefCell, RefMut};

use jpeg_decoder::{Decoder, ImageInfo};
use once_cell::unsync::OnceCell;

use crate::container;
use crate::io::View;

/// JFIF Container to just read a JPEG image.
pub(crate) struct Container {
    /// The `io::View`.
    view: RefCell<View>,
    /// JPEG image info
    image_info: OnceCell<Option<ImageInfo>>,
    /// JPEG decoder
    decoder: OnceCell<RefCell<Decoder<View>>>,
}

impl container::GenericContainer for Container {
    fn endian(&self) -> container::Endian {
        container::Endian::Big
    }

    fn borrow_view_mut(&self) -> RefMut<'_, View> {
        self.view.borrow_mut()
    }
}

impl Container {
    pub(crate) fn new(view: View) -> Self {
        Self {
            view: RefCell::new(view),
            image_info: OnceCell::new(),
            decoder: OnceCell::new(),
        }
    }

    /// Initialize the JPEG decoder.
    fn decoder(&self) -> &RefCell<Decoder<View>> {
        self.decoder.get_or_init(|| {
            let view = &*self.view.borrow_mut();
            RefCell::new(Decoder::new(view.clone()))
        })
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
