/*
 * libopenraw - ifd/container.rs
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

use std::cell::RefCell;
use std::io::{Read, Seek, SeekFrom};
use std::rc::Rc;

use byteorder::{BigEndian, LittleEndian, ReadBytesExt};
use once_cell::unsync::OnceCell;

use crate::container;
use crate::ifd::{Dir, Type};
use crate::io::View;
use crate::thumbnail;
use crate::thumbnail::Thumbnail;
use crate::{Error, Result};

pub(crate) struct Container {
    view: RefCell<View>,
    endian: RefCell<container::Endian>,
    dirs: OnceCell<Vec<Rc<Dir>>>,
    /// offset correction for Exif. 0 in most cases.
    exif_correction: i32,
}

impl container::Container for Container {
    fn endian(&self) -> container::Endian {
        *self.endian.borrow()
    }

    fn make_thumbnail(&self, _desc: &thumbnail::ThumbDesc) -> Result<Thumbnail> {
        Err(Error::NotSupported)
    }
}

impl Container {
    pub(crate) fn new(view: View) -> Self {
        Self {
            view: RefCell::new(view),
            endian: RefCell::new(container::Endian::Unset),
            dirs: OnceCell::new(),
            exif_correction: 0,
        }
    }

    /// The the Exif correction.
    pub fn set_exif_correction(&mut self, correction: i32) {
        self.exif_correction = correction;
    }

    fn read_i32(&self, view: &mut View) -> std::io::Result<i32> {
        match *self.endian.borrow() {
            container::Endian::Little => view.read_i32::<LittleEndian>(),
            container::Endian::Big => view.read_i32::<BigEndian>(),
            container::Endian::Unset => unreachable!("endian unset"),
        }
    }

    pub(crate) fn load(&mut self) -> Result<()> {
        let mut view = self.view.borrow_mut();
        view.seek(SeekFrom::Start(0))?;
        let mut buf = [0_u8; 4];
        view.read_exact(&mut buf)?;
        self.endian.replace(self.is_magic_header(&buf)?);

        Ok(())
    }

    pub(crate) fn dirs(&self) -> &Vec<Rc<Dir>> {
        self.dirs.get_or_init(|| {
            let mut dirs = vec![];

            let mut view = self.view.borrow_mut();
            view.seek(SeekFrom::Start(4)).expect("Seek failed");
            let mut dir_offset = self.read_i32(&mut view).unwrap_or(0);
            while dir_offset != 0 {
                if let Ok(dir) = match *self.endian.borrow() {
                    container::Endian::Little => {
                        Dir::read::<LittleEndian>(&mut view, dir_offset, Type::Other)
                    }
                    container::Endian::Big => {
                        Dir::read::<BigEndian>(&mut view, dir_offset, Type::Other)
                    }
                    _ => {
                        // XXX log this
                        unreachable!("endian failed");
                    }
                } {
                    dir_offset = dir.next_ifd();
                    dirs.push(Rc::new(dir));
                } else {
                    // XXX log this
                    break;
                }
            }

            dirs
        })
    }

    /// Get the `ifd::Dir` from the container
    pub fn directory(&self, idx: usize) -> Option<Rc<Dir>> {
        let dirs = self.dirs();
        if dirs.len() <= idx {
            return None;
        }

        Some(dirs[idx].clone())
    }

    /// Will identify the magic header and return the endian
    fn is_magic_header(&self, buf: &[u8]) -> Result<container::Endian> {
        if buf.len() < 4 {
            return Err(Error::BufferTooSmall);
        }

        if buf == b"II\x2a\x00" {
            Ok(container::Endian::Little)
        } else if buf == b"MM\x00\x2a" {
            Ok(container::Endian::Big)
        } else {
            Err(Error::FormatError)
        }
    }
}
