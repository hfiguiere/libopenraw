// SPDX-License-Identifier: LGPL-3.0-or-later
// SPDX-Copyright: (C) 2022-2023 Hubert Figuière

//! RAF specific containers and type

use std::cell::{RefCell, RefMut};
use std::collections::{BTreeMap, HashMap};
use std::convert::TryFrom;
use std::io::{Read, Seek, SeekFrom};

use byteorder::{BigEndian, ReadBytesExt};
use once_cell::unsync::OnceCell;

use crate::container;
use crate::container::RawContainer;
use crate::io::{View, Viewer};
use crate::jpeg;
use crate::metadata;
use crate::tiff;
use crate::utils;
use crate::Type as RawType;
use crate::{Dump, Error, Point, Result, Size};

#[derive(Debug, Default)]
/// Just a list of offset/length
struct RafOffsetDirectory {
    jpeg_offset: u32,
    jpeg_len: u32,
    meta_offset: u32,
    meta_len: u32,
    cfa_offset: u32,
    cfa_len: u32,
    extra_meta_offset: u32,
    extra_meta_len: u32,
    extra_cfa_offset: u32,
    extra_cfa_len: u32,
}

#[derive(Debug)]
pub(super) struct RafContainer {
    view: RefCell<View>,
    version: [u8; 4],
    serial: [u8; 8],
    model: String,
    fw_version: [u8; 4],
    offsets: RafOffsetDirectory,
    meta: OnceCell<Option<MetaContainer>>,
    extra_meta: OnceCell<Option<MetaContainer>>,
    jpeg_preview: OnceCell<Option<jpeg::Container>>,
    cfa: OnceCell<Option<tiff::Container>>,
    extra_cfa: OnceCell<Option<tiff::Container>>,
}

impl RafContainer {
    pub fn new(view: View) -> Self {
        RafContainer {
            view: RefCell::new(view),
            version: [0u8; 4],
            serial: [0u8; 8],
            model: String::default(),
            fw_version: [0u8; 4],
            offsets: RafOffsetDirectory::default(),
            meta: OnceCell::new(),
            extra_meta: OnceCell::new(),
            jpeg_preview: OnceCell::new(),
            cfa: OnceCell::new(),
            extra_cfa: OnceCell::new(),
        }
    }

    /// Load the RAF Container header
    /// It is BigEndian
    ///
    /// The layout is as follow:
    /// ```text
    ///  0 +----------------------------+
    ///    | 16 bytes magic             |
    ///    | RAF_MAGIC                  |
    /// 16 +----------------------------+
    ///    | 4 bytes ASCII (version)    |
    /// 20 +----------------------------+
    ///    | 8 bytes ASCII (serial)     |
    /// 28 +----------------------------+
    ///    | 32 bytes ASCII             |
    ///    | zero filled                |
    ///    | Camera model string        |
    /// 60 +----------------------------+
    ///    | 4 bytes ASCII (FW version) |
    /// 64 +----------------------------+
    ///    | 20 bytes unknown (Skip)    |
    /// 84 +----------------------------+
    ///    | offsets directory          |
    ///    | 24 bytes:                  |
    ///    | +---------------------+    |
    ///    | | jpeg_offset: u32    |    |
    ///    | | jpeg_len: u32       |    |
    ///    | +---------------------+    |
    ///    | | meta_offset: u32    |    |
    ///    | | meta_len: u32       |    |
    ///    | +---------------------+    |
    ///    | | cfa_offset: u32     |    |
    ///    | | cfa_len: u32        |    |
    ///    | +---------------------+    |
    ///    | 12 bytes unknown (Skip)    |
    ///120 +----------------------------+
    ///    | if jpeg_offset > 120       |
    ///    | +------------------------+ |
    ///    | | extra_meta_offset: u32 | |
    ///    | | extra_meta_len: u32    | |
    ///    | +------------------------+ |
    ///    | | extra_cfa_offset: u32  | |
    ///    | | extra_cfa_len: u32     | |
    ///    | +------------------------+ |
    ///    +----------------------------+
    /// ```
    pub fn load(&mut self) -> Result<()> {
        let mut view = self.view.borrow_mut();
        view.seek(SeekFrom::Start(0))?;

        let mut magic = [0u8; super::RAF_MAGIC.len()];
        view.read_exact(&mut magic)?;
        if magic != super::RAF_MAGIC {
            return Err(Error::FormatError);
        }
        view.read_exact(&mut self.version)?;
        view.read_exact(&mut self.serial)?; // 8 bytes
        let mut model = [0u8; 32];
        view.read_exact(&mut model)?;
        self.model = utils::from_maybe_nul_terminated(&model);

        // looks like it is "0100" in ASCII
        view.read_exact(&mut self.fw_version)?; // 4 bytes
        view.seek(SeekFrom::Current(20))?;

        // offset 84
        self.offsets.jpeg_offset = view.read_u32::<BigEndian>()?;
        self.offsets.jpeg_len = view.read_u32::<BigEndian>()?;
        // offset 92
        self.offsets.meta_offset = view.read_u32::<BigEndian>()?;
        self.offsets.meta_len = view.read_u32::<BigEndian>()?;
        // offset 100
        self.offsets.cfa_offset = view.read_u32::<BigEndian>()?;
        self.offsets.cfa_len = view.read_u32::<BigEndian>()?;
        // offset 108
        view.seek(SeekFrom::Current(12))?;
        // offset 120
        if self.offsets.jpeg_offset > 120 {
            self.offsets.extra_meta_offset = view.read_u32::<BigEndian>()?;
            self.offsets.extra_meta_len = view.read_u32::<BigEndian>()?;
            self.offsets.extra_cfa_offset = view.read_u32::<BigEndian>()?;
            self.offsets.extra_cfa_len = view.read_u32::<BigEndian>()?;
        }
        Ok(())
    }

