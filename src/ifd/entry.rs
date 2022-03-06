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

use byteorder::ByteOrder;

use crate::exif::ExifValue;

/// IFD entry
pub struct Entry {
    /// The tag
    _id: u16,
    /// The type. See `exif::TagType`
    type_: i16,
    count: u32,
    data: [u8; 4],
}

impl Entry {
    pub fn new(id: u16, type_: i16, count: u32, data: [u8; 4]) -> Self {
        Entry {
            _id: id,
            type_,
            count,
            data,
        }
    }

    /// Get the value at index.
    pub fn value_at_index<T, E>(&self, index: u32) -> Option<T>
    where
        T: ExifValue,
        E: ByteOrder,
    {
        if self.type_ == T::exif_type() as i16 {
            if index >= self.count {
                return None;
            }
            // inline
            let data_size = T::unit_size() * self.count as usize;
            if data_size <= 4 {
                return Some(T::read::<E>(&self.data[T::unit_size() * index as usize..]));
            } else {
                unimplemented!("Need to implement fetching data")
            }
        }
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
}

#[cfg(test)]
mod test {
    use byteorder::{ByteOrder, LittleEndian};

    use super::Entry;

    use crate::exif::TagType;

    #[test]
    fn test_entry_get_value() {
        let e = Entry::new(0, TagType::Byte as i16, 3, [10, 20, 30, 0]);
        assert_eq!(e.value_at_index::<u8, LittleEndian>(3), None);
        assert_eq!(e.value::<u16, LittleEndian>(), None);
        assert_eq!(e.value_at_index::<u8, LittleEndian>(2), Some(30));

        let e = Entry::new(0, TagType::Ascii as i16, 4, *b"asci");
        assert_eq!(
            e.value::<String, LittleEndian>(),
            Some(String::from("asci"))
        );

        let mut buf = [0_u8; 4];
        LittleEndian::write_f32(&mut buf, 3.14);
        let e = Entry::new(0, TagType::Float as i16, 1, buf);
        assert_eq!(e.value::<f32, LittleEndian>(), Some(3.14));
    }
}
