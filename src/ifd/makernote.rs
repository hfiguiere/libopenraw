/*
 * libopenraw - ifd/makernote.rs
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

use super::{Entry, Ifd, Type};

use crate::container;

pub struct MakerNote {
    /// Endian for the IFD
    endian: container::Endian,
    /// All the IFD entries
    entries: HashMap<u16, Entry>,
    /// The MakerNote ID
    id: String,
}

impl Ifd for MakerNote {
    fn ifd_type(&self) -> Type {
        Type::MakerNote
    }

    fn endian(&self) -> container::Endian {
        self.endian
    }

    /// Return the number of entries.
    fn num_entries(&self) -> usize {
        self.entries.len()
    }

    /// Return the entry for the `tag`.
    fn entry(&self, tag: u16) -> Option<&Entry> {
        self.entries.get(&tag)
    }
}
