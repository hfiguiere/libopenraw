mod cr3;
mod rawfile;
mod thumbnail;

pub use rawfile::RawFile;
pub use rawfile::RawFileImpl;

pub type Result<T> = std::result::Result<T, Error>;

#[derive(Debug)]
pub enum Error {
    /// File format is unrecognized
    UnrecognizedFormat,
    /// Not supported
    NotSupported,
    /// Not found in file
    NotFound,
}

pub enum Type {
    Cr3,
    #[cfg(test)]
    Test,
}

/// Type ID
pub type TypeId = u32;
