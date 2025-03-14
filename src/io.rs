// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - io.rs
 *
 * Copyright (C) 2022-2025 Hubert Figuière
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

//! Abstract the IO to allow for "stacking".

use std::cell::{RefCell, RefMut};
use std::io::{ErrorKind, Read, SeekFrom};
use std::rc::{Rc, Weak};

use byteorder::{BigEndian, ByteOrder, LittleEndian, ReadBytesExt};

use crate::container::Endian;
use crate::rawfile::ReadAndSeek;
use crate::utils;
use crate::{Error, Result};

/// This trait exists because `from_le_bytes()` and `from_be_bytes`
/// can't be used on a generics `T` as there is no bound for primitive types.
/// And `byteorder` crate doesn't have generics over type.
/// Implement the trait as needed.
pub trait FromBuf {
    /// Read from LittleEndian bytes.
    fn le_bytes(bytes: &[u8]) -> Self;
    /// Read from BigEndian bytes.
    fn be_bytes(bytes: &[u8]) -> Self;
}

impl FromBuf for u32 {
    #[inline]
    fn le_bytes(bytes: &[u8]) -> Self {
        LittleEndian::read_u32(bytes)
    }

    #[inline]
    fn be_bytes(bytes: &[u8]) -> Self {
        BigEndian::read_u32(bytes)
    }
}

impl FromBuf for u16 {
    #[inline]
    fn le_bytes(bytes: &[u8]) -> Self {
        LittleEndian::read_u16(bytes)
    }

    #[inline]
    fn be_bytes(bytes: &[u8]) -> Self {
        BigEndian::read_u16(bytes)
    }
}

#[derive(Debug)]
/// Wrap the IO for views.
///
/// ```no_compile
/// use io::Viewer;
///
/// let buffer = b"abcdefg";
/// let cursor = Box::new(std::io::Cursor::new(buffer.as_slice()));
///
/// let viewer = Viewer::new(cursor);
/// ```
pub(crate) struct Viewer {
    inner: RefCell<Box<dyn ReadAndSeek>>,
    length: u64,
}

impl Viewer {
    /// Create a new Viewer from an actual I/O.
    pub fn new(mut inner: Box<dyn ReadAndSeek>, length: u64) -> Rc<Self> {
        let length = if length == 0 {
            log::warn!("Length of ZERO passed to Viewer::new()");
            inner.seek(SeekFrom::End(0)).unwrap_or(0)
            // we assume the position will be reset.
        } else {
            length
        };

        Rc::new(Viewer {
            inner: RefCell::new(inner),
            length,
        })
    }

    /// Create a view at offset.
    pub fn create_view(viewer: &Rc<Viewer>, offset: u64) -> Result<View> {
        if offset > viewer.length() {
            return Err(Error::from(std::io::Error::new(
                ErrorKind::Other,
                "create_view: offset beyond EOF.",
            )));
        }
        View::new(viewer, offset, viewer.length() - offset)
    }

    /// Create a subview for view.
    pub fn create_subview(view: &View, offset: u64) -> Result<View> {
        view.inner
            .upgrade()
            .ok_or_else(|| {
                Error::from(std::io::Error::new(
                    ErrorKind::Other,
                    "failed to acquire Rc",
                ))
            })
            .and_then(|viewer| {
                if offset > viewer.length() {
                    return Err(Error::from(std::io::Error::new(
                        ErrorKind::Other,
                        "create_subview: offset beyond EOF.",
                    )));
                }
                View::new(&viewer, offset, viewer.length() - offset)
            })
    }

    pub fn length(&self) -> u64 {
        self.length
    }

    /// Get the inner io to make an io call
    pub fn get_io(&self) -> RefMut<'_, Box<dyn ReadAndSeek>> {
        self.inner.borrow_mut()
    }
}

#[derive(Clone, Debug)]
/// And IO View. Allow having file IO as an offset of another
/// Useful for containers.
pub struct View {
    inner: Weak<Viewer>,
    offset: u64,
    length: u64,
}

impl View {
    /// Crate a new view. `Viewer::create_view()` should be used instead.
    /// Length is the length of the view.
    fn new(viewer: &Rc<Viewer>, offset: u64, length: u64) -> Result<Self> {
        viewer.get_io().seek(SeekFrom::Start(offset))?;
        Ok(View {
            inner: Rc::downgrade(viewer),
            offset,
            length,
        })
    }

