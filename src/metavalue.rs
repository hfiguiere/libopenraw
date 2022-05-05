// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - metavalue.rs
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

pub enum Value {
    UInt(u64),
    SInt(i64),
    Float(f64),
    Byte(u8),
    SByte(i8),
    Str(String),
}

/// Metadata value
pub struct MetaValue {
    values: Vec<Value>,
}

impl MetaValue {

    /// Return the item count
    pub fn count(&self) -> usize {
        self.values.len()
    }
}

impl From<&Entry> for MetaValue {
    fn from(e: &entry) -> MetaValue {
        
    }
}



