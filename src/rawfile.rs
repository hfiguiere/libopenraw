// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - rawfile.rs
 *
 * Copyright (C) 2022-2023 Hubert Figuière
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
use num_enum::TryFromPrimitive;

use super::{Error, RawImage, Result, Type, TypeId};
use crate::colour::MatrixOrigin;
use crate::container::RawContainer;
use crate::factory;
use crate::identify;
use crate::io;
use crate::metadata;
use crate::render::RenderingOptions;
use crate::thumbnail::{ThumbDesc, Thumbnail};
use crate::tiff;
use crate::tiff::{exif, Ifd};

/// The trait for any IO
pub trait ReadAndSeek: std::io::Read + std::io::Seek + std::fmt::Debug {}

impl ReadAndSeek for std::io::BufReader<std::fs::File> {}
impl ReadAndSeek for std::io::Cursor<&[u8]> {}
impl ReadAndSeek for std::io::Cursor<Vec<u8>> {}

pub(crate) type RawFileFactory = fn(Rc<io::Viewer>) -> RawFileHandle;
/// Holds a RawFile implementation.
pub type RawFileHandle = RawFileHandleType<dyn RawFile>;
pub type RawFileHandleType<T> = Rc<T>;

#[derive(Debug)]
pub struct ThumbnailStorage {
    pub thumbnails: Vec<(u32, ThumbDesc)>,
    pub sizes: Vec<u32>,
}

impl ThumbnailStorage {
    pub(crate) fn with_thumbnails(thumbnails: Vec<(u32, ThumbDesc)>) -> Self {
        let sizes = thumbnails.iter().map(|v| v.0).collect();

        Self { thumbnails, sizes }
    }
}

/// Very specific implementation trait.
/// It should be the only things that needs to be implemented
/// for a new type of RAW file
pub trait RawFileImpl {
    /// Will identify ID. Ensure it's cached.
    fn identify_id(&self) -> TypeId;

    /// Return the main continer.
    fn container(&self) -> &dyn RawContainer;

    /// Return the thumbnails. Implementation lazy load them
    fn thumbnails(&self) -> &ThumbnailStorage;

    /// Get the thumbnail for the exact size.
    fn thumbnail_for_size(&self, size: u32) -> Result<Thumbnail> {
        let thumbnails = &self.thumbnails().thumbnails;
        if let Some((_, desc)) = thumbnails.iter().find(|t| t.0 == size) {
            self.container().make_thumbnail(desc)
        } else {
            log::warn!("Thumbnail size {} not found", size);
            Err(Error::NotFound)
        }
    }

    /// Get the ifd with type
    fn ifd(&self, ifd_type: tiff::IfdType) -> Option<&tiff::Dir>;

    /// Load the [`RawImage`] and return it.
    ///
    /// If `skip_decompress` is true then the decompression will not be performed.
    fn load_rawdata(&self, skip_decompress: bool) -> Result<RawImage>;

    /// Get the builtin colour matrix for this file.
    fn get_builtin_colour_matrix(&self) -> Result<Vec<f64>>;
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
        .and_then(|e| identify::type_for_extension(e.to_ascii_lowercase().to_str().unwrap()))
}

/// Crate RawFile object from IO.
/// Use `RawFile::from_file() or `RawFile::from_memory`
/// Will return `Error::UnrecognizedFormat` or some `Error::IOError`
/// if the file can't be identified.
fn from_io(readable: Box<dyn ReadAndSeek>, type_hint: Option<Type>) -> Result<RawFileHandle> {
    let viewer = io::Viewer::new(readable, 0);
    let type_hint = if type_hint.is_some() {
        type_hint
    } else {
        let mut view = io::Viewer::create_view(&viewer, 0)?;
        identify::type_for_content(&mut view)?
    };

    if type_hint.is_none() {
        return Err(Error::UnrecognizedFormat);
    }

    let hint = type_hint.unwrap();
    if let Some(f) = factory::get_rawfile_factory(hint) {
        Ok(f(viewer))
    } else {
        Err(Error::UnrecognizedFormat)
    }
}

