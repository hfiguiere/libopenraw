// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - io.rs
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

//! Abstract the IO to allow for "stacking".

use std::cell::{RefCell, RefMut};
use std::io::{Error, ErrorKind, Result, SeekFrom};
use std::rc::{Rc, Weak};

use crate::rawfile::ReadAndSeek;

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
            return Err(Error::new(
                ErrorKind::Other,
                "create_view: offset beyond EOF.",
            ));
        }
        View::new(viewer, offset, viewer.length() - offset)
    }

    /// Create a subview for view.
    pub fn create_subview(view: &View, offset: u64) -> Result<View> {
        view.inner
            .upgrade()
            .ok_or_else(|| Error::new(ErrorKind::Other, "failed to acquire Rc"))
            .and_then(|viewer| {
                if offset > viewer.length() {
                    return Err(Error::new(
                        ErrorKind::Other,
                        "create_subview: offset beyond EOF.",
                    ));
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

/// And IO View. Allow having file IO as an offset of another
/// Useful for containers.
#[derive(Clone)]
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
    fn read(&mut self, buf: &mut [u8]) -> Result<usize> {
        let inner = self.inner.upgrade().expect("Couldn't upgrade inner");
        let mut io = inner.get_io();
        io.read(buf)
    }
}

impl std::io::Seek for View {
    fn seek(&mut self, pos: SeekFrom) -> Result<u64> {
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
