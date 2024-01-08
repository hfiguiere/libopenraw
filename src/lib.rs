// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - lib.rs
 *
 * Copyright (C) 2022-2024 Hubert Figuière
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

// For mp4::parse
#[macro_use]
extern crate log;

#[macro_use]
mod dump;
#[macro_use]
mod utils;

mod apple;
mod bitmap;
mod camera_ids;
mod canon;
mod capi;
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
pub mod metadata;
mod minolta;
mod mosaic;
mod mp4;
mod nikon;
mod olympus;
mod panasonic;
mod pentax;
mod rawfile;
mod rawimage;
mod render;
mod ricoh;
mod sigma;
mod sony;
mod thumbnail;
pub mod tiff;

pub use bitmap::{Bitmap, Image, Point, Rect, Size};
pub use colour::ColourSpace;
pub use dump::Dump;
pub use mosaic::Pattern as CfaPattern;
pub use rawfile::{RawFile, RawFileHandle, RawFileImpl};
pub use rawimage::RawImage;
pub use render::{RenderingOptions, RenderingStage};
pub use thumbnail::Thumbnail;
pub use tiff::Ifd;

#[cfg(any(feature = "fuzzing", feature = "bench"))]
pub use decompress::LJpeg;
#[cfg(any(feature = "fuzzing", feature = "bench"))]
pub use olympus::decompress::decompress_olympus;

pub use rawfile::rawfile_from_file;
pub use rawfile::rawfile_from_io;
pub use rawfile::rawfile_from_memory;

/// Standard Result for libopenraw
pub type Result<T> = std::result::Result<T, Error>;

use mp4::parse as mp4parse;

/// Standard `Error` for libopenraw
#[derive(Debug, thiserror::Error)]
pub enum Error {
    // No error. For compatibility with `capi::or_error`
    #[error("No error")]
    None,
    /// This is unimplemented
    #[error("Unimplemented")]
    Unimplemented,
    /// File format is unrecognized
    #[error("Unrecognized format")]
    UnrecognizedFormat,
    /// Not supported
    #[error("Operation not supported")]
    NotSupported,
    /// Not found in file
    #[error("Data not found")]
    NotFound,
    /// Buffer too small: we expect a bigger amount of data
    #[error("Buffer is too small")]
    BufferTooSmall,
    /// Unexpected end of file
    #[error("Unexpected end-of-file")]
    UnexpectedEOF,
    /// IO Error
    #[error("IO Error: {0}")]
    IoError(#[from] std::io::Error),
    /// Error parsing format
    #[error("Format error")]
    FormatError,
    /// Already inited
    #[error("Already Inited")]
    AlreadyInited,
    /// Invalid parameter
    #[error("Invalid parameter")]
    InvalidParam,
    /// Invalid format: wrong kind of data found
    #[error("Invalid format")]
    InvalidFormat,
    /// Decompression error.
    #[error("Decompression error: {0}")]
    Decompression(String),
    /// MP4 parse error
    #[error("MP4 Parse Error: {0}")]
    Mp4Parse(#[from] mp4parse::Error),
    /// Jpeg decompress
    #[error("JPEG error: {0}")]
    JpegFormat(String),
    /// Bit reader error.
    #[error("BitReader error: {0}")]
    BitReaderError(#[from] bitreader::BitReaderError),
    #[error("Invalid address")]
    InvalidAddress,
    /// Other error.
    #[error("Other error: {0}")]
    Other(String),
    /// Unknown error: placeholder for anything else.
    #[error("Unknown error")]
    Unknown,
}

/// What type is the data.
///
#[derive(Clone, Copy, Debug, Default, PartialEq)]
pub enum DataType {
    /// JPEG stream
    Jpeg,
    /// RGB8 Pixmap
    PixmapRgb8,
    /// RGB8 Pixmap
    PixmapRgb16,
    /// RAW data compressed. (undetermined codec)
    CompressedRaw,
    /// RAW data uncompressed
    Raw,
    /// Unknown type
    #[default]
    Unknown,
}

impl From<&str> for DataType {
    fn from(s: &str) -> DataType {
        match s {
            "JPEG" => Self::Jpeg,
            "8RGB" => Self::PixmapRgb8,
            "16RGB" => Self::PixmapRgb16,
            "COMP_RAW" => Self::CompressedRaw,
            "RAW" => Self::Raw,
            _ => Self::Unknown,
        }
    }
}

impl From<DataType> for String {
    fn from(t: DataType) -> String {
        match t {
            DataType::Jpeg => "JPEG",
            DataType::Raw => "RAW",
            DataType::PixmapRgb8 => "8RGB",
            DataType::PixmapRgb16 => "16RGB",
            DataType::CompressedRaw => "COMP_RAW",
            DataType::Unknown => "UNKNOWN",
        }
        .to_string()
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
    /// Panasonic old RAW
    Rw = 17,
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
            "GPR" => Self::Gpr,
            "MRW" => Self::Mrw,
            "NEF" => Self::Nef,
            "NRW" => Self::Nrw,
            "ORF" => Self::Orf,
            "PEF" => Self::Pef,
            "RAF" => Self::Raf,
            "RW2" => Self::Rw2,
            "SR2" => Self::Sr2,
            _ => Self::Unknown,
        }
    }
}

impl From<Type> for String {
    fn from(t: Type) -> String {
        match t {
            Type::Arw => "ARW",
            Type::Cr2 => "CR2",
            Type::Cr3 => "CR3",
            Type::Crw => "CRW",
            Type::Dng => "DNG",
            Type::Erf => "ERF",
            Type::Gpr => "GPR",
            Type::Jpeg => "JPEG",
            Type::Mrw => "MRW",
            Type::Nef => "NEF",
            Type::Nrw => "NRW",
            Type::Orf => "ORF",
            Type::Pef => "PEF",
            Type::Raf => "RAF",
            Type::Rw => "RW",
            Type::Rw2 => "RW2",
            Type::Sr2 => "SR2",
            #[cfg(test)]
            Type::Test => "TEST",
            Type::Tiff => "TIF",
            Type::Unknown => "UNKNOWN",
        }
        .to_string()
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