/// Create a RawFile object from a file
pub fn rawfile_from_file<P>(filename: P, type_hint: Option<Type>) -> Result<RawFileHandle>
where
    P: AsRef<Path>,
{
    let type_hint = match type_hint {
        Some(_) => type_hint,
        None => identify_extension(&filename),
    };
    let file = std::fs::File::open(filename)?;
    let buffered = Box::new(std::io::BufReader::new(file));
    from_io(buffered, type_hint)
}

/// Create a RawFile object from a buffer
pub fn rawfile_from_io(io: Box<dyn ReadAndSeek>, type_hint: Option<Type>) -> Result<RawFileHandle> {
    from_io(io, type_hint)
}

/// Standard trait for RAW files.
/// Mostly using the default implementation
pub trait RawFile: RawFileImpl + crate::dump::DumpFile + std::fmt::Debug {
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
    fn thumbnail_sizes(&self) -> &[u32] {
        &self.thumbnails().sizes
    }

    /// Return the thumbnail of at least size
    fn thumbnail(&self, tsize: u32) -> Result<Thumbnail> {
        use std::cmp::Ordering;

        debug!("Requested thumbnail of size {}", tsize);

        if tsize == 0 {
            error!("0 is an invalid size");
            return Err(Error::InvalidParam);
        }

        let sizes = &self.thumbnails().sizes;
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
                    if *s > biggest_smaller {
                        biggest_smaller = *s;
                    }
                }
                Ordering::Greater => {
                    if *s < smallest_bigger {
                        smallest_bigger = *s;
                    }
                }
                Ordering::Equal => {
                    found_size = *s;
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
    fn raw_data(&self, skip_decompression: bool) -> Result<RawImage> {
        self.load_rawdata(skip_decompression).map(|mut rawdata| {
            for i in 1..2_usize {
                if let Ok((_, matrix)) = self.colour_matrix(i) {
                    rawdata.set_colour_matrix(i, &matrix);
                }
            }

            rawdata
        })
    }

    /// Render the image.
    fn rendered_image(&self, options: RenderingOptions) -> Result<RawImage> {
        let raw_data = self.raw_data(false)?;
        raw_data.rendered_image(options)
    }

    /// Get the main IFD
    fn main_ifd(&self) -> Option<&tiff::Dir> {
        self.ifd(tiff::IfdType::Main)
    }

    /// Get the Exif IFD
    fn exif_ifd(&self) -> Option<&tiff::Dir> {
        self.ifd(tiff::IfdType::Exif)
    }

    /// Get the MakerNote
    fn maker_note_ifd(&self) -> Option<&tiff::Dir> {
        self.ifd(tiff::IfdType::MakerNote)
    }

    /// Get a metadata iterator. This will iterate over
    /// all the metadata for the raw file.
    fn metadata(&self) -> metadata::Iterator {
        self.container().dir_iterator()
    }

    /// Get the metadata value
    fn metadata_value(&self, key: &metadata::Key) -> Option<metadata::Value> {
        self.main_ifd()
            .map(|ifd| ifd.iter().into())
            .and_then(|mut iter: metadata::Iterator| iter.find(|item| &item.0 == key))
            .map(|item| item.1)
    }

    /// File orientation
    fn orientation(&self) -> u32 {
        self.metadata_value(&"Exif.Image.Orientation".to_string())
            .and_then(|value| value.integer())
            .unwrap_or(0)
    }

    /// Return the indexed callibration illumant: 1 or 2.
    fn calibration_illuminant(&self, index: u32) -> exif::LightsourceValue {
        let tag = match index {
            1 => exif::DNG_TAG_CALIBRATION_ILLUMINANT1,
            2 => exif::DNG_TAG_CALIBRATION_ILLUMINANT2,
            _ => return exif::LightsourceValue::Unknown,
        };
        self.main_ifd()
            .and_then(|dir| dir.uint_value(tag))
            .and_then(|value| exif::LightsourceValue::try_from_primitive(value).ok())
            .or_else(|| {
                if index == 1 {
                    self.get_builtin_colour_matrix()
                        .map(|_| exif::LightsourceValue::D65)
                        .ok()
                } else {
                    None
                }
            })
            .unwrap_or(exif::LightsourceValue::Unknown)
    }

    /// Return the colour matrix for the file.
    fn colour_matrix(&self, index: usize) -> Result<(MatrixOrigin, Vec<f64>)> {
        let tag = match index {
            1 => exif::DNG_TAG_COLORMATRIX1,
            2 => exif::DNG_TAG_COLORMATRIX2,
            _ => return Err(Error::InvalidParam),
        };

        self.main_ifd()
            .and_then(|dir| {
                dir.entry(tag)
                    .and_then(|e| e.value_array::<exif::SRational>(dir.endian()))
                    .map(|a| (MatrixOrigin::Provided, a.iter().map(|r| r.into()).collect()))
            })
            .ok_or_else(|| {
                log::debug!("DNG color matrix not found");
                Error::NotFound
            })
            .or_else(|_| {
                if index == 1 {
                    self.get_builtin_colour_matrix()
                        .map(|matrix| (MatrixOrigin::Builtin, matrix))
                } else {
                    Err(Error::NotFound)
                }
            })
    }
}

#[cfg(test)]
mod test {
    use std::cell::{RefCell, RefMut};

