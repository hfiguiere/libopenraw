/*
 * libopenraw - identify.rs
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
use std::ffi::{OsStr, OsString};

use super::{Error, Result, Type};
use crate::raf;
use crate::rawfile::ReadAndSeek;

lazy_static::lazy_static! {
    static ref EXT_TO_TYPE: HashMap<OsString, Type> = {
        let mut m = HashMap::new();
        // The extension MUST be lowercase
        m.insert(OsString::from("cr3"), Type::Cr3);

        m
    };
}

/// Get the type associated to the extension.
/// `ext` must be lowercase ASCII.
pub(crate) fn type_for_extension(ext: &OsStr) -> Option<Type> {
    EXT_TO_TYPE.get(ext).cloned()
}

/// Return the `Type` based on the content of the file.
pub(crate) fn type_for_content(content: &mut dyn ReadAndSeek) -> Result<Option<Type>> {
    use crate::Type::*;

    // Buffer to read the content to identify
    // Size is max of (14, RAF_MAGIC.len())
    // Change as needed
    let mut buf = [0_u8; 16];

    let len = content.read(&mut buf)?;
    if len <= 4 {
        return Err(Error::BufferTooSmall);
    }

    if &buf[0..4] == b"\0MRM" {
        return Ok(Some(Mrw));
    }
    if len >= 12 && &buf[4..12] == b"ftypcrx " {
        return Ok(Some(Cr3));
    }
    if len >= 14 && &buf[0..14] == b"II\x1a\0\0\0HEAPCCDR" {
        return Ok(Some(Crw));
    }
    if &buf[0..4] == b"IIRO" {
        return Ok(Some(Orf));
    }
    if &buf[0..4] == b"IIU\0" {
        return Ok(Some(Rw2));
    }
    if len >= raf::RAF_MAGIC.len() && &buf[0..raf::RAF_MAGIC.len()] == raf::RAF_MAGIC {
        return Ok(Some(Raf));
    }
    if &buf[0..4] == b"II\x2a\0" || &buf[0..4] == b"MM\0\x2a" {
        // TIFF based format
        if len >= 12 {
            if &buf[8..11] == b"CR\x02" {
                return Ok(Some(Cr2));
            }
        }
        if len >= 8 {
            // XXX missing the TIFF part.
        }
    }

    Ok(None)
}

#[cfg(test)]
mod test {
    #[test]
    fn test_type_for_extension() {
        use std::ffi::OsString;

        use super::type_for_extension;
        use crate::Type;

        assert_eq!(type_for_extension(&OsString::from("CR3")), None);
        assert_eq!(type_for_extension(&OsString::from("cr3")), Some(Type::Cr3));
        assert_eq!(type_for_extension(&OsString::from("NOPE")), None);
    }

    #[test]
    fn test_type_for_content() {
        use super::type_for_content;
        use crate::{Error, Type};
        use std::io::Cursor;

        let mut four_bytes = Cursor::new([0_u8; 4].as_slice());
        assert_eq!(
            type_for_content(&mut four_bytes),
            Err(Error::BufferTooSmall)
        );

        // Canon
        let mut crw = Cursor::new(include_bytes!("../testdata/identify/content_crw").as_slice());
        assert_eq!(type_for_content(&mut crw), Ok(Some(Type::Crw)));

        let mut cr2 = Cursor::new(include_bytes!("../testdata/identify/content_cr2").as_slice());
        assert_eq!(type_for_content(&mut cr2), Ok(Some(Type::Cr2)));

        let mut cr3 = Cursor::new(include_bytes!("../testdata/identify/content_cr3").as_slice());
        assert_eq!(type_for_content(&mut cr3), Ok(Some(Type::Cr3)));

        let mut mrw = Cursor::new(include_bytes!("../testdata/identify/content_mrw").as_slice());
        assert_eq!(type_for_content(&mut mrw), Ok(Some(Type::Mrw)));

        let mut raf = Cursor::new(include_bytes!("../testdata/identify/content_raf").as_slice());
        assert_eq!(type_for_content(&mut raf), Ok(Some(Type::Raf)));
    }
}
