/*
 * libopenraw - lib.rs
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

mod container;
mod cr3;
pub mod exif;
mod factory;
mod identify;
pub mod ifd;
mod io;
mod mp4;
mod raf;
mod rawfile;
mod thumbnail;

pub use rawfile::RawFile;
pub use rawfile::RawFileImpl;

pub use rawfile::raw_file_from_file;

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
    /// MP4 parse error. Can't use native error as it doesn't do `PartialEq`
    Mp4Parse(String),
}

impl From<std::io::Error> for Error {
    fn from(err: std::io::Error) -> Error {
        Error::IoError(err.to_string())
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
            Self::IoError(ref err) => write!(f, "IO Error: {}", err),
            Self::FormatError => write!(f, "Format error"),
            Self::Mp4Parse(ref err) => write!(f, "MP4 Parse Error: {}", err),
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
    /// RAW data compressed. (undetermined codec)
    CompressedRaw,
}

/// RAW file type. This list the type of files, which
/// coincidentally match the vendor, except for DNG.
///
#[derive(Clone, Copy, Debug, Eq, Hash, PartialEq)]
pub enum Type {
    /// Sony Alpha
    Arw,
    /// Canon RAW CIFF based
    Crw,
    /// Canon RAW TIFF based
    Cr2,
    /// Canon RAW MPEG4 ISO based
    Cr3,
    /// Adobe Digital Negative
    Dng,
    /// Epson RAW
    Erf,
    /// GoPro RAW
    Gpr,
    /// Minolta RAW
    Mrw,
    /// Nikon RAW
    Nef,
    /// Nikon RAW (NRW variant)
    Nrw,
    /// Olympus RAW
    Orf,
    /// Pentax RAW
    Pef,
    /// Fujfilm RAW
    Raf,
    /// Panasonic RAW (old)
    Raw,
    /// Panasonic RAW
    Rw2,
    /// Sony RAW (old)
    Sr2,
    #[cfg(test)]
    /// Value for testing only
    Test,
}

/// Type ID
pub type TypeId = u32;
