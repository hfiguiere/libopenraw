// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - identify.rs
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

//! Indentification of RAW files.

use std::collections::HashMap;
use std::io::{Read, Seek, SeekFrom};
use std::iter::FromIterator;

use once_cell::sync::Lazy;

use super::{Error, Result, Type};
use crate::fujifilm;
use crate::io::View;
use crate::tiff;
use crate::tiff::{exif, Ifd, IfdType};

const TYPE_MIME: [(Type, &str); 15] = [
    (Type::Arw, "image/x-sony-arw"),
    (Type::Cr2, "image/x-canon-cr2"),
    (Type::Cr3, "image/x-canon-cr3"),
    (Type::Crw, "image/x-canon-crw"),
    (Type::Dng, "image/x-adobe-dng"),
    (Type::Erf, "image/x-epson-erf"),
    (Type::Mrw, "image/x-minolta-mrw"),
    (Type::Nef, "image/x-nikon-nef"),
    (Type::Nrw, "image/x-nikon-nrw"),
    (Type::Orf, "image/x-olympus-orf"),
    (Type::Pef, "image/x-pentax-pef"),
    (Type::Raf, "image/x-fuji-raf"),
    (Type::Rw, "image/x-panasonic-rw"),
    (Type::Rw2, "image/x-panasonic-rw2"),
    (Type::Sr2, "image/x-sony-sr2"),
];

const EXT_TYPE: [(&str, Type); 17] = [
    // The extension MUST be lowercase
    ("arw", Type::Arw),
    ("cr2", Type::Cr2),
    ("cr3", Type::Cr3),
    ("dng", Type::Dng),
    ("erf", Type::Erf),
    ("jpg", Type::Jpeg),
    ("jpeg", Type::Jpeg),
    ("gpr", Type::Gpr),
    ("nef", Type::Nef),
    ("nrw", Type::Nrw),
    ("orf", Type::Orf),
    ("pef", Type::Pef),
    ("raf", Type::Raf),
    ("raw", Type::Rw),
    ("rw2", Type::Rw2),
    ("rwl", Type::Rw2),
    ("sr2", Type::Arw),
];

lazy_static::lazy_static! {
    /// Mapping of extensions (lowercase) to a `Type`.
    pub(crate) static ref EXT_TO_TYPE: HashMap<&'static str, Type> = HashMap::from(
        EXT_TYPE
    );

    pub(crate) static ref TYPE_TO_MIME: HashMap<Type, &'static str> = HashMap::from(
        TYPE_MIME
    );

    pub(crate) static ref MIME_TO_TYPE: HashMap<&'static str, Type> = HashMap::from_iter(
        TYPE_MIME.iter().map(|(t, m)| (*m, *t))
    );
}

static MIME_TYPES: Lazy<Vec<String>> = Lazy::new(|| {
    crate::identify::TYPE_MIME
        .iter()
        .map(|(_, m)| String::from(*m))
        .collect()
});

/// Return the list of supported mimetypes
pub fn mime_types() -> &'static [String] {
    &MIME_TYPES
}

/// Get the mime type associated for the file.
pub(crate) fn mime_for_type(type_: Type) -> Option<&'static str> {
    TYPE_TO_MIME.get(&type_).copied()
}

/// Get the type associated to the extension.
/// `ext` must be lowercase ASCII.
pub(crate) fn type_for_extension(ext: &str) -> Option<Type> {
    EXT_TO_TYPE.get(ext).cloned()
}

/// Get the type associated to the mimetype.
pub(crate) fn type_for_mime_type(mime: &str) -> Option<Type> {
    MIME_TO_TYPE.get(mime).cloned()
}


