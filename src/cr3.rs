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

use std::rc::Rc;

use crate::io::Viewer;
use crate::mp4;
use crate::thumbnail::Thumbnail;

use super::rawfile::ReadAndSeek;
use super::{Error, RawFile, RawFileImpl, Result, Type, TypeId};

/// Canon CR3 File
pub struct Cr3File {
    reader: Rc<Viewer>,
    container: mp4::Container,
}

impl Cr3File {
    pub fn factory(reader: Box<dyn ReadAndSeek>) -> Box<dyn RawFile> {
        let viewer = Viewer::new(reader);
        // XXX we should be faillible here.
        let view = Viewer::create_view(&viewer, 0).expect("Created view");
        let container = mp4::Container::new(view);
        Box::new(Cr3File {
            reader: viewer,
            container,
        })
    }
}

impl RawFileImpl for Cr3File {
    fn identify_id(&self) -> TypeId {
        0
    }

    fn thumbnail_for_size(&self, _size: u32) -> Result<Thumbnail> {
        Err(Error::NotSupported)
    }

    fn list_thumbnail_sizes(&self) -> Vec<u32> {
        vec![]
    }
}

impl RawFile for Cr3File {
    fn type_(&self) -> Type {
        super::Type::Cr3
    }
}
