// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - tiff/dir.rs
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

//! Image File Directory is the main data structure of TIFF used by Exif
//! and most RAW format.

use std::collections::{BTreeMap, HashMap};
use std::io::{Read, Seek, SeekFrom};

use byteorder::{BigEndian, LittleEndian, ReadBytesExt};
use log::debug;
use once_cell::unsync::OnceCell;

use crate::apple;
use crate::canon;
use crate::container::{self, RawContainer};
use crate::epson;
use crate::fujifilm;
use crate::io::View;
use crate::leica;
use crate::minolta;
use crate::nikon;
use crate::olympus;
use crate::panasonic;
use crate::pentax;
use crate::ricoh;
use crate::sigma;
use crate::sony;
use crate::tiff;
use crate::tiff::exif;
#[cfg(feature = "dump")]
use crate::Dump;
use crate::Type as RawType;
use crate::{Error, Result};

use super::{Entry, Ifd, IfdType};

lazy_static::lazy_static! {
    /// Empty tag list
    static ref MNOTE_EMPTY_TAGS: HashMap<u16, &'static str> = HashMap::new();
}

/// IFD
/// Also handle MakerNotes
pub struct Dir {
    /// Endian for the IFD
    pub(crate) endian: container::Endian,
    /// Type of IFD
    pub(crate) type_: IfdType,
    /// All the IFD entries
    pub(crate) entries: BTreeMap<u16, Entry>,
    /// Position of the next IFD
    next: u32,
    /// The MakerNote ID
    id: String,
    /// Offset in MakerNote
    pub mnote_offset: u32,
    /// Tag names to decode.
    pub(crate) tag_names: &'static HashMap<u16, &'static str>,
    /// SubIFs
    sub_ifds: OnceCell<Vec<Dir>>,
}

impl Dir {
    pub(crate) fn iter(&self) -> super::Iterator {
        super::Iterator::new(self)
    }

