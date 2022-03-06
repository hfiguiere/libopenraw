/*
 * libopenraw - ifd.rs
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
mod dir;
mod entry;

pub(crate) use container::Container;
pub(crate) use dir::Dir;
pub(crate) use entry::Entry;

/// Type of IFD
#[derive(Clone, Copy)]
pub enum Type {
    /// Main IFD (see TIFF)
    Main,
    /// CFA specific IFD
    Cfa,
    /// Exif IFD
    Exif,
    /// MakerNote IFD
    MakerNote,
    /// Any other IFD
    Other,
}
