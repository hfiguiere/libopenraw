// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - capi/mod.rs
 *
 * Copyright (C) 2023 Hubert Figuière
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

//! C API for libopenraw
//!
//! This module will implement all the C API for libopenraw
//! It will only be built if the feature `capi` is enabled.
//!
//! By default the autotools build system will generate libopenraw.so
//! and the associated pkg-config files that can be used like in the
//! C++ version.
//!
//! To preserve the consistency of the API, some of the symbols in this
//! module don't follow Rust conventions.
//!
//! The goal is to expose all the functionality of the library.

#![warn(missing_docs)]

use crate::mosaic::Pattern;
use crate::Error;

#[repr(C)]
#[allow(non_camel_case_types)]
#[allow(dead_code)]
#[allow(clippy::upper_case_acronyms)]
#[derive(Debug, Eq, PartialEq)]
/// Error types.
pub enum or_error {
    /// No error.
    NONE = 0,
    /// The buffer is too small
    BUF_TOO_SMALL = 1,
    /// The expected pointer / ref is invalid.
    NOT_AREF = 2,
    /// Not found.
    NOT_FOUND = 5,
    /// Invalid parameter
    INVALID_PARAM = 6,
    /// An unknown error.
    UNKNOWN = 42,
}

impl From<Error> for or_error {
    fn from(err: Error) -> or_error {
        match err {
            Error::NotFound => or_error::NOT_FOUND,
            _ => or_error::UNKNOWN,
        }
    }
}

impl From<or_error> for Error {
    fn from(err: or_error) -> Error {
        match err {
            or_error::NONE => Error::None,
            or_error::BUF_TOO_SMALL => Error::BufferTooSmall,
            or_error::NOT_AREF => Error::InvalidAddress,
            or_error::NOT_FOUND => Error::NotFound,
            or_error::INVALID_PARAM => Error::InvalidParam,
            or_error::UNKNOWN => Error::Unknown,
        }
    }
}

#[repr(C)]
#[allow(non_camel_case_types)]
#[allow(clippy::upper_case_acronyms)]
/// The CFA pattern type.
pub enum or_cfa_pattern {
    /// Invalid value
    NONE = 0,
    /// Non RGB 2x2 CFA
    NON_RGB22 = 1,
    /// RGGB 2x2 pattern
    RGGB = 2,
    /// GBRG 2x2 pattern
    GBRG = 3,
    /// BGGR 2x2 pattern
    BGGR = 4,
    /// GRBG 2x2 pattern
    GRBG = 5,
}

impl From<&Pattern> for or_cfa_pattern {
    fn from(pattern: &Pattern) -> Self {
        match *pattern {
            Pattern::Empty => Self::NONE,
            Pattern::NonRgb22(_) => Self::NON_RGB22,
            Pattern::Rggb => Self::RGGB,
            Pattern::Gbrg => Self::GBRG,
            Pattern::Bggr => Self::BGGR,
            Pattern::Grbg => Self::GRBG,
        }
    }
}
