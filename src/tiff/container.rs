/*
 * libopenraw - tiff/container.rs
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

//! The IFD Container. Contains the IFD `Dir`

use std::cell::{RefCell, RefMut};
use std::io::{Read, Seek, SeekFrom};
use std::rc::Rc;

use byteorder::{BigEndian, LittleEndian, ReadBytesExt};
use log::error;
use once_cell::unsync::OnceCell;

use crate::container;
use crate::container::RawContainer;
use crate::io;
use crate::io::View;
use crate::jpeg;
use crate::thumbnail;
use crate::tiff::{Dir, Entry, Type};
use crate::Type as RawType;
use crate::{DataType, Dump, Error, Result};

type CheckMagicHeader = fn(&[u8]) -> Result<container::Endian>;

/// IFD Container for TIFF based file.
pub(crate) struct Container {
    /// The `io::View`.
    view: RefCell<View>,
    /// Endian of the container.
    endian: RefCell<container::Endian>,
    /// IFD.
    dirs: OnceCell<Vec<Rc<Dir>>>,
    /// index to `Type` map
    dir_map: Vec<Type>,
    /// offset correction for Exif. 0 in most cases.
    exif_correction: i32,
    /// The Exif IFD
    exif_ifd: OnceCell<Option<Rc<Dir>>>,
    /// The MakerNote IFD
    mnote_ifd: OnceCell<Option<Rc<Dir>>>,
    raw_type: RawType,
}

impl container::RawContainer for Container {
    fn endian(&self) -> container::Endian {
        *self.endian.borrow()
    }

    fn borrow_view_mut(&self) -> RefMut<'_, View> {
        self.view.borrow_mut()
    }

    fn raw_type(&self) -> RawType {
        self.raw_type
    }
}

impl Container {
    /// Create a new container for the view.
    pub(crate) fn new(view: View, dir_map: Vec<Type>, raw_type: RawType) -> Self {
        Self {
            view: RefCell::new(view),
            endian: RefCell::new(container::Endian::Unset),
            dirs: OnceCell::new(),
            dir_map,
            exif_correction: 0,
            exif_ifd: OnceCell::new(),
            mnote_ifd: OnceCell::new(),
            raw_type,
        }
    }

    /// The the Exif correction.
    pub fn set_exif_correction(&mut self, correction: i32) {
        self.exif_correction = correction;
    }

    pub fn exif_correction(&self) -> i32 {
        self.exif_correction
    }

    pub fn read_u16_array(
        &self,
        view: &mut View,
        array: &mut [u16],
        count: usize,
    ) -> std::io::Result<usize> {
        assert!(array.len() >= count);
        match *self.endian.borrow() {
            container::Endian::Little => {
                for item in array.iter_mut().take(count) {
                    *item = view.read_u16::<LittleEndian>()?
                }
                Ok(count)
            }
            container::Endian::Big => {
                for item in array.iter_mut().take(count) {
                    *item = view.read_u16::<BigEndian>()?
                }
                Ok(count)
            }
            container::Endian::Unset => {
                unreachable!("endian unset");
            }
        }
    }

    /// Read an `u16` based on the container endian.
    pub fn read_u16(&self, view: &mut View) -> std::io::Result<u16> {
        match *self.endian.borrow() {
            container::Endian::Little => view.read_u16::<LittleEndian>(),
            container::Endian::Big => view.read_u16::<BigEndian>(),
            container::Endian::Unset => {
                unreachable!("endian unset");
            }
        }
    }

    /// Read an `u32` based on the container endian.
    fn read_u32(&self, view: &mut View) -> std::io::Result<u32> {
        match *self.endian.borrow() {
            container::Endian::Little => view.read_u32::<LittleEndian>(),
            container::Endian::Big => view.read_u32::<BigEndian>(),
            container::Endian::Unset => {
                unreachable!("endian unset");
            }
        }
    }

    /// load the container.
    pub(crate) fn load(&mut self, check_magic_header: Option<CheckMagicHeader>) -> Result<()> {
        let mut view = self.view.borrow_mut();
        view.seek(SeekFrom::Start(0))?;
        let mut buf = [0_u8; 4];
        view.read_exact(&mut buf)?;
        if let Some(check_magic_header) = check_magic_header {
            self.endian.replace(check_magic_header(&buf)?);
        } else {
            self.endian.replace(Self::is_magic_header(&buf)?);
        }

        Ok(())
    }

    /// Read the dir at the offset
    pub(crate) fn dir_at(&self, view: &mut View, offset: u32, t: Type) -> Result<Dir> {
        match *self.endian.borrow() {
            container::Endian::Little => Dir::read::<LittleEndian>(view, offset, 0, t),
            container::Endian::Big => Dir::read::<BigEndian>(view, offset, 0, t),
            _ => {
                error!("Endian unset to read directory");
                Err(Error::NotFound)
            }
        }
    }

    /// Get the directories. They get loaded once as needed.
    pub(crate) fn dirs(&self) -> &Vec<Rc<Dir>> {
        self.dirs.get_or_init(|| {
            let mut dirs = vec![];

            let mut index = 0_usize;
            let mut dir_offset = {
                let mut view = self.view.borrow_mut();
                view.seek(SeekFrom::Start(4)).expect("Seek failed");
                self.read_u32(&mut view).unwrap_or(0)
            };
            while dir_offset != 0 {
                let t = if index < self.dir_map.len() {
                    self.dir_map[index]
                } else {
                    Type::Other
                };
                if let Ok(dir) = if t == Type::MakerNote {
                    Dir::create_maker_note(self, dir_offset)
                } else {
                    self.dir_at(&mut *self.view.borrow_mut(), dir_offset, t)
                } {
                    dir_offset = dir.next_ifd();
                    dirs.push(Rc::new(dir));
                    index += 1
                } else {
                    error!("Endian couldn't read directory");
                    break;
                }
            }

            dirs
        })
    }

    /// Get the indexed `tiff::Dir` from the container
    pub fn directory(&self, idx: usize) -> Option<Rc<Dir>> {
        let dirs = self.dirs();
        if dirs.len() <= idx {
            return None;
        }

        Some(dirs[idx].clone())
    }

    /// Will identify the magic header and return the endian
    fn is_magic_header(buf: &[u8]) -> Result<container::Endian> {
        if buf.len() < 4 {
            error!("IFD magic header buffer too small: {} bytes", buf.len());
            return Err(Error::BufferTooSmall);
        }

        if &buf[0..4] == b"II\x2a\x00" {
            Ok(container::Endian::Little)
        } else if &buf[0..4] == b"MM\x00\x2a" {
            Ok(container::Endian::Big)
        } else {
            error!("Incorrect IFD magic: {:?}", buf);
            Err(Error::FormatError)
        }
    }

    /// Lazily load the Exif dir and return it.
    pub(crate) fn exif_dir(&self) -> Option<Rc<Dir>> {
        self.exif_ifd
            .get_or_init(|| {
                self.directory(0)
                    .and_then(|dir| dir.get_exif_ifd(self))
                    .or_else(|| {
                        log::warn!("Coudln't find Exif IFD");
                        None
                    })
            })
            .clone()
    }

    /// Lazily load the MakerNote and return it.
    pub(crate) fn mnote_dir(&self) -> Option<Rc<Dir>> {
        self.mnote_ifd
            .get_or_init(|| {
                log::debug!("Loading MakerNote");
                self.exif_dir()
                    .and_then(|dir| dir.get_mnote_ifd(self))
                    .or_else(|| {
                        log::warn!("Couldn't find MakerNote");
                        None
                    })
            })
            .clone()
    }

    /// Add the thumbnail from data in the container
    pub(crate) fn add_thumbnail_from_stream(
        &self,
        offset: u32,
        len: u32,
        list: &mut Vec<(u32, thumbnail::ThumbDesc)>,
    ) -> Result<usize> {
        let view = io::Viewer::create_subview(&self.borrow_view_mut(), offset as u64)?;
        let jpeg = jpeg::Container::new(view, self.raw_type);
        let width = jpeg.width() as u32;
        let height = jpeg.height() as u32;
        let dim = std::cmp::max(width, height);
        // "Olympus" MakerNote carries a 160 px thubnail we might already have.
        // We don't check it is the same.
        if !list.iter().any(|t| t.0 == dim) {
            use crate::thumbnail::{Data, DataOffset};

            list.push((
                dim,
                thumbnail::ThumbDesc {
                    width,
                    height,
                    data_type: DataType::Jpeg,
                    data: Data::Offset(DataOffset {
                        offset: offset as u64,
                        len: len as u64,
                    }),
                },
            ));
            Ok(1)
        } else {
            Ok(0)
        }
    }

    /// Add a thumbnail from the entry
    pub(crate) fn add_thumbnail_from_entry(
        &self,
        e: &Entry,
        offset: u32,
        list: &mut Vec<(u32, thumbnail::ThumbDesc)>,
    ) -> Result<usize> {
        if let Some(val_offset) = e.offset() {
            let val_offset = val_offset + offset;
            self.add_thumbnail_from_stream(val_offset, e.count, list)
        } else {
            log::error!("Entry for thumbnail has no offset");
            Err(Error::NotFound)
        }
    }
}

impl Dump for Container {
    #[cfg(feature = "dump")]
    fn print_dump(&self, indent: u32) {
        let dirs = self.dirs();
        dump_println!(
            indent,
            "<TIFF Container endian={} {} directories @{}>",
            match self.endian() {
                container::Endian::Little => "II",
                container::Endian::Big => "MM",
                _ => "Unknown",
            },
            dirs.len(),
            self.view.borrow().offset()
        );
        {
            let indent = indent + 1;
            for dir in dirs {
                dir.print_dump(indent);
                if let Some(exif_dir) = dir.get_exif_ifd(self) {
                    exif_dir.print_dump(indent + 1);
                    if let Some(mnote_dir) = exif_dir.get_mnote_ifd(self) {
                        mnote_dir.print_dump(indent + 2);
                    }
                }
                if let Some(subdirs) = dir.get_sub_ifds(self) {
                    for subdir in subdirs {
                        subdir.print_dump(indent + 1);
                    }
                }
            }
        }
        dump_println!(indent, "</TIFF Container>");
    }
}