    pub fn cfa_container2(&self) -> Option<&tiff::Container> {
        self.extra_cfa
            .get_or_init(|| {
                if self.offsets.extra_cfa_offset == 0 || self.offsets.extra_cfa_len == 0 {
                    return None;
                }

                log::error!("Extra CFA not implemented");
                None
            })
            .as_ref()
    }

    pub fn cfa_container(&self) -> Option<&tiff::Container> {
        self.cfa
            .get_or_init(|| {
                if self.offsets.cfa_offset == 0 || self.offsets.cfa_len == 0 {
                    return None;
                }
                let meta = self.meta_container()?;
                if meta.value(TAG_WB_OLD).is_some() {
                    log::debug!("Found old WB, no container");
                    return None;
                }
                let container =
                    Viewer::create_subview(&self.view.borrow_mut(), self.offsets.cfa_offset as u64)
                        .map_err(Error::from)
                        .map(|view| tiff::Container::new(view, vec![], RawType::Raf))
                        .and_then(|mut container| {
                            container.load(None)?;
                            Ok(container)
                        })
                        .ok();

                container
            })
            .as_ref()
    }

    fn meta_container_at(&self, offset: u32, len: u32) -> Option<MetaContainer> {
        if offset == 0 || len == 0 {
            return None;
        }
        let container = Viewer::create_subview(&self.view.borrow_mut(), offset as u64)
            .map_err(Error::from)
            .map(MetaContainer::new)
            .and_then(|mut container| {
                container.load()?;
                Ok(container)
            })
            .ok();

        container
    }

    pub fn meta_container(&self) -> Option<&MetaContainer> {
        self.meta
            .get_or_init(|| self.meta_container_at(self.offsets.meta_offset, self.offsets.meta_len))
            .as_ref()
    }

