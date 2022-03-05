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

use std::cell::{RefCell, RefMut};
use std::io::{Result, SeekFrom};
use std::rc::{Rc, Weak};

use crate::rawfile::ReadAndSeek;

/// Wrap the IO for views.
pub(crate) struct Viewer {
    inner: RefCell<Box<dyn ReadAndSeek>>,
}

impl Viewer {
    pub fn new(inner: Box<dyn ReadAndSeek>) -> Rc<Self> {
        Rc::new(Viewer {
            inner: RefCell::new(inner),
        })
    }

    /// Create a view at offset.
    pub fn create_view(viewer: &Rc<Viewer>, offset: u64) -> Result<View> {
        View::new(viewer, offset)
    }

    /// Get the inner io to make an io call
    pub fn get_io(&self) -> RefMut<'_, Box<dyn ReadAndSeek>> {
        self.inner.borrow_mut()
    }
}

/// And IO View. Allow having file IO as an offset of another
/// Useful for containers.
pub(crate) struct View {
    inner: Weak<Viewer>,
    offset: u64,
}

impl View {
    fn new(viewer: &Rc<Viewer>, offset: u64) -> Result<Self> {
        viewer.get_io().seek(SeekFrom::Start(offset))?;
        Ok(View {
            inner: Rc::downgrade(viewer),
            offset,
        })
    }
}

impl std::io::Read for View {
    fn read(&mut self, buf: &mut [u8]) -> Result<usize> {
        let inner = self.inner.upgrade().unwrap();
        let mut io = inner.get_io();
        io.read(buf)
    }
}

impl std::io::Seek for View {
    fn seek(&mut self, pos: SeekFrom) -> Result<u64> {
        let inner = self.inner.upgrade().unwrap();
        let mut io = inner.get_io();
        io.seek(match pos {
            SeekFrom::Start(p) => SeekFrom::Start(p + self.offset),
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

        let viewer = Viewer::new(io);

        let mut view = Viewer::create_view(&viewer, OFFSET).unwrap();

        assert_eq!(view.stream_position().unwrap(), 0);
        assert_eq!(viewer.get_io().stream_position().unwrap(), OFFSET);

        let mut buf = [0u8; 4];
        let r = view.read(&mut buf);
        assert_eq!(r.unwrap(), 4);
        assert_eq!(&buf, b"ijkl");
    }
}
