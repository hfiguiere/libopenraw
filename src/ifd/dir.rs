/*
 * libopenraw - ifd/dir.rs
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
use std::io::{Read, Seek, SeekFrom};

use byteorder::ReadBytesExt;
use log::debug;

use crate::container;
use crate::io::View;
use crate::Result;

use super::{Entry, Ifd, Type};

/// IFD
pub struct Dir {
    /// Endian for the IFD
    endian: container::Endian,
    /// Type of IFD
    type_: Type,
    /// All the IFD entries
    entries: HashMap<u16, Entry>,
    /// Position of the next IFD
    next: i32,
}

impl Dir {
    /// Read an IFD from the view, using endian `E`.
    pub(crate) fn read<E>(view: &mut View, dir_offset: i32, type_: Type) -> Result<Self>
    where
        E: container::EndianType,
    {
        let mut entries = HashMap::new();
        view.seek(SeekFrom::Start(dir_offset as u64))?;

        let num_entries = view.read_i16::<E>()?;
        for _ in 0..num_entries {
            let id = view.read_u16::<E>()?;
            let type_ = view.read_i16::<E>()?;
            let count = view.read_u32::<E>()?;
            let mut data = [0_u8; 4];
            view.read_exact(&mut data)?;
            debug!("Entry {:x} with type {} added", id, type_);
            let mut entry = Entry::new(id, type_, count, data);
            if !entry.is_inline() {
                let pos = view.seek(SeekFrom::Current(0))?;
                entry.load_data::<E>(view)?;
                view.seek(SeekFrom::Start(pos))?;
            }
            entries.insert(id, entry);
        }

        let next = view.read_i32::<E>()?;
        Ok(Dir {
            endian: E::endian(),
            type_,
            entries,
            next,
        })
    }

    /// Offset of the next IFD. 0 mean this was the last one.
    pub fn next_ifd(&self) -> i32 {
        self.next
    }
}

impl Ifd for Dir {
    fn ifd_type(&self) -> Type {
        self.type_
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