    pub(crate) fn create_maker_note(
        container: &dyn container::RawContainer,
        offset: u32,
    ) -> Result<Dir> {
        let file_type = container.raw_type();
        match file_type {
            RawType::Cr2 | RawType::Cr3 | RawType::Crw => {
                return Dir::new_makernote("Canon", container, offset, 0, &canon::MNOTE_TAG_NAMES)
            }
            RawType::Arw => {
                return Dir::new_makernote("Sony5", container, offset, 0, &sony::MNOTE_TAG_NAMES)
            }
            _ => {
                // The size of this buffer should be adjusted depending on
                // what the various detection need.
                let mut data = [0_u8; 16];
                {
                    let mut view = container.borrow_view_mut();
                    view.seek(SeekFrom::Start(offset as u64))?;
                    view.read_exact(&mut data)?;
                }

                if &data[0..6] == b"Nikon\0" {
                    if data[6] == 1 {
                        return Dir::new_makernote(
                            "Nikon2",
                            container,
                            offset + 8,
                            offset + 8,
                            &nikon::MNOTE_TAG_NAMES_2,
                        );
                    } else if data[6] == 2 {
                        // this one has an endian / TIFF header after the magic
                        return Dir::new_makernote(
                            "Nikon",
                            container,
                            offset + 18,
                            offset + 10,
                            &nikon::MNOTE_TAG_NAMES,
                        );
                    } else {
                        log::error!("Unidentified Nikon makernote");
                        return Dir::new_makernote(
                            "",
                            container,
                            offset,
                            offset,
                            &MNOTE_EMPTY_TAGS,
                        );
                    }
                }
                // Headerless Nikon.
                if file_type == RawType::Nef {
                    // Found on D1, D1X, D1H
                    return Dir::new_makernote(
                        "Nikon (Headerless)",
                        container,
                        offset,
                        offset,
                        &nikon::MNOTE_TAG_NAMES,
                    );
                }

                if &data[0..8] == b"OLYMPUS\0" {
                    return Dir::new_makernote(
                        "Olympus2",
                        container,
                        offset + 12,
                        offset,
                        &olympus::MNOTE_TAG_NAMES,
                    );
                }

                if &data[0..9] == b"OM SYSTEM" {
                    return Dir::new_makernote(
                        "Olympus2",
                        container,
                        offset + 16,
                        offset,
                        &olympus::MNOTE_TAG_NAMES,
                    );
                }

                if &data[0..6] == b"OLYMP\0" {
                    return Dir::new_makernote(
                        "Olympus",
                        container,
                        offset + 8,
                        0,
                        &olympus::MNOTE_TAG_NAMES,
                    );
                }

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

                // Pentax Asahi Optical Corporation (pre Ricoh merger)
                if &data[0..4] == b"AOC\0" {
                    return Dir::new_makernote(
                        "Pentax",
                        container,
                        offset + 6,
                        0,
                        &pentax::MNOTE_TAG_NAMES,
                    );
                }

                // Pentax post Ricoh merger
                if &data[0..8] == b"PENTAX \0" {
                    return Dir::new_makernote(
                        "Pentax",
                        container,
                        offset + 10,
                        offset,
                        &pentax::MNOTE_TAG_NAMES,
                    );
                }

                // Panasonic
                if &data[0..10] == b"Panasonic\0" {
                    return Dir::new_makernote(
                        "Panasonic",
                        container,
                        offset + 12,
                        0,
                        &panasonic::MNOTE_TAG_NAMES,
                    );
                }

                if &data[0..6] == b"Ricoh\0" {
                    return Dir::new_makernote(
                        "Ricoh",
                        container,
                        offset + 8,
                        0,
                        &ricoh::MNOTE_TAG_NAMES,
                    );
                }

                if &data[0..16] == b"LEICA CAMERA AG\0" && file_type == RawType::Rw2 {
                    // Rebadged Panasonic
                    // Leica C-Lux
                    // Leica V-Lux 5
                    // Leica D-Lux 7
                    return Dir::new_makernote(
                        "Panasonic",
                        container,
                        offset + 18,
                        0,
                        &panasonic::MNOTE_TAG_NAMES,
                    );
                }

                if &data[0..5] == b"LEICA" {
                    if &data[5..8] == b"\0\0\0" {
                        if file_type == RawType::Rw2 {
                            // Panasonic
                            return Dir::new_makernote(
                                "Panasonic",
                                container,
                                offset + 8,
                                0,
                                &panasonic::MNOTE_TAG_NAMES,
                            );
                        } else {
                            // Leica M8
                            return Dir::new_makernote(
                                "Leica2",
                                container,
                                offset + 8,
                                offset,
                                &leica::MNOTE_TAG_NAMES_2,
                            );
                        }
                    }

                    if data[5] == 0 && data[7] == 0 {
                        match data[6] {
                            0x08 | 0x09 =>
                            // Leica Q Typ 116 and SL (Type 601)
                                return Dir::new_makernote(
                                    "Leica5",
                                    container,
                                    offset + 8, 0,
                                    &leica::MNOTE_TAG_NAMES_5,
                                ),
                            0x01 | // Leica X1
                            0x04 | // Leica X VARIO
                            0x05 | // Leica X2
                            0x06 | // Leica T (Typ 701)
                            0x07 | // Leica X (Typ 113)
                            0x10 | // Leica X-U (Typ 113)
                            0x1a =>
                                return Dir::new_makernote(
                                    "Leica5",
                                    container,
                                    offset + 8,
                                    offset,
                                    &leica::MNOTE_TAG_NAMES_5,
                                ),
                            _ => {}
                        }
                    }

                    // Leica M (Typ 240)
                    if data[5] == 0x0 && data[6] == 0x02 && data[7] == 0xff {
                        return Dir::new_makernote(
                            "Leica6",
                            container,
                            offset + 8,
                            0,
                            &leica::MNOTE_TAG_NAMES_6,
                        );
                    }

                    // Leica M9/Monochrom
                    if data[5] == b'0' && data[6] == 0x03 && data[7] == 0 {
                        return Dir::new_makernote(
                            "Leica4",
                            container,
                            offset + 8,
                            offset,
                            &leica::MNOTE_TAG_NAMES_4,
                        );
                    }

                    // Leica M10
                    if data[5] == 0 && data[6] == 0x02 && data[7] == 0 {
                        return Dir::new_makernote(
                            "Leica9",
                            container,
                            offset + 8,
                            0,
                            &leica::MNOTE_TAG_NAMES_9,
                        );
                    }
                }

                if &data[0..8] == b"YI     \0" {
                    return Dir::new_makernote(
                        "Xiaoyi",
                        container,
                        offset + 12,
                        offset,
                        // XXX we have no idea.
                        &MNOTE_EMPTY_TAGS,
                    );
                }

                if &data[0..10] == b"Apple iOS\0" {
                    return Dir::new_makernote(
                        "Apple",
                        container,
                        offset + 14,
                        offset,
                        &apple::MNOTE_TAG_NAMES,
                    );
                }

                if &data[0..4] == b"STMN" {
                    return if &data[8..12] == b"\0\0\0\0" {
                        Dir::new_makernote(
                            "Samsung1a",
                            container,
                            offset,
                            offset,
                            &MNOTE_EMPTY_TAGS,
                        )
                    } else {
                        Dir::new_makernote(
                            "Samsung1b",
                            container,
                            offset,
                            offset,
                            &MNOTE_EMPTY_TAGS,
                        )
                    };
                }

                if &data[0..8] == b"FUJIFILM" {
                    return Dir::new_makernote(
                        "Fujifilm",
                        container,
                        offset + 12,
                        offset,
                        &fujifilm::MNOTE_TAG_NAMES,
                    );
                }

                if &data[0..6] == b"SIGMA\0" {
                    return Dir::new_makernote(
                        "Sigma",
                        container,
                        offset + 10,
                        0,
                        &sigma::MNOTE_TAG_NAMES,
                    );
                }

                if &data[10..14] == b"MLT0" {
                    return Dir::new_makernote(
                        "Minolta",
                        container,
                        offset,
                        offset,
                        &minolta::MNOTE_TAG_NAMES,
                    );
                }

                // Others

                // SilverFast scanner. (Plustek)
                // Doesn't follow the standard.
                if &data[0..5] == b"LSI1\0" {
                    return Err(Error::InvalidFormat);
                }
                // Google Pixel
                // BLOB
                if &data[0..4] == b"HDRP" {
                    return Err(Error::InvalidFormat);
                }
            }
        }
        Dir::new_makernote("", container, offset, 0, &MNOTE_EMPTY_TAGS)
    }