/// Return the `Type` based on the content of the file.
pub(crate) fn type_for_content(content: &mut View) -> Result<Option<Type>> {
    use crate::Type::*;

    // Buffer to read the content to identify
    // Size is max of (14, RAF_MAGIC.len())
    // Change as needed
    let mut buf = [0_u8; 16];

    let len = content.read(&mut buf)?;
    if len <= 4 {
        return Err(Error::BufferTooSmall);
    }

    if buf[0..4] == [0xff, 0xd8, 0xff, 0xdb] {
        return Ok(Some(Jpeg));
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
    if &buf[0..4] == b"IIRO" || &buf[0..4] == b"IIRS" {
        return Ok(Some(Orf));
    }
    if &buf[0..4] == b"IIU\0" {
        return Ok(Some(Rw2));
    }
    if len >= fujifilm::RAF_MAGIC.len() && &buf[0..fujifilm::RAF_MAGIC.len()] == fujifilm::RAF_MAGIC
    {
        return Ok(Some(Raf));
    }
    if &buf[0..4] == b"II\x2a\0" || &buf[0..4] == b"MM\0\x2a" {
        // TIFF based format
        if len >= 12 && &buf[8..11] == b"CR\x02" {
            return Ok(Some(Cr2));
        }
        if len >= 8 {
            content.seek(SeekFrom::Start(0))?;

            let mut container =
                tiff::Container::new(content.clone(), vec![(IfdType::Main, None)], Type::Unknown);
            container.load(None)?;
            if let Some(dir) = container.directory(0) {
                if dir.entry(exif::TIFF_TAG_DNG_VERSION).is_some() {
                    return Ok(Some(Dng));
                }
                if let Some(make) = dir.value::<String>(exif::EXIF_TAG_MAKE) {
                    //let make = String::from_utf8_lossy(&bytes);
                    if make.contains("NIKON") {
                        return Ok(Some(Nef));
                    } else if &make == "SEIKO EPSON CORP." {
                        return Ok(Some(Erf));
                    } else if &make == "PENTAX Corporation " {
                        return Ok(Some(Pef));
                    } else if make.contains("SONY") {
                        return Ok(Some(Arw));
                    } else if &make == "Canon" {
                        return Ok(Some(Cr2));
                    }
                }
            }
        }
    }

    Ok(None)
}

#[cfg(test)]
mod test {
    #[test]
    fn test_type_for_extension() {
        use super::type_for_extension;
        use crate::Type;

        assert_eq!(type_for_extension("CR3"), None);
        assert_eq!(type_for_extension("cr3"), Some(Type::Cr3));
        assert_eq!(type_for_extension("NOPE"), None);
    }

    #[test]
    fn test_type_for_mime_type() {
        use super::type_for_mime_type;
        use crate::Type;

        assert_eq!(type_for_mime_type("image/x-fuji-raf"), Some(Type::Raf));
        assert_eq!(type_for_mime_type("image/x-canon-cr3"), Some(Type::Cr3));
        assert_eq!(type_for_mime_type("application/octet-stream"), None);
    }

    #[test]
    fn test_type_for_content() {
        use super::type_for_content;
        use crate::{io, Error, Type};
        use std::io::Cursor;

        let four_bytes = Cursor::new([0_u8; 4].as_slice());
        let viewer = io::Viewer::new(Box::new(four_bytes), 0);
        let mut view = io::Viewer::create_view(&viewer, 0).expect("Couldn't create view");
        assert!(matches!(
            type_for_content(&mut view),
            Err(Error::BufferTooSmall)
        ));

        // Canon
        let crw = Cursor::new(include_bytes!("../testdata/identify/content_crw").as_slice());
        let viewer = io::Viewer::new(Box::new(crw), 0);
        let mut view = io::Viewer::create_view(&viewer, 0).expect("Couldn't create view");
        assert!(matches!(type_for_content(&mut view), Ok(Some(Type::Crw))));

        let cr2 = Cursor::new(include_bytes!("../testdata/identify/content_cr2").as_slice());
        let viewer = io::Viewer::new(Box::new(cr2), 0);
        let mut view = io::Viewer::create_view(&viewer, 0).expect("Couldn't create view");
        assert!(matches!(type_for_content(&mut view), Ok(Some(Type::Cr2))));

        let cr3 = Cursor::new(include_bytes!("../testdata/identify/content_cr3").as_slice());
        let viewer = io::Viewer::new(Box::new(cr3), 0);
        let mut view = io::Viewer::create_view(&viewer, 0).expect("Couldn't create view");
        assert!(matches!(type_for_content(&mut view), Ok(Some(Type::Cr3))));

        let mrw = Cursor::new(include_bytes!("../testdata/identify/content_mrw").as_slice());
        let viewer = io::Viewer::new(Box::new(mrw), 0);
        let mut view = io::Viewer::create_view(&viewer, 0).expect("Couldn't create view");
        assert!(matches!(type_for_content(&mut view), Ok(Some(Type::Mrw))));

        let raf = Cursor::new(include_bytes!("../testdata/identify/content_raf").as_slice());
        let viewer = io::Viewer::new(Box::new(raf), 0);
        let mut view = io::Viewer::create_view(&viewer, 0).expect("Couldn't create view");
        assert!(matches!(type_for_content(&mut view), Ok(Some(Type::Raf))));
    }
}