    use once_cell::unsync::OnceCell;

    use super::{RawFile, RawFileImpl, RawImage, ThumbnailStorage};
    use crate::bitmap::Bitmap;
    use crate::container::RawContainer;
    use crate::io::View;
    use crate::thumbnail::{Data, ThumbDesc, Thumbnail};
    use crate::tiff;
    use crate::{DataType, Dump, Error, Result, Type, TypeId};

    #[derive(Debug)]
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

    impl RawContainer for TestContainer {
        fn borrow_view_mut(&self) -> RefMut<'_, View> {
            self.view.borrow_mut()
        }

        fn raw_type(&self) -> Type {
            Type::Test
        }
    }

    impl Dump for TestContainer {
        #[cfg(feature = "dump")]
        fn write_dump<W: std::io::Write + ?Sized>(&self, _out: &mut W, _indent: u32) {}
    }

    #[derive(Debug)]
    struct TestRawFile {
        container: TestContainer,
        thumbnails: OnceCell<ThumbnailStorage>,
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

        fn container(&self) -> &dyn RawContainer {
            &self.container
        }

        fn thumbnails(&self) -> &ThumbnailStorage {
            self.thumbnails.get_or_init(|| {
                ThumbnailStorage::with_thumbnails(vec![
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
                ])
            })
        }

        fn thumbnail_for_size(&self, size: u32) -> Result<Thumbnail> {
            let sizes = &self.thumbnails().sizes;
            if sizes.contains(&size) {
                Ok(Thumbnail::with_data(size, size, DataType::Jpeg, vec![]))
            } else {
                Err(Error::NotFound)
            }
        }

        fn ifd(&self, _ifd_type: tiff::IfdType) -> Option<&tiff::Dir> {
            None
        }

        fn load_rawdata(&self, _skip_decompress: bool) -> Result<RawImage> {
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
        fn write_dump<W: std::io::Write + ?Sized>(&self, _out: &mut W, _indent: u32) {}
    }

    dumpfile_impl!(TestRawFile);

    #[test]
    fn test_thumbnail() {
        let rawfile = TestRawFile::new();
        let t = rawfile.thumbnail(160);
        assert!(t.is_ok());
        let t = t.unwrap();
        assert_eq!(t.width(), 160);

        let t = rawfile.thumbnail(1024);
        assert!(t.is_ok());
        let t = t.unwrap();
        assert_eq!(t.width(), 1024);

        let t = rawfile.thumbnail(512);
        assert!(t.is_ok());
        let t = t.unwrap();
        assert_eq!(t.width(), 1024);

        let t = rawfile.thumbnail(8192);
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
