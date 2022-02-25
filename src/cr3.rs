use super::{Error, RawFile, RawFileImpl, Result, Type, TypeId};
use crate::thumbnail::Thumbnail;

/// Canon CR3 File
pub struct Cr3File {}

impl RawFileImpl for Cr3File {
    fn identify_id(&self) -> TypeId {
        0
    }

    fn thumbnail_for_size(&self, size: u32) -> Result<Thumbnail> {
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
