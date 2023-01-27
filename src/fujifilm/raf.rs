// SPDX-License-Identifier: LGPL-3.0-or-later
// SPDX-Copyright: (C) 2022-2023 Hubert Figuière

//! RAF specific containers and type

use std::cell::{RefCell, RefMut};
use std::collections::{BTreeMap, HashMap};
use std::io::{Read, Seek, SeekFrom};

use byteorder::{BigEndian, ReadBytesExt};
use once_cell::unsync::OnceCell;

use crate::bitmap::{Point, Size};
use crate::container;
use crate::container::RawContainer;
use crate::io::{View, Viewer};
use crate::jpeg;
use crate::utils;
use crate::Type as RawType;
use crate::{Dump, Error, Result};

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
    jpeg_preview: OnceCell<Option<jpeg::Container>>,
}

impl RafContainer {
    pub fn new(view: View) -> Self {
        RafContainer {
            view: RefCell::new(view),
            model: String::from(""),
            offsets: RafOffsetDirectory::default(),
            meta: OnceCell::new(),
            jpeg_preview: OnceCell::new(),
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
                    &self.view.borrow_mut(),
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

    pub fn jpeg_preview(&self) -> Option<&jpeg::Container> {
        self.jpeg_preview
            .get_or_init(|| {
                Viewer::create_subview(&self.view.borrow_mut(), self.offsets.jpeg_offset as u64)
                    .map(|view| jpeg::Container::new(view, RawType::Raf))
                    .ok()
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

impl RawContainer for RafContainer {
    fn endian(&self) -> container::Endian {
        container::Endian::Big
    }

    fn borrow_view_mut(&self) -> RefMut<'_, View> {
        self.view.borrow_mut()
    }

    fn raw_type(&self) -> RawType {
        RawType::Raf
    }
}

impl Dump for RafContainer {
    #[cfg(feature = "dump")]
    fn print_dump(&self, indent: u32) {
        dump_println!(indent, "<RAF Container @{}>", self.view.borrow().offset());
        {
            let indent = indent + 1;
            dump_println!(indent, "Model  = {}", self.model);
            dump_println!(indent, "<Offsets>");
            {
                let indent = indent + 1;
                dump_println!(indent, "JPEG Offset = {}", self.offsets.jpeg_offset);
                dump_println!(indent, "JPEG Len    = {}", self.offsets.jpeg_len);
                dump_println!(indent, "Meta Offset = {}", self.offsets.meta_offset);
                dump_println!(indent, "Meta Len    = {}", self.offsets.meta_len);
                dump_println!(indent, "CFA Offset  = {}", self.offsets.cfa_offset);
                dump_println!(indent, "CFA Len     = {}", self.offsets.cfa_len);
            }
            dump_println!(indent, "</Offsets>");
            if let Some(jpeg_preview) = self.jpeg_preview() {
                jpeg_preview.print_dump(indent);
            } else {
                dump_println!(indent, "ERROR: JPEG Preview not found");
            }
            if let Some(meta_container) = self.meta_container() {
                meta_container.print_dump(indent);
            } else {
                dump_println!(indent, "ERROR: Meta container not found");
            }
            dump_println!(indent, "CFA Container TODO");
        }
        dump_println!(indent, "</RAF Container>");
    }
}

/// the RAW dimensions
pub(super) const TAG_SENSOR_DIMENSION: u16 = 0x100;
/// Top Left of activate area
pub(super) const TAG_IMG_TOP_LEFT: u16 = 0x110;
/// Width Height of activate area
pub(super) const TAG_IMG_HEIGHT_WIDTH: u16 = 0x111;
const TAG_OUTPUT_HEIGHT_WIDTH: u16 = 0x121;
/// some info about the RAW.
pub(super) const TAG_RAW_INFO: u16 = 0x130;

lazy_static::lazy_static! {
    static ref META_TAG_NAMES: HashMap<u16, &'static str> = HashMap::from([
        (TAG_SENSOR_DIMENSION, "SensorDimension"),
        (TAG_IMG_TOP_LEFT, "ImageTopLeft"),
        (TAG_IMG_HEIGHT_WIDTH, "ImageHeightWidth"),
        (TAG_OUTPUT_HEIGHT_WIDTH, "OutputHeightWidth"),
        (TAG_RAW_INFO, "RawInfo"),
    ]);
}

#[derive(Debug)]
pub(super) enum Value {
    Int(u32),
    Bytes(Vec<u8>),
}

impl std::fmt::Display for Value {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::Int(n) => write!(f, "{n:x}"),
            Self::Bytes(b) => write!(f, "bytes len={}", b.len()),
        }
    }
}
impl std::convert::TryFrom<&Value> for Point {
    type Error = crate::Error;
    fn try_from(v: &Value) -> Result<Self> {
        match v {
            Value::Int(n) => {
                let y = (n & 0xffff0000) >> 16;
                let x = n & 0x0000ffff;
                Ok(Point { x, y })
            }
            _ => Err(Error::InvalidFormat),
        }
    }
}

impl std::convert::TryFrom<&Value> for Size {
    type Error = crate::Error;
    fn try_from(v: &Value) -> Result<Self> {
        match v {
            Value::Int(n) => {
                let height = (n & 0xffff0000) >> 16;
                let width = n & 0x0000ffff;
                Ok(Size { width, height })
            }
            _ => Err(Error::InvalidFormat),
        }
    }
}

pub(super) struct MetaContainer {
    view: RefCell<View>,
    tags: BTreeMap<u16, Value>,
}

impl MetaContainer {
    fn new(view: View) -> MetaContainer {
        MetaContainer {
            view: RefCell::new(view),
            tags: BTreeMap::new(),
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

impl RawContainer for MetaContainer {
    fn endian(&self) -> container::Endian {
        container::Endian::Big
    }

    fn borrow_view_mut(&self) -> RefMut<'_, View> {
        self.view.borrow_mut()
    }

    fn raw_type(&self) -> RawType {
        RawType::Raf
    }
}

impl Dump for MetaContainer {
    #[cfg(feature = "dump")]
    fn print_dump(&self, indent: u32) {
        dump_println!(
            indent,
            "<RAF Meta Container @{}>",
            self.view.borrow().offset()
        );
        {
            let indent = indent + 1;
            for (tag, value) in &self.tags {
                let tag_name = META_TAG_NAMES.get(tag).unwrap_or(&"");
                dump_println!(indent, "<0x{:x}={}> {} = {}", tag, tag, tag_name, value);
            }
        }
        dump_println!(indent, "</RAF Meta Container>");
    }
}

#[cfg(test)]
mod test {

    use std::convert::TryFrom;

    use super::Value;
    use crate::bitmap::{Point, Size};

    #[test]
    fn test_value_convert() {
        let test_value = 0x0100_0001;

        let value = Value::Int(test_value);
        let value_bytes = Value::Bytes(vec![]);

        let pt = Point::try_from(&value);
        assert!(pt.is_ok());
        assert_eq!(pt.unwrap(), Point { x: 1, y: 256 });

        let pt = Point::try_from(&value_bytes);
        assert!(pt.is_err());

        let sz = Size::try_from(&value);
        assert!(sz.is_ok());
        assert_eq!(
            sz.unwrap(),
            Size {
                width: 1,
                height: 256
            }
        );

        let sz = Size::try_from(&value_bytes);
        assert!(sz.is_err());
    }
}