    /// Read an `u16` based on endian.
    pub fn read_endian_u16(&mut self, endian: Endian) -> std::io::Result<u16> {
        match endian {
            Endian::Little => self.read_u16::<LittleEndian>(),
            Endian::Big => self.read_u16::<BigEndian>(),
            Endian::Unset => {
                unreachable!("endian unset");
            }
        }
    }

    /// Read an array of `u16` with endian
    pub fn read_endian_u16_array(
        &mut self,
        arr: &mut [u16],
        endian: Endian,
    ) -> std::io::Result<()> {
        let bytes = utils::to_u8_slice_mut(arr);
        self.read_exact(bytes)?;
        match endian {
            Endian::Little => LittleEndian::from_slice_u16(arr),
            Endian::Big => BigEndian::from_slice_u16(arr),
            Endian::Unset => {
                unreachable!("endian unset");
            }
        }
        Ok(())
    }

    /// Read an `u32` based on endian.
    pub fn read_endian_u32(&mut self, endian: Endian) -> std::io::Result<u32> {
        match endian {
            Endian::Little => self.read_u32::<LittleEndian>(),
            Endian::Big => self.read_u32::<BigEndian>(),
            Endian::Unset => {
                unreachable!("endian unset");
            }
        }
    }

    /// Read an array of `u32` with endian
    pub fn read_endian_u32_array(
        &mut self,
        arr: &mut [u32],
        endian: Endian,
    ) -> std::io::Result<()> {
        let bytes = utils::to_u8_slice_mut(arr);
        self.read_exact(bytes)?;
        match endian {
            Endian::Little => LittleEndian::from_slice_u32(arr),
            Endian::Big => BigEndian::from_slice_u32(arr),
            Endian::Unset => {
                unreachable!("endian unset");
            }
        }
        Ok(())
    }

    /// Read an `i32` based on endian.
    pub fn read_endian_i32(&mut self, endian: Endian) -> std::io::Result<i32> {
        match endian {
            Endian::Little => self.read_i32::<LittleEndian>(),
            Endian::Big => self.read_i32::<BigEndian>(),
            Endian::Unset => {
                unreachable!("endian unset");
            }
        }
    }

    pub(crate) fn len(&self) -> u64 {
        self.length
    }

    #[cfg(feature = "dump")]
    pub(crate) fn offset(&self) -> u64 {
        self.offset
    }

    /// Only for test to create a non functional `View`
    #[cfg(test)]
    pub fn new_test() -> Self {
        View {
            inner: Weak::new(),
            offset: 0,
            length: 0,
        }
    }
}

impl std::io::Read for View {
    fn read(&mut self, buf: &mut [u8]) -> std::io::Result<usize> {
        let inner = self.inner.upgrade().expect("Couldn't upgrade inner");
        let mut io = inner.get_io();
        io.read(buf)
    }
}

impl std::io::Seek for View {
    fn seek(&mut self, pos: SeekFrom) -> std::io::Result<u64> {
        let inner = self.inner.upgrade().expect("Couldn't upgrade inner");
        let mut io = inner.get_io();
        io.seek(match pos {
            SeekFrom::Start(p) => {
                if p > self.length {
                    log::error!("Seeking past EOF {}", p);
                    return Err(std::io::Error::from(std::io::ErrorKind::UnexpectedEof));
                } else {
                    SeekFrom::Start(p + self.offset)
                }
            }
            _ => pos,
        })
        .map(|i| i - self.offset)
    }
}

impl ReadAndSeek for View {}

#[cfg(test)]
mod test {
    use std::io::{Read, Seek};

    use super::Viewer;

    #[test]
    fn test_view() {
        const OFFSET: u64 = 8;
        let buffer = b"abcdefghijklmnopqrstuvwxyz0123456789";

        let mut io = Box::new(std::io::Cursor::new(buffer.as_slice()));
        assert_eq!(io.stream_position().unwrap(), 0);

        let viewer = Viewer::new(io, buffer.len() as u64);

        let mut view = Viewer::create_view(&viewer, OFFSET).unwrap();

        assert_eq!(view.stream_position().unwrap(), 0);
        assert_eq!(viewer.get_io().stream_position().unwrap(), OFFSET);

        let mut buf = [0u8; 4];
        let r = view.read(&mut buf);
        assert_eq!(r.unwrap(), 4);
        assert_eq!(&buf, b"ijkl");
    }
}
