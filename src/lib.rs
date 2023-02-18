// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - lib.rs
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

#[macro_use]
mod dump;

mod apple;
mod bitmap;
mod camera_ids;
mod canon;
mod colour;
mod container;
mod decompress;
mod dng;
mod epson;
mod factory;
mod fujifilm;
mod identify;
mod io;
mod jpeg;
mod leica;
mod mosaic;
mod mp4;
mod nikon;
mod olympus;
mod panasonic;
mod pentax;
mod rawdata;
mod rawfile;
mod ricoh;
mod sigma;
mod sony;
mod thumbnail;
pub mod tiff;
mod utils;

pub use bitmap::{Bitmap, Rect};
pub use dump::Dump;
pub use rawdata::RawData;
pub use rawfile::RawFile;
pub use rawfile::RawFileImpl;
pub use thumbnail::Thumbnail;
pub use tiff::Ifd;

#[cfg(any(feature = "fuzzing", feature = "bench"))]
pub use decompress::LJpeg;
#[cfg(any(feature = "fuzzing", feature = "bench"))]
pub use olympus::decompress::decompress_olympus;

pub use rawfile::rawfile_from_file;
pub use rawfile::rawfile_from_io;

/// Standard Result for libopenraw
pub type Result<T> = std::result::Result<T, Error>;

/// Standard Error for libopenraw
#[derive(Debug, PartialEq)]
pub enum Error {
    /// File format is unrecognized
    UnrecognizedFormat,
    /// Not supported
    NotSupported,
    /// Not found in file
    NotFound,
    /// Buffer too small: we expect a bigger amount of data
    BufferTooSmall,
    /// Unextepected end of file
    UnexpectedEOF,
    /// IO Error
    IoError(String),
    /// Error parsing format
    FormatError,
    /// Already inited
    AlreadyInited,
    /// Invalid parameter
    InvalidParam,
    /// Invalid format: wrong kind of data found
    InvalidFormat,
    /// Decompression error.
    Decompression(String),
    /// MP4 parse error. Can't use native error as it doesn't do `PartialEq`
    Mp4Parse(String),
    /// Jpeg decompress
    JpegFormat(String),
    /// Bit reader error.
    BitReaderError(bitreader::BitReaderError),
    /// Unknown error: placeholder for anything else.
    Unknown,
}

impl From<std::io::Error> for Error {
    fn from(err: std::io::Error) -> Error {
        Error::IoError(err.to_string())
    }
}

impl From<bitreader::BitReaderError> for Error {
    fn from(err: bitreader::BitReaderError) -> Error {
        Error::BitReaderError(err)
    }
}

impl From<mp4parse::Error> for Error {
    fn from(err: mp4parse::Error) -> Error {
        Error::Mp4Parse(err.to_string())
    }
}

impl std::fmt::Display for Error {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match *self {
            Self::UnrecognizedFormat => write!(f, "Unrecognized format"),
            Self::NotSupported => write!(f, "Operation not supported"),
            Self::NotFound => write!(f, "Data not found"),
            Self::BufferTooSmall => write!(f, "Buffer is too small"),
            Self::UnexpectedEOF => write!(f, "Unexpected end-of-file"),
            Self::IoError(ref err) => write!(f, "IO Error: {err}"),
            Self::FormatError => write!(f, "Format error"),
            Self::AlreadyInited => write!(f, "Already Inited"),
            Self::InvalidParam => write!(f, "Invalid parameter"),
            Self::InvalidFormat => write!(f, "Invalid format"),
            Self::Decompression(ref reason) => write!(f, "Decompression error: {reason}"),
            Self::Mp4Parse(ref err) => write!(f, "MP4 Parse Error: {err}"),
            Self::JpegFormat(ref err) => write!(f, "JPEG error: {err}"),
            Self::BitReaderError(ref err) => write!(f, "BitReader error: {err}"),
            Self::Unknown => write!(f, "Unknown error"),
        }
    }
}

