use super::rawfile::ReadAndSeek;
use super::{Error, RawFile, RawFileImpl, Result, Type, TypeId};
use crate::thumbnail::Thumbnail;

/// Canon CR3 File
pub struct Cr3File {
    reader: Box<dyn ReadAndSeek>,
}

impl Cr3File {
    pub fn factory(reader: Box<dyn ReadAndSeek>) -> Box<dyn RawFile> {
        Box::new(Cr3File { reader })
    }
}

impl RawFileImpl for Cr3File {
    fn identify_id(&self) -> TypeId {
        0
    }

    fn thumbnail_for_size(&self, _size: u32) -> Result<Thumbnail> {
        Err(Error::NotSupported)
    }

    fn list_thumbnail_sizes(&self) -> Vec<u32> {
        vec![]
    }
}

impl RawFile for Cr3File {
    fn type_(&self) -> Type {
        super::Type::Cr3
    }
}
