/*
 * libopenraw - ifd/entry.rs
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

use byteorder::{BigEndian, ByteOrder, LittleEndian};
use log::debug;
use std::io::{Read, Seek, SeekFrom};

use crate::container::Endian;
use crate::io::View;
use crate::{Error, Result};

use super::exif;
use super::exif::{ExifValue, TagType};

/// Represent the data bytes, either the 4 bytes read
/// Or the read bytes from the view.
enum DataBytes {
    Inline([u8; 4]),
    External(Vec<u8>),
}

impl DataBytes {
    pub fn as_slice(&self) -> &[u8] {
        match *self {
            Self::Inline(ref b) => b,
            Self::External(ref v) => v.as_slice(),
        }
    }
}

/// IFD entry
pub struct Entry {
    /// The tag
    _id: u16,
    /// The type. See `exif::TagType`, use `exif::TagType::try_from()`
    /// to get the enum.
    type_: i16,
    count: u32,
    data: DataBytes,
}

impl Entry {
    pub fn new(id: u16, type_: i16, count: u32, data: [u8; 4]) -> Self {
        Entry {
            _id: id,
            type_,
            count,
            data: DataBytes::Inline(data),
        }
    }

    /// Return wether the entry is inline.
    pub fn is_inline(&self) -> bool {
        let tag_type = TagType::try_from(self.type_).unwrap_or(TagType::Invalid);
        let data_size = exif::tag_unit_size(tag_type) * self.count as usize;
        data_size <= 4
    }

    /// Load the data for the entry from the `io::View`.
    pub(crate) fn load_data<E>(&mut self, view: &mut View) -> Result<usize>
    where
        E: ByteOrder,
    {
        if let DataBytes::External(_) = self.data {
            return Err(Error::AlreadyInited);
        }

        let offset = E::read_u32(self.data.as_slice());
        let tag_type = TagType::try_from(self.type_).unwrap_or(TagType::Invalid);
        let data_size = exif::tag_unit_size(tag_type) * self.count as usize;
        debug!("Loading data at {}: {} bytes", offset, data_size);

        view.seek(SeekFrom::Start(offset as u64))?;
        let mut data = Vec::new();
        // XXX can we use an unitialized vector?
        data.resize(data_size, 0);
        let bytes = view.read(&mut data)?;
        self.data = DataBytes::External(data);

        Ok(bytes)
    }

    /// Get the value at index.
    pub fn value_at_index<T, E>(&self, index: u32) -> Option<T>
    where
        T: ExifValue,
        E: ByteOrder,
    {
        if self.type_ == T::exif_type() as i16 {
            if index >= self.count {
                log::error!("index {} is >= {}", index, self.count);
                return None;
            }
            return Some(T::read::<E>(
                &self.data.as_slice()[T::unit_size() * index as usize..],
            ));
        }
        log::error!("incorrect type {} for {:?}", self.type_, T::exif_type());
        None
    }

    /// Get the value out of the entry.
    pub fn value<T, E>(&self) -> Option<T>
    where
        T: ExifValue,
        E: ByteOrder,
    {
        self.value_at_index::<T, E>(0)
    }

    /// Get the value array out of the entry, using `endian`.
    pub fn value_array<T>(&self, endian: Endian) -> Option<Vec<T>>
    where
        T: ExifValue,
    {
        if !T::is_array() && self.type_ == T::exif_type() as i16 {
            let mut values = Vec::new();
            for index in 0..self.count {
                let v = match endian {
                    Endian::Big => T::read::<BigEndian>(
                        &self.data.as_slice()[T::unit_size() * index as usize..],
                    ),
                    Endian::Little => T::read::<LittleEndian>(
                        &self.data.as_slice()[T::unit_size() * index as usize..],
                    ),
                    _ => unreachable!(),
                };
                values.push(v);
            }
            Some(values)
        } else {
            log::error!("incorrect type {} for {:?}", self.type_, T::exif_type());
            None
        }
    }
}

#[cfg(test)]
mod test {
    use byteorder::{BigEndian, ByteOrder, LittleEndian};

    use super::Entry;

    use crate::container::Endian;
    use crate::ifd::exif::TagType;
    use crate::Error;

    #[test]
    fn test_entry_get_value() {
        let e = Entry::new(0, TagType::Byte as i16, 3, [10, 20, 30, 0]);
        assert_eq!(e.value_at_index::<u8, LittleEndian>(3), None);
        assert_eq!(e.value::<u16, LittleEndian>(), None);
        assert_eq!(e.value_at_index::<u8, LittleEndian>(2), Some(30));
        // testing value_array
        assert_eq!(
            e.value_array::<u8>(Endian::Little),
            Some(vec![10_u8, 20, 30])
        );
        assert_eq!(e.value_array::<u16>(Endian::Little), None);

        let e = Entry::new(0, TagType::Ascii as i16, 4, *b"asci");
        assert_eq!(
            e.value::<String, LittleEndian>(),
            Some(String::from("asci"))
        );

        let mut buf = [0_u8; 4];
        LittleEndian::write_f32(&mut buf, 3.14);
        let e = Entry::new(0, TagType::Float as i16, 1, buf);
        assert_eq!(e.value::<f32, LittleEndian>(), Some(3.14));

        BigEndian::write_f32(&mut buf, 3.14);
        let e = Entry::new(0, TagType::Float as i16, 1, buf);
        assert_eq!(e.value::<f32, BigEndian>(), Some(3.14));
    }

    #[test]
    fn test_load_value() {
        use crate::io;

        let buf = Vec::from(b"abcdedfgijkl".as_slice());

        let cursor = Box::new(std::io::Cursor::new(buf));
        let viewer = std::rc::Rc::new(io::Viewer::new(cursor));

        let view = io::Viewer::create_view(&viewer, 0);
        assert!(view.is_ok());
        let mut view = view.unwrap();

        // Little endian

        let mut e = Entry::new(0, TagType::Ascii as i16, 8, [4, 0, 0, 0]);
        let r = e.load_data::<LittleEndian>(&mut view);
        assert_eq!(r, Ok(8));
        assert_eq!(
            e.value::<String, LittleEndian>(),
            Some(String::from("edfgijkl"))
        );
        // Trying to load again should fail.
        let r = e.load_data::<LittleEndian>(&mut view);
        assert_eq!(r, Err(Error::AlreadyInited));

        // Big endian
        let mut e = Entry::new(0, TagType::Ascii as i16, 8, [0, 0, 0, 4]);
        let r = e.load_data::<BigEndian>(&mut view);
        assert_eq!(r, Ok(8));
        assert_eq!(
            e.value::<String, BigEndian>(),
            Some(String::from("edfgijkl"))
        );
    }
}
