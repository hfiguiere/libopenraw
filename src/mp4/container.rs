/*
 * libopenraw - mp4/container.rs
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

use crate::container;
use crate::io::View;

pub(crate) struct Container {
    view: View,
}

impl container::Container for Container {
    fn endian() -> container::Endian {
        container::Endian::BigEndian
    }
}

impl Container {
    pub fn new(view: View) -> Self {
        Self { view }
    }
}
