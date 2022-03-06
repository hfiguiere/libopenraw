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

pub struct Entry {
    id: u16,
    type_: i16,
    count: i32,
    data: [u8; 4],
}

impl Entry {
    pub fn new(id: u16, type_: i16, count: i32, data: [u8; 4]) -> Self {
        Entry {
            id,
            type_,
            count,
            data,
        }
    }
}
