/*
 * libopenraw - tiff/dir.rs
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

//! Image File Directory is the main data structure of TIFF used by Exif
//! and most RAW format.

use std::collections::HashMap;
use std::io::{Read, Seek, SeekFrom};
use std::rc::Rc;

use byteorder::{BigEndian, LittleEndian, ReadBytesExt};
use log::debug;

use crate::canon;
use crate::container;
use crate::container::GenericContainer;
use crate::epson;
use crate::io::View;
use crate::sony;
use crate::tiff;
use crate::tiff::exif;
use crate::Type as RawType;
use crate::{Error, Result};

use super::{Entry, Ifd, Type};

lazy_static::lazy_static! {
    /// Empty tag list
    static ref MNOTE_EMPTY_TAGS: HashMap<u16, &'static str> = HashMap::new();
}

/// IFD
/// Also handle MakerNotes
pub struct Dir {
    /// Endian for the IFD
    endian: container::Endian,
    /// Type of IFD
    type_: Type,
    /// All the IFD entries
    entries: HashMap<u16, Entry>,
    /// Position of the next IFD
    next: u32,
    /// The MakerNote ID
    id: String,
    /// Offset in MakerNote
    mnote_offset: u64,
    /// Tag names to decode.
    tag_names: &'static HashMap<u16, &'static str>,
}

impl Dir {
    pub(crate) fn create_maker_note(
        container: &dyn container::GenericContainer,
        offset: u32,
        file_type: RawType,
    ) -> Result<Rc<Dir>> {
        match file_type {
            RawType::Cr2 | RawType::Cr3 | RawType::Crw => {
                return Dir::new_makernote("Canon", container, offset, 0, &canon::MNOTE_TAG_NAMES)
            }
            RawType::Arw => {
                return Dir::new_makernote("Sony5", container, offset, 0, &sony::MNOTE_TAG_NAMES)
            }
            _ => {
                let mut data = [0_u8; 8];
                {
                    let mut view = container.borrow_view_mut();
                    view.seek(SeekFrom::Start(offset as u64))?;
                    view.read_exact(&mut data)?;
                }
                // XXX missing other

                // EPSON R-D1, use Olympus
                // XXX deal with endian.
                if &data[0..6] == b"EPSON\0" {
                    return Dir::new_makernote(
                        "Epson",
                        container,
                        offset + 8,
                        0,
                        &epson::MNOTE_TAG_NAMES,
                    );
                }
            }
        }
        Dir::new_makernote("", container, offset, 0, &MNOTE_EMPTY_TAGS)
    }

    ///
    pub(crate) fn new_makernote(
        id: &str,
        container: &dyn container::GenericContainer,
        offset: u32,
        mnote_offset: u64,
        tag_names: &'static HashMap<u16, &'static str>,
    ) -> Result<Rc<Dir>> {
        if let Ok(mut dir) = match container.endian() {
            container::Endian::Little => {
                let mut view = container.borrow_view_mut();
                Dir::read::<LittleEndian>(&mut view, offset, Type::MakerNote)
            }
            container::Endian::Big => {
                let mut view = container.borrow_view_mut();
                Dir::read::<BigEndian>(&mut view, offset, Type::MakerNote)
            }
            _ => {
                log::error!("Endian unset to read directory");
                Err(Error::NotFound)
            }
        } {
            dir.id = id.to_string();
            dir.mnote_offset = mnote_offset;
            dir.tag_names = tag_names;
            Ok(Rc::new(dir))
        } else {
            Err(Error::NotFound)
        }
    }

    /// Read an IFD from the view, using endian `E`.
    pub(crate) fn read<E>(view: &mut View, dir_offset: u32, type_: Type) -> Result<Self>
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
            if type_ == exif::TagType::Undefined as i16 {
                let offset = data.as_slice().read_u32::<E>()?;
                entry.set_offset(offset);
            } else if !entry.is_inline() {
                let pos = view.seek(SeekFrom::Current(0))?;
                entry.load_data::<E>(view)?;
                view.seek(SeekFrom::Start(pos))?;
            }
            entries.insert(id, entry);
        }

        let next = view.read_u32::<E>()?;
        Ok(Dir {
            endian: E::endian(),
            type_,
            entries,
            next,
            id: String::new(),
            mnote_offset: 0,
            tag_names: &exif::TAG_NAMES,
        })
    }

    pub fn id(&self) -> &str {
        &self.id
    }

    /// Offset of the next IFD. 0 mean this was the last one.
    pub fn next_ifd(&self) -> u32 {
        self.next
    }

    /// Get and unsigned integer that could be either size.
    pub fn uint_value(&self, tag: u16) -> Option<u32> {
        use exif::TagType::*;
        self.entry(tag).and_then(|e| {
            exif::TagType::try_from(e.type_)
                .ok()
                .and_then(|typ| match typ {
                    Short => self.entry_value::<u16>(e, 0).map(|v| v as u32),
                    Long => self.entry_value::<u32>(e, 0),
                    _ => {
                        log::warn!("Entry {} has wrong type {}", tag, e.type_);
                        None
                    }
                })
        })
    }

    /// Whether the IFD is primary
    pub fn is_primary(&self) -> bool {
        if let Some(v) = self.value::<u32>(exif::EXIF_TAG_NEW_SUBFILE_TYPE) {
            v == 0
        } else {
            false
        }
    }

    /// Get sub IFDs.
    pub(crate) fn get_sub_ifds(&self, container: &tiff::Container) -> Option<Vec<Rc<Dir>>> {
        let entry = self.entry(exif::EXIF_TAG_SUB_IFDS)?;
        let offsets = entry.value_array::<u32>(self.endian())?;

        let mut ifds = Vec::new();
        for offset in offsets {
            if let Ok(dir) = match self.endian() {
                container::Endian::Little => {
                    let mut view = container.borrow_view_mut();
                    Dir::read::<LittleEndian>(&mut view, offset, Type::SubIfd)
                }
                container::Endian::Big => {
                    let mut view = container.borrow_view_mut();
                    Dir::read::<BigEndian>(&mut view, offset, Type::SubIfd)
                }
                _ => {
                    log::error!("Endian unset to read directory");
                    Err(Error::NotFound)
                }
            } {
                ifds.push(Rc::new(dir));
            };
        }
        Some(ifds)
    }

    /// Return the cloned entry for the `tag`.
    /// This is not just a clone, it allow also making sure the data
    /// in self contained.
    /// If the data can't be loaded (error), `None` is returned.
    pub(crate) fn entry_cloned(&self, tag: u16, view: &mut View) -> Option<Entry> {
        self.entries.get(&tag).and_then(|e| {
            let mut e = (*e).clone();
            if e.offset().is_some() {
                match self.endian() {
                    container::Endian::Little => {
                        e.load_data::<LittleEndian>(view).ok()?;
                    }
                    container::Endian::Big => {
                        e.load_data::<BigEndian>(view).ok()?;
                    }
                    _ => {
                        log::error!("Endian unset to read Entry");
                        return None;
                    }
                }
            }

            Some(e)
        })
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