    ///
    pub(crate) fn new_makernote(
        id: &str,
        container: &dyn container::RawContainer,
        offset: u32,
        mnote_offset: u32,
        tag_names: &'static HashMap<u16, &'static str>,
    ) -> Result<Dir> {
        if let Ok(mut dir) = match container.endian() {
            container::Endian::Little => {
                let mut view = container.borrow_view_mut();
                Dir::read::<LittleEndian>(&mut view, offset, mnote_offset, IfdType::MakerNote)
            }
            container::Endian::Big => {
                let mut view = container.borrow_view_mut();
                Dir::read::<BigEndian>(&mut view, offset, mnote_offset, IfdType::MakerNote)
            }
            _ => {
                log::error!("Endian unset to read directory");
                Err(Error::NotFound)
            }
        } {
            dir.id = id.to_string();
            dir.mnote_offset = mnote_offset;
            dir.tag_names = tag_names;
            Ok(dir)
        } else {
            Err(Error::NotFound)
        }
    }

    /// Read an IFD from the view, using endian `E`.
    /// `base_offset` is the offset for reading data, like it is used in
    /// MakerNote. 0 is the default.
    pub(crate) fn read<E>(
        view: &mut View,
        dir_offset: u32,
        base_offset: u32,
        type_: IfdType,
    ) -> Result<Self>
    where
        E: container::EndianType,
    {
        let mut entries = BTreeMap::new();
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
                let pos = view.stream_position()?;
                let r = entry.load_data::<E>(base_offset, view);
                view.seek(SeekFrom::Start(pos))?;
                if r.is_err() {
                    // We'll just stop parsing.
                    // This is encountered on Somy A100 and is likely
                    // a bug, but it's also in the C++ code.
                    break;
                }
            }
            entries.insert(id, entry);
        }

        let next = view.read_u32::<E>()?;
        Ok(Dir {
            endian: E::ENDIAN,
            type_,
            entries,
            next,
            id: String::new(),
            mnote_offset: base_offset,
            tag_names: &exif::TAG_NAMES,
            sub_ifds: OnceCell::new(),
        })
    }

    pub(crate) fn new(endian: container::Endian, type_: IfdType) -> Self {
        Dir {
            endian,
            type_,
            entries: BTreeMap::new(),
            id: String::new(),
            next: 0,
            mnote_offset: 0,
            tag_names: &exif::TAG_NAMES,
            sub_ifds: OnceCell::new(),
        }
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
        self.entry(tag).and_then(|e| match self.endian() {
            container::Endian::Little => e.uint_value::<LittleEndian>(),
            container::Endian::Big => e.uint_value::<BigEndian>(),
            _ => {
                log::error!("Endian unset to read directory");
                None
            }
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

    /// Get the Exif IFD from the directory
    pub(crate) fn get_exif_ifd(&self, container: &tiff::Container) -> Option<Dir> {
        self.value::<u32>(exif::EXIF_TAG_EXIF_IFD_POINTER)
            .and_then(|offset| {
                let mut view = container.borrow_view_mut();
                container
                    .dir_at(&mut view, offset, IfdType::Exif)
                    .map_err(|e| {
                        log::warn!("Coudln't get exif dir at {}: {}", offset, e);
                        e
                    })
                    .ok()
            })
    }

    /// Get the MakerNote IFD from the directory
    pub(crate) fn get_mnote_ifd(&self, container: &tiff::Container) -> Option<Dir> {
        self.entry(exif::EXIF_TAG_MAKER_NOTE)
            .and_then(|e| e.offset())
            .and_then(|offset| {
                Dir::create_maker_note(container, offset)
                    .map_err(|e| {
                        log::warn!("Coudln't create maker_note: {}", e);
                        e
                    })
                    .ok()
            })
    }

    /// Get the IFD stored in the entry
    pub(crate) fn ifd_in_entry(&self, container: &tiff::Container, tag: u16) -> Option<Dir> {
        self.entry(tag).and_then(|e| {
            if e.type_ == 13 || e.type_ == exif::TagType::Long as i16 {
                match self.endian() {
                    container::Endian::Little => e.value_untyped::<u32, LittleEndian>(),
                    container::Endian::Big => e.value_untyped::<u32, BigEndian>(),
                    _ => {
                        log::error!("Endian unset to read directory");
                        return None;
                    }
                }
                .map(|offset| (offset as i64 + self.mnote_offset as i64) as u32)
            } else {
                e.offset()
            }
            .and_then(|val_offset| {
                let mut view = container.borrow_view_mut();
                container.dir_at(&mut view, val_offset, IfdType::Other).ok()
            })
        })
    }

    /// Get sub IFDs. The vec is empty if none is found.
    pub(crate) fn get_sub_ifds(&self, container: &tiff::Container) -> &Vec<Dir> {
        self.sub_ifds.get_or_init(|| {
            let mut ifds = Vec::new();
            if let Some(offsets) = self
                .entry(exif::EXIF_TAG_SUB_IFDS)
                .and_then(|entry| entry.value_array::<u32>(self.endian()))
            {
                for offset in offsets {
                    if offset == 0 {
                        log::warn!("SubIFD with offset 0");
                        continue;
                    }
                    if let Ok(dir) = match self.endian() {
                        container::Endian::Little => {
                            let mut view = container.borrow_view_mut();
                            Dir::read::<LittleEndian>(
                                &mut view,
                                offset,
                                self.mnote_offset,
                                IfdType::SubIfd,
                            )
                        }
                        container::Endian::Big => {
                            let mut view = container.borrow_view_mut();
                            Dir::read::<BigEndian>(
                                &mut view,
                                offset,
                                self.mnote_offset,
                                IfdType::SubIfd,
                            )
                        }
                        _ => {
                            log::error!("Endian unset to read directory");
                            Err(Error::NotFound)
                        }
                    } {
                        ifds.push(dir);
                    };
                }
            }
            ifds
        })
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
                        e.load_data::<LittleEndian>(self.mnote_offset, view).ok()?;
                    }
                    container::Endian::Big => {
                        e.load_data::<BigEndian>(self.mnote_offset, view).ok()?;
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
    fn ifd_type(&self) -> IfdType {
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

#[cfg(feature = "dump")]
impl Dump for Dir {
    fn write_dump<W>(&self, out: &mut W, indent: u32)
    where
        W: std::io::Write + ?Sized,
    {
        let maker_note_id = if self.type_ == IfdType::MakerNote {
            format!(" id={}", self.id)
        } else {
            String::default()
        };
        dump_writeln!(
            out,
            indent,
            "<IFD type={:?}{} {} entries next=@{}>",
            self.type_,
            maker_note_id,
            self.num_entries(),
            self.next_ifd()
        );
        {
            let indent = indent + 1;
            for (id, entry) in &self.entries {
                let tag_name = self.tag_names.get(id).unwrap_or(&"");
                let args = HashMap::from([("tag_name", String::from(*tag_name))]);
                entry.write_dump_entry(out, indent, self.endian(), args);
            }
        }
        dump_writeln!(out, indent, "</IFD>");
    }
}

#[cfg(test)]
mod test {
    use log;
    use simple_logger;

    use crate::io::Viewer;
    use crate::tiff;
    use crate::Type;

    #[test]
    fn test_ifd_iterator() {
        simple_logger::init_with_level(log::Level::Debug).unwrap();

        let reader = std::fs::File::open("sample.tiff").expect("Couldn't open file");
        let viewer = Viewer::new(Box::new(std::io::BufReader::new(reader)), 0);
        let view = Viewer::create_view(&viewer, 0).expect("Couldn't create view");
        let mut container = tiff::Container::new(view, vec![tiff::IfdType::Main], Type::Jpeg);
        container.load(None).expect("Failed to load IFD");
        let dir = container.directory(0).expect("Couldn't get directory");
        for meta in dir.iter() {
            println!("meta {:?}", meta);
        }
    }
}
