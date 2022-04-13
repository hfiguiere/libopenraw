/*
 * libopenraw - factory.rs
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

//! Factory for RAW files.

use std::collections::HashMap;

use super::rawfile::RawFileFactory;
use super::Type;

use crate::canon::Cr2File;
use crate::canon::Cr3File;
use crate::dng::DngFile;
use crate::epson::ErfFile;
use crate::fujifilm::RafFile;
use crate::panasonic::Rw2File;

lazy_static::lazy_static! {
    /// Factory map. This is where new types are registered.
    static ref FACTORY_MAP: HashMap<Type, RawFileFactory> = HashMap::from([
        (Type::Cr2, Cr2File::factory as RawFileFactory),
        (Type::Cr3, Cr3File::factory as RawFileFactory),
        (Type::Dng, DngFile::factory as RawFileFactory),
        (Type::Erf, ErfFile::factory as RawFileFactory),
        (Type::Gpr, DngFile::factory as RawFileFactory),
        (Type::Raf, RafFile::factory as RawFileFactory),
        (Type::Rw2, Rw2File::factory as RawFileFactory),
    ]);
}

/// Get the factory for a type.
pub(crate) fn get_raw_file_factory(t: Type) -> Option<&'static RawFileFactory> {
    FACTORY_MAP.get(&t)
}
