/*
 * libopenraw - rawfile.rs
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

//! Camera RAW file

use std::path::Path;
use std::rc::Rc;

use log::{debug, error};

use super::{Dump, Error, RawData, Result, Type, TypeId};
use crate::container::GenericContainer;
use crate::factory;
use crate::identify;
use crate::thumbnail::{ThumbDesc, Thumbnail};
use crate::tiff;
use crate::tiff::{exif, Ifd};

/// The trait for any IO
pub trait ReadAndSeek: std::io::Read + std::io::Seek {}

impl ReadAndSeek for std::fs::File {}
impl ReadAndSeek for std::io::Cursor<&[u8]> {}
impl ReadAndSeek for std::io::Cursor<Vec<u8>> {}

pub type RawFileFactory = fn(Box<dyn ReadAndSeek>) -> Box<dyn RawFile>;

/// Very specific implementation trait.
/// It should be the only things that needs to be implemented
/// for a new type of RAW file
pub trait RawFileImpl {
    /// Will identify ID. Ensure it's cached.
    fn identify_id(&self) -> TypeId;

    /// Return the main continer.
    fn container(&self) -> &dyn GenericContainer;

    /// Return the thumbnails. Implementation lazy load them
    fn thumbnails(&self) -> &Vec<(u32, ThumbDesc)>;

    /// Get the thumbnail for the exact size.
    fn thumbnail_for_size(&self, size: u32) -> Result<Thumbnail> {
        let thumbnails = self.thumbnails();
        if let Some((_, desc)) = thumbnails.iter().find(|t| t.0 == size) {
            self.container().make_thumbnail(desc)
        } else {
            log::warn!("Thumbnail size {} not found", size);
            Err(Error::NotFound)
        }
    }

    /// List the thumbnail sizes in the file
    fn list_thumbnail_sizes(&self) -> Vec<u32> {
        let thumbnails = self.thumbnails();

        // XXX shall we cache this?
        thumbnails.iter().map(|v| v.0).collect()
    }

    /// Get the ifd with type
    fn ifd(&self, ifd_type: tiff::Type) -> Option<Rc<tiff::Dir>>;

    /// Load the RawData and return it.
    fn load_rawdata(&self) -> Result<RawData>;

    /// Get the builtin colour matrix for this file.
    fn get_builtin_colour_matrix(&self) -> Result<Vec<f64>>;

    /// Return the value for white
    fn white(&self) -> u16 {
        0xffff
    }

    /// Return the value for black
    fn black(&self) -> u16 {
        0
    }
}

/// Identify the RAW file type from file extension
/// `filename` is the file path.
pub fn identify_extension<P>(filename: &P) -> Option<Type>
where
    P: AsRef<Path>,
{
    let file_path = filename.as_ref();
    file_path
        .extension()
        .and_then(|e| identify::type_for_extension(&e.to_ascii_lowercase()))
}

/// Crate RawFile object from IO.
/// Use `RawFile::from_file() or `RawFile::from_memory`
/// Will return `Error::UnrecognizedFormat` or some `Error::IOError`
/// if the file can't be identified.
fn from_io(
    mut readable: Box<dyn ReadAndSeek>,
    type_hint: Option<Type>,
) -> Result<Box<dyn RawFile>> {
    let type_hint = if type_hint.is_some() {
        type_hint
    } else {
        identify::type_for_content(&mut *readable)?
    };
    readable.rewind()?;

    if type_hint == None {
        return Err(Error::UnrecognizedFormat);
    }

    let hint = type_hint.unwrap();
    if let Some(f) = factory::get_raw_file_factory(hint) {
        Ok(f(readable))
    } else {
        Err(Error::UnrecognizedFormat)
    }
}

/// Create a RawFile object from a file
pub fn raw_file_from_file<P>(filename: P, type_hint: Option<Type>) -> Result<Box<dyn RawFile>>
where
    P: AsRef<Path>,
{
    let type_hint = match type_hint {
        Some(_) => type_hint,
        None => identify_extension(&filename),
    };
    let file = Box::new(std::fs::File::open(filename)?);
    from_io(file, type_hint)
}

/// Create a RawFile object from a buffer
// XXX figure out the lifetime issue
//    fn from_memory<B>(buffer: B, type_hint: Option<Type>) -> Result<Box<dyn RawFile>>
//    where
//        B: AsRef<[u8]>,
//        Self: Sized,
//    {
//        from_io(Box::new(std::io::Cursor::new(buffer.as_ref())), type_hint)
//    }

/// Standard trait for RAW files.
/// Mostly using the default implementation
pub trait RawFile: RawFileImpl + Dump {
    /// Return the type for the RAW file
    fn type_(&self) -> Type;

    /// Return the type ID
    fn type_id(&self) -> TypeId {
        self.identify_id()
    }

    /// Return the vendor ID
    fn vendor_id(&self) -> u16 {
        self.identify_id().0
    }

    /// The rawfile thumbnail sizes
    fn thumbnail_sizes(&self) -> Vec<u32> {
        self.list_thumbnail_sizes()
    }

    /// Return the thumbnail of at least size
    fn thumbnail(&self, tsize: u32) -> Result<Thumbnail> {
        use std::cmp::Ordering;

        debug!("Requested thumbnail of size {}", tsize);

        let sizes = self.list_thumbnail_sizes();
        if sizes.is_empty() {
            error!("No thumbnail available");
            return Err(Error::NotFound);
        }

        let mut smallest_bigger = u32::MAX;
        let mut biggest_smaller = 0_u32;
        let mut found_size = 0_u32;

        for s in sizes {
            match s.cmp(&tsize) {
                Ordering::Less => {
                    if s > biggest_smaller {
                        biggest_smaller = s;
                    }
                }
                Ordering::Greater => {
                    if s < smallest_bigger {
                        smallest_bigger = s;
                    }
                }
                Ordering::Equal => {
                    found_size = s;
                    break;
                }
            }
        }

        if found_size == 0 {
            found_size = if smallest_bigger < u32::MAX {
                smallest_bigger
            } else {
                biggest_smaller
            };
        }

        if found_size == 0 {
            return Err(Error::NotFound);
        }

        debug!("Found thumbnail of size {}", found_size);
        self.thumbnail_for_size(found_size)
    }

    /// Get the RAW data
    fn raw_data(&self) -> Result<RawData> {
        self.load_rawdata()
    }

    /// Get the main IFD
    fn main_ifd(&self) -> Option<Rc<tiff::Dir>> {
        self.ifd(tiff::Type::Main)
    }

    /// Get the Exif IFD
    fn exif_ifd(&self) -> Option<Rc<tiff::Dir>> {
        self.ifd(tiff::Type::Exif)
    }

    /// Get the MakerNote
    fn maker_note_ifd(&self) -> Option<Rc<tiff::Dir>> {
        self.ifd(tiff::Type::MakerNote)
    }

    /// Return the colour matrix for the file.
    fn colour_matrix(&self, index: u32) -> Result<Vec<f64>> {
        let tag = match index {
            1 => exif::DNG_TAG_COLORMATRIX1,
            2 => exif::DNG_TAG_COLORMATRIX2,
            _ => return Err(Error::InvalidParam),
        };

        self.main_ifd()
            .and_then(|dir| {
                dir.entry(tag)
                    .and_then(|e| e.value_array::<exif::SRational>(dir.endian()))
                    .map(|a| a.iter().map(|r| r.into()).collect())
            })
            .ok_or_else(|| {
                log::debug!("DNG color matrix not found");
                Error::NotFound
            })
            .or_else(|_| {
                if index != 1 {
                    self.get_builtin_colour_matrix()
                } else {
                    Err(Error::NotFound)
                }
            })
    }
}

#[cfg(test)]
mod test {
    use std::cell::{RefCell, RefMut};
    use std::rc::Rc;

    use once_cell::unsync::OnceCell;

    use super::{RawData, RawFile, RawFileImpl};
    use crate::bitmap::Bitmap;
    use crate::container::GenericContainer;
    use crate::io::View;
    use crate::thumbnail::{Data, ThumbDesc, Thumbnail};
    use crate::tiff;
    use crate::{DataType, Dump, Error, Result, Type, TypeId};

    struct TestContainer {
        view: RefCell<View>,
    }

    impl TestContainer {
        pub fn new() -> TestContainer {
            TestContainer {
                view: RefCell::new(View::new_test()),
            }
        }
    }

    impl GenericContainer for TestContainer {
        fn borrow_view_mut(&self) -> RefMut<'_, View> {
            self.view.borrow_mut()
        }

        fn raw_type(&self) -> Type {
            Type::Test
        }
    }

    impl Dump for TestContainer {
        #[cfg(feature = "dump")]
        fn print_dump(&self, _indent: u32) {}
    }

    struct TestRawFile {
        container: TestContainer,
        thumbnails: OnceCell<Vec<(u32, ThumbDesc)>>,
    }

    impl TestRawFile {
        fn new() -> TestRawFile {
            TestRawFile {
                container: TestContainer::new(),
                thumbnails: OnceCell::new(),
            }
        }
    }
    impl RawFileImpl for TestRawFile {
        fn identify_id(&self) -> TypeId {
            TypeId::default()
        }

        fn container(&self) -> &dyn GenericContainer {
            &self.container
        }

        fn thumbnails(&self) -> &Vec<(u32, ThumbDesc)> {
            self.thumbnails.get_or_init(|| {
                vec![
                    (
                        160,
                        ThumbDesc {
                            width: 160,
                            height: 160,
                            data_type: DataType::Jpeg,
                            data: Data::Bytes(vec![]),
                        },
                    ),
                    (
                        1024,
                        ThumbDesc {
                            width: 1024,
                            height: 1024,
                            data_type: DataType::Jpeg,
                            data: Data::Bytes(vec![]),
                        },
                    ),
                    (
                        4096,
                        ThumbDesc {
                            width: 4096,
                            height: 4096,
                            data_type: DataType::Jpeg,
                            data: Data::Bytes(vec![]),
                        },
                    ),
                ]
            })
        }

        fn thumbnail_for_size(&self, size: u32) -> Result<Thumbnail> {
            let sizes = self.list_thumbnail_sizes();
            if sizes.contains(&size) {
                Ok(Thumbnail::new(size, size, DataType::Jpeg, vec![]))
            } else {
                Err(Error::NotFound)
            }
        }

        fn ifd(&self, _ifd_type: tiff::Type) -> Option<Rc<tiff::Dir>> {
            None
        }

        fn load_rawdata(&self) -> Result<RawData> {
            Err(Error::NotFound)
        }

        fn get_builtin_colour_matrix(&self) -> Result<Vec<f64>> {
            Err(Error::NotSupported)
        }
    }

    impl RawFile for TestRawFile {
        fn type_(&self) -> Type {
            Type::Test
        }
    }

    impl Dump for TestRawFile {
        #[cfg(feature = "dump")]
        fn print_dump(&self, _indent: u32) {}
    }

    #[test]
    fn test_thumbnail() {
        let raw_file = TestRawFile::new();
        let t = raw_file.thumbnail(160);
        assert!(t.is_ok());
        let t = t.unwrap();
        assert_eq!(t.width(), 160);

        let t = raw_file.thumbnail(1024);
        assert!(t.is_ok());
        let t = t.unwrap();
        assert_eq!(t.width(), 1024);

        let t = raw_file.thumbnail(512);
        assert!(t.is_ok());
        let t = t.unwrap();
        assert_eq!(t.width(), 1024);

        let t = raw_file.thumbnail(8192);
        assert!(t.is_ok());
        let t = t.unwrap();
        assert_eq!(t.width(), 4096);
    }

    #[test]
    fn test_identify_extension() {
        use std::path::PathBuf;

        use super::identify_extension;

        assert_eq!(
            identify_extension(&PathBuf::from("FILE.CR3")),
            Some(Type::Cr3)
        );
        assert_eq!(
            identify_extension(&PathBuf::from("FiLe.cr3")),
            Some(Type::Cr3)
        );
        assert_eq!(identify_extension(&PathBuf::from("NOPE")), None);
    }
}
