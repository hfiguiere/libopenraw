/*
 * libopenraw - tiff/entry.rs
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

//! IFD entries.

#[cfg(feature = "dump")]
use std::collections::HashMap;
use std::convert::TryFrom;
use std::io::{Read, Seek, SeekFrom};

use byteorder::{BigEndian, ByteOrder, LittleEndian};
use log::debug;

use crate::container::Endian;
use crate::io::View;
use crate::{Error, Result};

use super::exif;
use super::exif::{ExifValue, TagType};

/// Represent the data bytes, either the 4 bytes read,
/// the read bytes from the view or the offset.
#[derive(Clone)]
enum DataBytes {
    /// Inline data. In the IFD byte order.
    Inline([u8; 4]),
    /// External data loaded from the contain in the IFD byte order.
    External(Vec<u8>),
    /// Offset of the data in the container. This is only for
    /// `Undefined` entry types. Use `Entry::offset()` to retrieve it.
    Offset(u32),
}

impl DataBytes {
    /// Convert the data buffer into a slice
    /// Will panic if it is an offset.
    pub fn as_slice(&self) -> &[u8] {
        match *self {
            Self::Inline(ref b) => b,
            Self::External(ref v) => v.as_slice(),
            _ => unreachable!("Entry data is offset"),
        }
    }
}

/// IFD entry
#[derive(Clone)]
pub struct Entry {
    /// The tag
    _id: u16,
    /// The type. See `exif::TagType`, use `exif::TagType::try_from()`
    /// to get the enum.
    pub(crate) type_: i16,
    pub(crate) count: u32,
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
        if tag_type == TagType::Undefined {
            false
        } else {
            let data_size = exif::tag_unit_size(tag_type) * self.count as usize;
            data_size <= 4
        }
    }

    /// Set the entry as containing an offset.
    pub(crate) fn set_offset(&mut self, offset: u32) {
        self.data = DataBytes::Offset(offset);
    }

    /// Get the offset if it exist
    pub(crate) fn offset(&self) -> Option<u32> {
        match self.data {
            DataBytes::Offset(offset) => Some(offset),
            _ => None,
        }
    }

    pub(crate) fn data(&self) -> &[u8] {
        self.data.as_slice()
    }

    /// Load the data for the entry from the `io::View`.
    /// It doesn't check if the value is inline.
    pub(crate) fn load_data<E>(&mut self, view: &mut View) -> Result<usize>
    where
        E: ByteOrder,
    {
        if let DataBytes::External(_) = self.data {
            return Err(Error::AlreadyInited);
        }

        let offset = match self.data {
            DataBytes::Offset(offset) => offset,
            _ => E::read_u32(self.data.as_slice()),
        };
        let tag_type = TagType::try_from(self.type_).unwrap_or(TagType::Invalid);
        let data_size = exif::tag_unit_size(tag_type) * self.count as usize;
        debug!("Loading data at {}: {} bytes", offset, data_size);

        view.seek(SeekFrom::Start(offset as u64))?;
        let mut data = Vec::with_capacity(data_size);
        // Avoiding initialization of the big buffer.
        // This is deliberate.
        #[allow(clippy::uninit_vec)]
        unsafe {
            data.set_len(data_size);
        }

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
        self.value_at_index_::<T, E>(index, false)
    }

    /// Get the value at index. Ignore typing if `untyped` is true.
    fn value_at_index_<T, E>(&self, index: u32, untyped: bool) -> Option<T>
    where
        T: ExifValue,
        E: ByteOrder,
    {
        if untyped || (self.type_ == T::exif_type() as i16) {
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

    /// Get the uint value at index. Ignore typing between SHORT and LONG
    fn uint_value_at_index<E>(&self, index: u32) -> Option<u32>
    where
        E: ByteOrder,
    {
        if index >= self.count {
            log::error!("index {} is >= {}", index, self.count);
            return None;
        }
        exif::TagType::try_from(self.type_)
            .ok()
            .and_then(|typ| match typ {
                TagType::Short => Some(u16::read::<E>(
                    &self.data.as_slice()[u16::unit_size() * index as usize..],
                ) as u32),
                TagType::Long => Some(u32::read::<E>(
                    &self.data.as_slice()[u32::unit_size() * index as usize..],
                )),
                _ => {
                    log::error!("incorrect type {} for uint", self.type_);
                    None
                }
            })
    }

    /// Get the signed int value at index. Ignore typing between SSHORT
    /// and SLONG
    fn int_value_at_index<E>(&self, index: u32) -> Option<i32>
    where
        E: ByteOrder,
    {
        if index >= self.count {
            log::error!("index {} is >= {}", index, self.count);
            return None;
        }
        exif::TagType::try_from(self.type_)
            .ok()
            .and_then(|typ| match typ {
                TagType::SShort => Some(i16::read::<E>(
                    &self.data.as_slice()[i16::unit_size() * index as usize..],
                ) as i32),
                TagType::SLong => Some(i32::read::<E>(
                    &self.data.as_slice()[i32::unit_size() * index as usize..],
                )),
                _ => {
                    log::error!("incorrect type {} for uint", self.type_);
                    None
                }
            })
    }

    /// Get the value out of the entry, ignoring the type.
    pub fn value_untyped<T, E>(&self) -> Option<T>
    where
        T: ExifValue,
        E: ByteOrder,
    {
        self.value_at_index_::<T, E>(0, true)
    }

    /// Get an uint value out of the entry
    pub fn uint_value<E>(&self) -> Option<u32>
    where
        E: ByteOrder,
    {
        self.uint_value_at_index::<E>(0)
    }

    /// Get an int value out of the entry
    pub fn int_value<E>(&self) -> Option<i32>
    where
        E: ByteOrder,
    {
        self.int_value_at_index::<E>(0)
    }

    /// Get the value out of the entry.
    pub fn value<T, E>(&self) -> Option<T>
    where
        T: ExifValue,
        E: ByteOrder,
    {
        self.value_at_index_::<T, E>(0, false)
    }

    /// Get the value array out of the entry, using `endian`.
    pub fn uint_value_array(&self, endian: Endian) -> Option<Vec<u32>> {
        let type_ = match exif::TagType::try_from(self.type_) {
            Ok(t @ TagType::Short) | Ok(t @ TagType::Long) => t,
            _ => {
                log::error!("incorrect type {} for uint", self.type_);
                return None;
            }
        };
        let unit_size = match type_ {
            TagType::Short => u16::unit_size(),
            TagType::Long => u32::unit_size(),
            _ => unreachable!(),
        };

        let data_slice = self.data.as_slice();
        let count = self.count as usize;
        let mut values = Vec::new();
        for index in 0..count {
            let slice = &data_slice[unit_size * index as usize..];
            let v = match type_ {
                TagType::Short => {
                    (match endian {
                        Endian::Big => u16::read::<BigEndian>(slice),
                        Endian::Little => u16::read::<LittleEndian>(slice),
                        _ => unreachable!(),
                    }) as u32
                }
                TagType::Long => match endian {
                    Endian::Big => u32::read::<BigEndian>(slice),
                    Endian::Little => u32::read::<LittleEndian>(slice),
                    _ => unreachable!(),
                },
                _ => unreachable!(),
            };
            values.push(v);
        }
        Some(values)
    }

    /// Get the value array out of the entry, using `endian`.
    pub fn int_value_array(&self, endian: Endian) -> Option<Vec<i32>> {
        let type_ = match exif::TagType::try_from(self.type_) {
            Ok(t @ TagType::SShort) | Ok(t @ TagType::SLong) => t,
            _ => {
                log::error!("incorrect type {} for uint", self.type_);
                return None;
            }
        };
        let unit_size = match type_ {
            TagType::SShort => i16::unit_size(),
            TagType::SLong => i32::unit_size(),
            _ => unreachable!(),
        };

        let data_slice = self.data.as_slice();
        let count = self.count as usize;
        let mut values = Vec::new();
        for index in 0..count {
            let slice = &data_slice[unit_size * index as usize..];
            let v = match type_ {
                TagType::SShort => {
                    (match endian {
                        Endian::Big => i16::read::<BigEndian>(slice),
                        Endian::Little => i16::read::<LittleEndian>(slice),
                        _ => unreachable!(),
                    }) as i32
                }
                TagType::SLong => match endian {
                    Endian::Big => i32::read::<BigEndian>(slice),
                    Endian::Little => i32::read::<LittleEndian>(slice),
                    _ => unreachable!(),
                },
                _ => unreachable!(),
            };
            values.push(v);
        }
        Some(values)
    }

    /// Get the value array out of the entry, using `endian`.
    pub fn value_array<T>(&self, endian: Endian) -> Option<Vec<T>>
    where
        T: ExifValue,
    {
        let data_slice = self.data.as_slice();
        let count = if self.type_ == TagType::Undefined as i16 {
            // count is in bytes
            self.count as usize / T::unit_size()
        } else {
            self.count as usize
        };
        if !T::is_array()
            && ((self.type_ == T::exif_type() as i16) || (self.type_ == TagType::Undefined as i16))
        {
            let mut values = Vec::new();
            for index in 0..count {
                let slice = &data_slice[T::unit_size() * index as usize..];
                let v = match endian {
                    Endian::Big => T::read::<BigEndian>(slice),
                    Endian::Little => T::read::<LittleEndian>(slice),
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

    #[cfg(feature = "dump")]
    pub(crate) fn print_dump_entry(
        &self,
        indent: u32,
        endian: Endian,
        args: HashMap<&str, String>,
    ) {
        fn array_to_str<V>(array: &Vec<V>) -> String
        where
            V: ToString,
        {
            if array.len() == 1 {
                array[0].to_string()
            } else {
                let mut s = String::from("[ ");
                for (i, v) in array.iter().enumerate() {
                    if i > 0 {
                        s.push_str(", ");
                    }
                    if i > 5 {
                        s.push_str("...");
                        break;
                    }
                    s.push_str(&v.to_string());
                }
                s.push_str(" ]");

                s
            }
        }

        fn value<E>(e: &Entry, endian: Endian) -> String
        where
            E: ByteOrder,
        {
            use crate::tiff::exif::{Rational, SRational};

            match TagType::try_from(e.type_) {
                Ok(TagType::Ascii) => e.value::<String, E>().map(|v| format!("\"{}\"", v)),
                Ok(TagType::Byte) => e.value_array::<u8>(endian).as_ref().map(array_to_str),
                Ok(TagType::SByte) => e.value_array::<i8>(endian).as_ref().map(array_to_str),
                Ok(TagType::Short) | Ok(TagType::Long) => {
                    e.uint_value_array(endian).as_ref().map(array_to_str)
                }
                Ok(TagType::SShort) | Ok(TagType::SLong) => {
                    e.int_value_array(endian).as_ref().map(array_to_str)
                }
                Ok(TagType::Rational) => {
                    e.value_array::<Rational>(endian).as_ref().map(array_to_str)
                }
                Ok(TagType::SRational) => e
                    .value_array::<SRational>(endian)
                    .as_ref()
                    .map(array_to_str),
                Err(_) => None,
                _ => Some("VALUE".to_string()),
            }
            .or_else(|| Some("ERROR".to_string()))
            .unwrap()
        }

        let type_: &str = TagType::try_from(self.type_)
            .map(|t| t.into())
            .unwrap_or("ERROR");
        let tag_name = args.get("tag_name").cloned().unwrap_or_default();
        let value = match endian {
            Endian::Little => value::<LittleEndian>(self, endian),
            Endian::Big => value::<BigEndian>(self, endian),
            _ => "NO ENDIAN".to_string(),
        };
        dump_println!(
            indent,
            "<0x{:04x}={:>5}> {:<30} [{:>2}={:<10} {}] = {}",
            self._id,
            self._id,
            tag_name,
            self.type_,
            type_,
            self.count,
            value
        );
    }
}

#[cfg(test)]
mod test {
    use byteorder::{BigEndian, ByteOrder, LittleEndian};

    use super::Entry;

    use crate::container::Endian;
    use crate::tiff::exif::TagType;
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

        // test Ascii to `String`
        let e = Entry::new(0, TagType::Ascii as i16, 4, *b"asci");
        assert_eq!(
            e.value::<String, LittleEndian>(),
            Some(String::from("asci"))
        );

        // test Ascii with trailing NUL to `String`
        let e = Entry::new(0, TagType::Ascii as i16, 4, *b"asc\0");
        assert_eq!(e.value::<String, LittleEndian>(), Some(String::from("asc")));

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
        assert_eq!(e.uint_value::<LittleEndian>(), None);

        // Undefined
        let e = Entry::new(0, TagType::Undefined as i16, 4, [4, 0, 8, 0]);
        let r = e.value_array::<u16>(Endian::Little);
        assert_eq!(r, Some(vec![4_u16, 8]));
        // Testing round of data
        let e = Entry::new(0, TagType::Undefined as i16, 3, [4, 0, 0, 0]);
        let r = e.value_array::<u16>(Endian::Little);
        assert_eq!(r, Some(vec![4_u16]));
    }

    #[test]
    fn test_uint_value() {
        // uint_value
        let e = Entry::new(0, TagType::Short as i16, 2, [4, 0, 3, 0]);
        assert_eq!(e.uint_value::<LittleEndian>(), Some(4));
        assert_eq!(e.uint_value_at_index::<LittleEndian>(1), Some(3));
        // out of range
        assert_eq!(e.uint_value_at_index::<LittleEndian>(2), None);
        assert_eq!(e.uint_value_array(Endian::Little), Some(vec![4, 3]),);

        let e = Entry::new(0, TagType::Long as i16, 1, [4, 0, 0, 0]);
        assert_eq!(e.uint_value::<LittleEndian>(), Some(4));
        // incorrect type
        let e = Entry::new(0, TagType::SLong as i16, 1, [4, 0, 0, 0]);
        assert_eq!(e.uint_value::<LittleEndian>(), None);
    }
}
