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

use std::collections::HashMap;

use super::rawfile::RawFileFactory;
use super::Type;
use crate::canon::Cr3File;

lazy_static::lazy_static! {
    static ref FACTORY_MAP: HashMap<Type, RawFileFactory> = {
        let mut m = HashMap::new();

        m.insert(Type::Cr3, Cr3File::factory as RawFileFactory);

        m
    };
}

pub fn get_raw_file_factory(t: Type) -> Option<&'static RawFileFactory> {
    FACTORY_MAP.get(&t)
}
