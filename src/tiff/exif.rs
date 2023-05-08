// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - tiff/exif.rs
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

//! Exif support

/// Exif tags constants
mod tags;
pub use tags::*;

use byteorder::ByteOrder;

use crate::utils;

/// Type a tag. See `tiff::Entry`.
#[derive(Debug, PartialEq)]
#[repr(i16)]
pub enum TagType {
    Byte = 1,
    Ascii = 2,
    Short = 3,
    Long = 4,
    Rational = 5,
    SByte = 6,
    Undefined = 7,
    SShort = 8,
    SLong = 9,
    SRational = 10,
    Float = 11,
    Double = 12,
    Invalid = 13,
}

impl std::convert::TryFrom<i16> for TagType {
    type Error = &'static str;
    fn try_from(value: i16) -> Result<Self, Self::Error> {
        if value <= 0 || value > 13 {
            Err("Value out of range")
        } else {
            Ok(unsafe { std::mem::transmute(value) })
        }
    }
}

impl std::convert::From<TagType> for &'static str {
    fn from(tag_type: TagType) -> Self {
        use TagType::*;
        match tag_type {
            Byte => "BYTE",
            Ascii => "ASCII",
            Short => "SHORT",
            Long => "LONG",
            Rational => "RATIONAL",
            SByte => "SBYTE",
            Undefined => "UNDEFINED",
            SShort => "SSHORT",
            SLong => "SLONG",
            SRational => "SRATIONAL",
            Float => "FLOAT",
            Double => "DOUBLE",
            Invalid => "INVALID",
        }
    }
}

/// Return the size of a unit for the tag type
pub fn tag_unit_size(tag_type: TagType) -> usize {
    use TagType::*;

    match tag_type {
        Byte | SByte | Ascii | Undefined | Invalid => 1,
        Short | SShort => 2,
        Long | SLong | Float => 4,
        Rational | SRational | Double => 8,
    }
}

/// Trait for ExifValue conversion
pub trait ExifValue {
    /// Return the `TagType` for the type.
    fn exif_type() -> TagType;

    /// Endian dependent reading of the data.
    fn read<E>(buf: &[u8]) -> Self
    where
        E: ByteOrder;

    /// How many bytes does the unit take.
    fn unit_size() -> usize
    where
        Self: Sized,
    {
        std::mem::size_of::<Self>()
    }

    /// The type is array. Default is false.
    /// `Ascii` turns to a `String` so it is an array.
    /// Same for `Undefined`.
    fn is_array() -> bool {
        false
    }
}

impl ExifValue for u8 {
    fn exif_type() -> TagType {
        TagType::Byte
    }

    fn read<E>(buf: &[u8]) -> Self
    where
        E: ByteOrder,
    {
        buf[0]
    }
}

impl ExifValue for i8 {
    fn exif_type() -> TagType {
        TagType::SByte
    }

    fn read<E>(buf: &[u8]) -> Self
    where
        E: ByteOrder,
    {
        buf[0] as i8
    }
}

impl ExifValue for u16 {
    fn exif_type() -> TagType {
        TagType::Short
    }

    fn read<E>(buf: &[u8]) -> Self
    where
        E: ByteOrder,
    {
        E::read_u16(buf)
    }
}

impl ExifValue for i16 {
    fn exif_type() -> TagType {
        TagType::SShort
    }

    fn read<E>(buf: &[u8]) -> Self
    where
        E: ByteOrder,
    {
        E::read_i16(buf)
    }
}

impl ExifValue for u32 {
    fn exif_type() -> TagType {
        TagType::Long
    }

    fn read<E>(buf: &[u8]) -> Self
    where
        E: ByteOrder,
    {
        E::read_u32(buf)
    }
}

impl ExifValue for i32 {
    fn exif_type() -> TagType {
        TagType::SLong
    }

    fn read<E>(buf: &[u8]) -> Self
    where
        E: ByteOrder,
    {
        E::read_i32(buf)
    }
}

impl ExifValue for f32 {
    fn exif_type() -> TagType {
        TagType::Float
    }

    fn read<E>(buf: &[u8]) -> Self
    where
        E: ByteOrder,
    {
        E::read_f32(buf)
    }
}

impl ExifValue for f64 {
    fn exif_type() -> TagType {
        TagType::Double
    }

    fn read<E>(buf: &[u8]) -> Self
    where
        E: ByteOrder,
    {
        E::read_f64(buf)
    }
}