impl std::error::Error for Error {}

/// What type is the data.
///
#[derive(Clone, Copy, Debug, PartialEq)]
pub enum DataType {
    /// JPEG stream
    Jpeg,
    /// RGB8 Pixmap
    PixmapRgb8,
    /// RAW data compressed. (undetermined codec)
    CompressedRaw,
    /// RAW data uncompressed
    Raw,
    /// Unknown type
    Unknown,
}

impl From<&str> for DataType {
    fn from(s: &str) -> DataType {
        match s {
            "JPEG" => Self::Jpeg,
            "8RGB" => Self::PixmapRgb8,
            "COMP_RAW" => Self::CompressedRaw,
            "RAW" => Self::Raw,
            _ => Self::Unknown,
        }
    }
}

/// RAW file type. This list the type of files, which
/// coincidentally match the vendor, except for DNG.
///
/// The value match the enum on the C++ headers.
#[derive(Clone, Copy, Debug, Eq, Hash, PartialEq)]
#[repr(u32)]
pub enum Type {
    /// Unknown vendor
    Unknown = 0,
    /// Sony Alpha
    Arw = 5,
    /// Canon RAW CIFF based
    Crw = 2,
    /// Canon RAW TIFF based
    Cr2 = 1,
    /// Canon RAW MPEG4 ISO based
    Cr3 = 14,
    /// Adobe Digital Negative
    Dng = 6,
    /// Epson RAW
    Erf = 9,
    /// GoPro RAW
    Gpr = 15,
    /// Minolta RAW
    Mrw = 4,
    /// Nikon RAW
    Nef = 3,
    /// Nikon RAW (NRW variant)
    Nrw = 11,
    /// Olympus RAW
    Orf = 7,
    /// Pentax RAW
    Pef = 8,
    /// Fujfilm RAW
    Raf = 13,
    /// Panasonic RAW
    Rw2 = 12,
    /// Sony RAW (old)
    Sr2 = 16,
    /// JPEG (definitely not a Raw)
    Jpeg = 100,
    Tiff = 10,
    #[cfg(test)]
    /// Value for testing only
    Test = 200,
}

impl From<&str> for Type {
    fn from(s: &str) -> Type {
        match s {
            "ARW" => Self::Arw,
            "CR2" => Self::Cr2,
            "CR3" => Self::Cr3,
            "CRW" => Self::Crw,
            "DNG" => Self::Dng,
            "ERF" => Self::Erf,
            "NEF" => Self::Nef,
            "NRW" => Self::Nrw,
            "ORF" => Self::Orf,
            "PEF" => Self::Pef,
            "RAF" => Self::Raf,
            "RW2" => Self::Rw2,
            _ => Self::Unknown,
        }
    }
}

/// Type ID (vendor, model)
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct TypeId(u16, u16);

impl std::fmt::Display for TypeId {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{} {}", self.0, self.1)
    }
}

impl Default for TypeId {
    fn default() -> Self {
        TypeId(camera_ids::vendor::NONE, camera_ids::generic::UNKNOWN)
    }
}

impl From<u32> for TypeId {
    fn from(id: u32) -> TypeId {
        TypeId(((id & 0xffff0000) >> 16) as u16, (id & 0xffff) as u16)
    }
}
impl From<TypeId> for u32 {
    fn from(type_id: TypeId) -> u32 {
        ((type_id.0 as u32) << 16) | type_id.1 as u32
    }
}

#[cfg(test)]
mod test {
    use super::TypeId;

    #[test]
    fn test_typeid_from_u32() {
        let id = 0x0001_0042;
        assert_eq!(TypeId(1, 0x42), TypeId::from(id));
    }

    #[test]
    fn test_typeid_to_u32() {
        let id: u32 = TypeId(1, 0x42).into();
        assert_eq!(id, 0x0001_0042);
    }
}
