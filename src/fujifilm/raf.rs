//! RAF specific containers and type

use std::cell::{RefCell, RefMut};
use std::collections::HashMap;
use std::io::{Read, Seek, SeekFrom};

use byteorder::{BigEndian, ReadBytesExt};
use once_cell::unsync::OnceCell;

use crate::container;
use crate::container::GenericContainer;
use crate::io::{View, Viewer};
use crate::utils;
use crate::{Error, Result};

/// Just a list of offset/length
#[derive(Default)]
struct RafOffsetDirectory {
    jpeg_offset: u32,
    jpeg_len: u32,
    meta_offset: u32,
    meta_len: u32,
    cfa_offset: u32,
    cfa_len: u32,
}

pub(super) struct RafContainer {
    view: RefCell<View>,
    model: String,
    offsets: RafOffsetDirectory,
    meta: OnceCell<Option<MetaContainer>>,
}

impl RafContainer {
    pub fn new(view: View) -> Self {
        RafContainer {
            view: RefCell::new(view),
            model: String::from(""),
            offsets: RafOffsetDirectory::default(),
            meta: OnceCell::new(),
        }
    }

    /// Load the RAF Container header
    /// It is BigEndian
    ///
    /// The layout is as follow:
    /// ```text
    ///  0 +--------------------------
    ///    | 16 bytes magic          |
    ///    | RAF_MAGIC               |
    /// 12 +-------------------------+
    ///    | 12 bytes serial (string)|
    /// 28 +-------------------------+
    ///    | 32 bytes string         |
    ///    | zero filled             |
    ///    | Camera model string     |
    /// 60 +-------------------------+
    ///    | 4 bytes u32 version (?) |
    /// 64 +-------------------------+
    ///    | 20 bytes unknown (Skip) |
    /// 84 +-------------------------+
    ///    | offsets directory       |
    ///    | 24 bytes:               |
    ///    | +---------------------+ |
    ///    | | jpeg_offset: u32    | |
    ///    | | jpeg_len: u32       | |
    ///    | +---------------------+ |
    ///    | | meta_offset: u32    | |
    ///    | | meta_len: u32       | |
    ///    | +---------------------+ |
    ///    | | cfa_offset: u32     | |
    ///    | | cfa_len: u32        | |
    ///    | +---------------------+ |
    ///    +-------------------------+
    /// ```
    pub fn load(&mut self) -> Result<()> {
        let mut view = self.view.borrow_mut();
        view.seek(SeekFrom::Start(0))?;

        let mut magic = [0u8; super::RAF_MAGIC.len()];
        view.read_exact(&mut magic)?;
        if magic != super::RAF_MAGIC {
            return Err(Error::FormatError);
        }
        let mut _serial = [0u8; 12];
        view.read_exact(&mut _serial)?;
        let mut model = [0u8; 32];
        view.read_exact(&mut model)?;
        self.model = utils::from_maybe_nul_terminated(&model);

        // looks like it is "0100" in ASCII
        let _version = view.read_u32::<BigEndian>()?;
        view.seek(SeekFrom::Current(20))?;

        self.offsets.jpeg_offset = view.read_u32::<BigEndian>()?;
        self.offsets.jpeg_len = view.read_u32::<BigEndian>()?;
        self.offsets.meta_offset = view.read_u32::<BigEndian>()?;
        self.offsets.meta_len = view.read_u32::<BigEndian>()?;
        self.offsets.cfa_offset = view.read_u32::<BigEndian>()?;
        self.offsets.cfa_len = view.read_u32::<BigEndian>()?;

        Ok(())
    }

    pub fn meta_container(&self) -> Option<&MetaContainer> {
        self.meta
            .get_or_init(|| {
                if self.offsets.meta_offset == 0 || self.offsets.meta_len == 0 {
                    return None;
                }
                let container = Viewer::create_subview(
                    &*self.view.borrow_mut(),
                    self.offsets.meta_offset as u64,
                )
                .map_err(Error::from)
                .map(MetaContainer::new)
                .and_then(|mut container| {
                    container.load()?;
                    Ok(container)
                })
                .ok();

                container
            })
            .as_ref()
    }

    pub fn jpeg_offset(&self) -> u32 {
        self.offsets.jpeg_offset
    }

    pub fn jpeg_len(&self) -> u32 {
        self.offsets.jpeg_len
    }

    pub fn cfa_offset(&self) -> u32 {
        self.offsets.cfa_offset
    }

    pub fn cfa_len(&self) -> u32 {
        self.offsets.cfa_len
    }

    pub fn get_model(&self) -> &str {
        &self.model
    }
}

impl GenericContainer for RafContainer {
    fn endian(&self) -> container::Endian {
        container::Endian::Big
    }

    fn borrow_view_mut(&self) -> RefMut<'_, View> {
        self.view.borrow_mut()
    }
}

/// the RAW dimensions
pub(super) const TAG_SENSOR_DIMENSION: u16 = 0x100;
// const TAG_IMG_TOP_LEFT: u16 = 0x110;
pub(super) const TAG_IMG_HEIGHT_WIDTH: u16 = 0x111;
/// this is the one dcraw use for the active area
//const TAG_OUTPUT_HEIGHT_WIDTH: u16 = 0x121;
/// some info about the RAW.
pub(super) const TAG_RAW_INFO: u16 = 0x130;

#[derive(Debug)]
pub(super) enum Value {
    Int(u32),
    Bytes(Vec<u8>),
}

pub(super) struct MetaContainer {
    view: RefCell<View>,
    tags: HashMap<u16, Value>,
}

impl MetaContainer {
    fn new(view: View) -> MetaContainer {
        MetaContainer {
            view: RefCell::new(view),
            tags: HashMap::new(),
        }
    }

    /// Load the metadata container (a table)
    /// It is BigEndian (MSB)
    /// Format is
    /// ```text
    ///  0 +-------------------------+
    ///    | count: u32              |
    ///  4 +-------------------------+
    ///    |   0 +-----------------+ |
    ///    |     | tag: u16        | |
    ///    |   2 +-----------------+ |
    ///    |     | sz: u16         | |
    ///    |   4 +-----------------+ |
    ///    |     | value: sz bytes | |
    ///    | sz+4+-----------------+ |
    ///    |      .... count*        |
    ///    |                         |
    ///  m +-------------------------+
    /// ```
    fn load(&mut self) -> Result<()> {
        let mut view = self.view.borrow_mut();
        let count = view.read_u32::<BigEndian>()?;
        for _ in 0..count {
            let tag = view.read_u16::<BigEndian>()?;
            let sz = view.read_u16::<BigEndian>()?;
            let value = if sz == 4 {
                let v = view.read_u32::<BigEndian>()?;
                Value::Int(v)
            } else {
                let mut v = Vec::with_capacity(sz as usize);
                // Avoiding initialization of the big buffer.
                // This is deliberate.
                #[allow(clippy::uninit_vec)]
                unsafe {
                    v.set_len(sz as usize);
                }
                view.read_exact(&mut v)?;
                Value::Bytes(v)
            };
            self.tags.insert(tag, value);
        }

        Ok(())
    }

    pub fn value(&self, tag: u16) -> Option<&Value> {
        self.tags.get(&tag)
    }
}

impl GenericContainer for MetaContainer {
    fn endian(&self) -> container::Endian {
        container::Endian::Big
    }

    fn borrow_view_mut(&self) -> RefMut<'_, View> {
        self.view.borrow_mut()
    }
}