impl ExifValue for String {
    fn exif_type() -> TagType {
        TagType::Ascii
    }

    fn unit_size() -> usize {
        1
    }

    fn is_array() -> bool {
        true
    }

    fn read<E>(buf: &[u8]) -> Self {
        // According to the Exif spec, the string is NUL terminated
        utils::from_maybe_nul_terminated(buf)
    }
}

impl ExifValue for Vec<u8> {
    fn exif_type() -> TagType {
        TagType::Undefined
    }

    fn unit_size() -> usize {
        1
    }

    fn is_array() -> bool {
        true
    }

    fn read<E>(buf: &[u8]) -> Self
    where
        E: ByteOrder,
    {
        Self::from(buf)
    }
}

/// Unsigned rational number (fraction)
pub struct Rational {
    pub num: u32,
    pub denom: u32,
}

impl ExifValue for Rational {
    fn exif_type() -> TagType {
        TagType::Rational
    }

    fn unit_size() -> usize {
        std::mem::size_of::<u32>() * 2
    }

    fn read<E>(buf: &[u8]) -> Self
    where
        E: ByteOrder,
    {
        Rational {
            num: E::read_u32(buf),
            denom: E::read_u32(&buf[4..]),
        }
    }
}

impl ToString for Rational {
    fn to_string(&self) -> String {
        format!("{}/{}", self.num, self.denom)
    }
}

/// Signed rational number (fraction)
pub struct SRational {
    pub num: i32,
    pub denom: i32,
}

impl From<&SRational> for f64 {
    fn from(r: &SRational) -> f64 {
        if r.denom != 0 {
            r.num as f64 / r.denom as f64
        } else {
            f64::NAN
        }
    }
}

impl ExifValue for SRational {
    fn exif_type() -> TagType {
        TagType::SRational
    }

    fn unit_size() -> usize {
        std::mem::size_of::<i32>() + std::mem::size_of::<i32>()
    }

    fn read<E>(buf: &[u8]) -> Self
    where
        E: ByteOrder,
    {
        SRational {
            num: E::read_i32(buf),
            denom: E::read_i32(&buf[4..]),
        }
    }
}

impl ToString for SRational {
    fn to_string(&self) -> String {
        format!("{}/{}", self.num, self.denom)
    }
}

/// Exif photometric interpretation
#[repr(u16)]
#[derive(Debug)]
pub enum PhotometricInterpretation {
    BlackIsZero = 1,
    Rgb = 2,
    YCbCr = 6,

    // RAW only
    CFA = 32803,
    LinearRaw = 34892,
}

impl std::convert::TryFrom<u32> for PhotometricInterpretation {
    type Error = crate::Error;

    fn try_from(v: u32) -> crate::Result<PhotometricInterpretation> {
        use PhotometricInterpretation::*;
        match v {
            1 => Ok(BlackIsZero),
            2 => Ok(Rgb),
            6 => Ok(YCbCr),
            32803 => Ok(CFA),
            34892 => Ok(LinearRaw),
            _ => Err(Self::Error::InvalidFormat),
        }
    }
}

#[cfg(test)]
mod test {

    use std::convert::TryFrom;

    use byteorder::LittleEndian;

    use super::{ExifValue, Rational, SRational, TagType};

    #[test]
    fn test_tag_type_convert() {
        let tag = TagType::try_from(1);
        assert_eq!(tag, Ok(TagType::Byte));

        let tag = TagType::try_from(4);
        assert_eq!(tag, Ok(TagType::Long));

        // Invalid value
        let tag = TagType::try_from(-1);
        assert!(tag.is_err());

        // Invalid value
        let tag = TagType::try_from(42);
        assert!(tag.is_err());
    }

    #[test]
    fn test_rational() {
        let r = SRational { num: 10, denom: 5 };
        let f: f64 = (&r).into();
        assert_eq!(f, 2.0);

        let r = SRational { num: -10, denom: 5 };
        let f: f64 = (&r).into();
        assert_eq!(f, -2.0);

        let buf = [10_u8, 0, 0, 0, 5, 0, 0, 0];

        let r = SRational::read::<LittleEndian>(buf.as_slice());
        assert_eq!(r.num, 10);
        assert_eq!(r.denom, 5);

        let r = Rational::read::<LittleEndian>(buf.as_slice());
        assert_eq!(r.num, 10);
        assert_eq!(r.denom, 5);
    }
}
