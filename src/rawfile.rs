use std::path::Path;

use super::{Error, Result, Type, TypeId};
use crate::thumbnail::Thumbnail;

/// Very specific implementation trait.
/// It should be the only things that needs to be implemented
/// for a new type of RAW file
pub trait RawFileImpl {
    /// Will identify ID. Ensure it's cached.
    fn identify_id(&self) -> TypeId;

    /// Get the thumbnail for the exact size.
    fn thumbnail_for_size(&self, size: u32) -> Result<Thumbnail>;

    /// List the thumbnail sizes in the file
    fn list_thumbnail_sizes(&self) -> Vec<u32>;
}

/// Standard trait for RAW files.
/// Mostly using the default implementation
pub trait RawFile: RawFileImpl {
    /// Create a RawFile object from a file
    fn new<P>(filename: P, type_hint: Option<Type>) -> Result<Box<dyn RawFile>>
    where
        P: AsRef<Path>,
        Self: Sized,
    {
        Err(Error::UnrecognizedFormat)
    }

    /// Create a RawFile object from a buffer
    fn from_memory<B>(buffer: B, type_hint: Option<Type>) -> Result<Box<dyn RawFile>>
    where
        B: AsRef<[u8]>,
        Self: Sized,
    {
        Err(Error::UnrecognizedFormat)
    }

    /// Return the type for the RAW file
    fn type_(&self) -> Type;

    /// Return the type ID
    fn type_id(&self) -> TypeId {
        self.identify_id()
    }

    /// Return the vendor ID
    fn vendor_id(&self) -> TypeId {
        self.identify_id() >> 16
    }

    /// The rawfile thumbnail sizes
    fn thumbnail_sizes(&self) -> Vec<u32> {
        self.list_thumbnail_sizes()
    }

    /// Return the thumbnail of at least size
    fn thumbnail(&self, tsize: u32) -> Result<Thumbnail> {
        use std::cmp::Ordering;

        let sizes = self.list_thumbnail_sizes();
        if sizes.is_empty() {
            return Err(Error::NotFound);
        }

        let mut smallest_bigger = u32::MAX;
        let mut biggest_smaller = 0_u32;
        let mut found_size = 0_u32;

        for s in sizes {
            match s.cmp(&tsize) {
                Ordering::Less =>  {
                    if s > biggest_smaller {
                        biggest_smaller = s;
                    }
                },
                Ordering::Greater => {
                    if s < smallest_bigger {
                        smallest_bigger = s;
                    }
                },
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

        self.thumbnail_for_size(found_size)
    }
}

#[cfg(test)]
mod test {
    use super::{RawFile, RawFileImpl};
    use crate::thumbnail::Thumbnail;
    use crate::{Error, Result, Type, TypeId};

    struct TestRawFile {}

    impl RawFileImpl for TestRawFile {
        fn identify_id(&self) -> TypeId {
            0
        }

        fn thumbnail_for_size(&self, size: u32) -> Result<Thumbnail> {
            let sizes = self.list_thumbnail_sizes();
            if sizes.contains(&size) {
                Ok(Thumbnail::new(size))
            } else {
                Err(Error::NotFound)
            }
        }

        fn list_thumbnail_sizes(&self) -> Vec<u32> {
            vec![160, 1024, 4096]
        }
    }

    impl RawFile for TestRawFile {
        fn type_(&self) -> Type {
            Type::Test
        }
    }

    #[test]
    fn test_thumbnail() {
        let raw_file = TestRawFile {};
        let t = raw_file.thumbnail(160);
        assert!(t.is_ok());
        let t = t.unwrap();
        assert_eq!(t.size, 160);

        let t = raw_file.thumbnail(1024);
        assert!(t.is_ok());
        let t = t.unwrap();
        assert_eq!(t.size, 1024);

        let t = raw_file.thumbnail(512);
        assert!(t.is_ok());
        let t = t.unwrap();
        assert_eq!(t.size, 1024);

        let t = raw_file.thumbnail(8192);
        assert!(t.is_ok());
        let t = t.unwrap();
        assert_eq!(t.size, 4096);
    }
}