    pub fn extra_meta_container(&self) -> Option<&MetaContainer> {
        self.extra_meta
            .get_or_init(|| {
                self.meta_container_at(self.offsets.extra_meta_offset, self.offsets.extra_meta_len)
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

    fn dir_iterator(&self) -> metadata::Iterator {
        self.jpeg_preview()
            .map(|preview| preview.dir_iterator())
            .unwrap_or_default()
    }
}

impl Dump for RafContainer {
    #[cfg(feature = "dump")]
    fn write_dump<W: std::io::Write + ?Sized>(&self, out: &mut W, indent: u32) {
        dump_writeln!(
            out,
            indent,
            "<RAF Container @{}>",
            self.view.borrow().offset()
        );
        {
            let indent = indent + 1;
            dump_writeln!(
                out,
                indent,
                "Version = {}",
                String::from_utf8_lossy(&self.version)
            );
            dump_writeln!(
                out,
                indent,
                "Serial  = {}",
                String::from_utf8_lossy(&self.serial)
            );
            dump_writeln!(out, indent, "Model   = {}", self.model);
            dump_writeln!(
                out,
                indent,
                "FW Ver. = {}",
                String::from_utf8_lossy(&self.fw_version)
            );
            dump_writeln!(out, indent, "<Offsets>");
            {
                let indent = indent + 1;
                dump_writeln!(out, indent, "JPEG Offset = {}", self.offsets.jpeg_offset);
                dump_writeln!(out, indent, "JPEG Len    = {}", self.offsets.jpeg_len);
                dump_writeln!(out, indent, "Meta Offset = {}", self.offsets.meta_offset);
                dump_writeln!(out, indent, "Meta Len    = {}", self.offsets.meta_len);
                dump_writeln!(out, indent, "CFA Offset  = {}", self.offsets.cfa_offset);
                dump_writeln!(out, indent, "CFA Len     = {}", self.offsets.cfa_len);
                dump_writeln!(
                    out,
                    indent,
                    "Extra Offset= {}",
                    self.offsets.extra_meta_offset
                );
                dump_writeln!(out, indent, "Extra Len   = {}", self.offsets.extra_meta_len);
                dump_writeln!(
                    out,
                    indent,
                    "Extra CFA Offset= {}",
                    self.offsets.extra_cfa_offset
                );
                dump_writeln!(
                    out,
                    indent,
                    "Extra CFA Len   = {}",
                    self.offsets.extra_cfa_len
                );
            }
            dump_writeln!(out, indent, "</Offsets>");
            if let Some(jpeg_preview) = self.jpeg_preview() {
                jpeg_preview.write_dump(out, indent);
            } else {
                dump_writeln!(out, indent, "ERROR: JPEG Preview not found");
            }
            if let Some(meta_container) = self.meta_container() {
                meta_container.write_dump(out, indent);
            } else {
                dump_writeln!(out, indent, "ERROR: Meta container not found");
            }
            if let Some(extra_meta_container) = self.extra_meta_container() {
                extra_meta_container.write_dump(out, indent);
            } else if self.offsets.extra_meta_offset != 0 && self.offsets.extra_meta_len != 0 {
                dump_writeln!(out, indent, "ERROR: Extra meta container not found");
            }
            dump_writeln!(out, indent, "<CFA @{}>", self.offsets.cfa_offset);
            {
                let indent = indent + 1;
                if let Some(cfa_container) = self.cfa_container() {
                    cfa_container.write_dump(out, indent);
                    if let Some(dir) = cfa_container.directory(0).and_then(|dir| {
                        dir.ifd_in_entry(
                            cfa_container,
                            FUJI_TAG_RAW_SUBIFD,
                            Some("Raw.Fujifilm"),
                            Some(&super::MNOTE_FUJIFILM_RAWIFD_TAG_NAMES),
                        )
                    }) {
                        dir.write_dump(out, indent);
                    }
                } else {
                    dump_writeln!(out, indent, "ERROR: CFA container not found");
                }
            }
            dump_writeln!(out, indent, "</CFA>");
            if self.offsets.extra_cfa_offset != 0 {
                dump_writeln!(out, indent, "<CFA2 @{}>", self.offsets.cfa_offset);
                {
                    let indent = indent + 1;
                    if let Some(cfa_container) = self.cfa_container2() {
                        cfa_container.write_dump(out, indent);
                        if let Some(dir) = cfa_container.directory(0).and_then(|dir| {
                            dir.ifd_in_entry(
                                cfa_container,
                                FUJI_TAG_RAW_SUBIFD,
                                Some("Raw.Fujifilm"),
                                Some(&super::MNOTE_FUJIFILM_RAWIFD_TAG_NAMES),
                            )
                        }) {
                            dir.write_dump(out, indent);
                        }
                    } else {
                        dump_writeln!(out, indent, "ERROR: Extra CFA container not found");
                    }
                }
                dump_writeln!(out, indent, "</CFA2>");
            }
        }
        dump_writeln!(out, indent, "</RAF Container>");
    }
}

/// The RAW dimensions
pub(super) const TAG_SENSOR_DIMENSION: u16 = 0x100;
/// Top Left of activate area
pub(super) const TAG_IMG_TOP_LEFT: u16 = 0x110;
/// Width Height of activate area
pub(super) const TAG_IMG_HEIGHT_WIDTH: u16 = 0x111;
// Aspect Ratio. w / h.
const TAG_IMG_ASPECT_RATIO: u16 = 0x115;
const TAG_OUTPUT_HEIGHT_WIDTH: u16 = 0x121;
/// Some info about the RAW. Sametime called "layout".
pub(super) const TAG_RAW_INFO: u16 = 0x130;
/// Colour Filter Array pattern
pub(super) const TAG_CFA_PATTERN: u16 = 0x131;
/// White balance, "old style". The presence of this seems to
/// correlate with the raw data no being a TIFF container.
pub(super) const TAG_WB_OLD: u16 = 0x2ff0;
const TAG_EXPOSURE_BIAS: u16 = 0x9650;
const TAG_RAF_DATA: u16 = 0xc000;

// TIFF tags

/// The subifd containing the raw data.
pub(super) const FUJI_TAG_RAW_SUBIFD: u16 = 0xf000;
/// Width of the raw image.
#[allow(dead_code)]
pub(super) const FUJI_TAG_RAW_WIDTH: u16 = 0xf001;
/// Height of the raw image.
#[allow(dead_code)]
pub(super) const FUJI_TAG_RAW_HEIGHT: u16 = 0xf002;
/// Bits per sample.
pub(super) const FUJI_TAG_RAW_BPS: u16 = 0xf003;
/// Offset of the raw image.
pub(super) const FUJI_TAG_RAW_OFFSET: u16 = 0xf007;
/// Byte len of the raw image.
pub(super) const FUJI_TAG_RAW_BYTE_LEN: u16 = 0xf008;
/// Black level (GRB?)
pub(super) const FUJI_TAG_RAW_BLACK_LEVEL_GRB: u16 = 0xf00a;
/// White balance coefficients.
pub(super) const FUJI_TAG_RAW_WB_GRB: u16 = 0xf00e;

#[derive(Clone, Copy, Default)]
enum RafTagType {
    #[default]
    U32,
    U16x2,
    U16x4,
    Bytes,
}

lazy_static::lazy_static! {
    static ref META_TAG_NAMES: HashMap<u16, (&'static str, RafTagType)> = HashMap::from([
        (TAG_SENSOR_DIMENSION, ("SensorDimension", RafTagType::U16x2)),
        (TAG_IMG_TOP_LEFT, ("ImageTopLeft", RafTagType::U16x2)),
        (TAG_IMG_HEIGHT_WIDTH, ("ImageHeightWidth", RafTagType::U16x2)),
        (TAG_IMG_ASPECT_RATIO, ("ImageAspectRatio", RafTagType::U16x2)),
        (TAG_OUTPUT_HEIGHT_WIDTH, ("OutputHeightWidth", RafTagType::U16x2)),
        (TAG_RAW_INFO, ("RawInfo", RafTagType::U32)),
        (TAG_CFA_PATTERN, ("CfaPattern", RafTagType::Bytes)),
        (TAG_WB_OLD, ("WhiteBalanceOld", RafTagType::U16x4)),
        (TAG_EXPOSURE_BIAS, ("ExposureBias", RafTagType::U32)),
        (TAG_RAF_DATA, ("RafData", RafTagType::Bytes)),
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
            Self::Int(n) => write!(f, "0x{n:08x}"),
            Self::Bytes(b) => {
                if b.len() > 36 {
                    write!(f, "bytes {:?}... len={}", &b[0..36], b.len())
                } else {
                    write!(f, "bytes {b:?} len={}", b.len())
                }
            }
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

#[derive(Debug)]
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
                let mut v = uninit_vec!(sz as usize);
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
    fn write_dump<W: std::io::Write + ?Sized>(&self, out: &mut W, indent: u32) {
        dump_writeln!(
            out,
            indent,
            "<RAF Meta Container @{}>",
            self.view.borrow().offset()
        );
        {
            let indent = indent + 1;
            for (tag, value) in &self.tags {
                let (tag_name, tag_type) = META_TAG_NAMES
                    .get(tag)
                    .cloned()
                    .unwrap_or(("", RafTagType::default()));
                let value = match tag_type {
                    RafTagType::U32 => format!("{value}"),
                    RafTagType::U16x2 => Size::try_from(value)
                        .map(|size| format!("{}, {}", size.width, size.height))
                        .unwrap_or_default(),
                    RafTagType::Bytes => format!("{value}"),
                    _ => String::default(),
                };
                dump_writeln!(
                    out,
                    indent,
                    "<0x{:x}={}> {} = {}",
                    tag,
                    tag,
                    tag_name,
                    value
                );
            }
        }
        dump_writeln!(out, indent, "</RAF Meta Container>");
    }
}

#[cfg(test)]
mod test {

    use std::convert::TryFrom;

    use super::Value;
    use crate::{Point, Size};

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
