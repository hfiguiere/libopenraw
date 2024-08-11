// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - canon/crw/ciff.rs
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

pub(crate) mod container;

use std::collections::BTreeMap;
use std::io::{Read, Seek, SeekFrom};

use num_enum::{FromPrimitive, IntoPrimitive};

use crate::container::{Endian, RawContainer};
use crate::io;
use crate::utils;
use crate::{Dump, Error, Result};
pub(crate) use container::Container;

/// Storage location bit mask
const STORAGELOC_MASK: u16 = 0xc000;
/// Format of the data
const FORMAT_MASK: u16 = 0x3800;
/// Include the format, as it is significant
const TAGCODE_MASK: u16 = 0x3fff;

/// Tags for the CIFF records.
///
/// List made by a combination of the CIFF spec and
/// what exifprobe by Duane H. Hesser has.
#[derive(
    Copy, Clone, Debug, Ord, Eq, PartialEq, PartialOrd, FromPrimitive, IntoPrimitive, Hash,
)]
#[repr(u16)]
pub(super) enum Tag {
    NullRecord = 0x0000,
    FreeBytes = 0x0001,
    ColourInfo1 = 0x0032,
    FileDescription = 0x0805,
    RawMakeModel = 0x080a,
    FirmwareVersion = 0x080b,
    ComponentVersion = 0x080c,
    RomOperationMode = 0x080d,
    OwnerName = 0x0810,
    ImageType = 0x0815,
    OriginalFileName = 0x0816,
    ThumbnailFileName = 0x0817,

    TargetImageType = 0x100a,
    ShutterReleaseMethod = 0x1010,
    ShutterReleaseTiming = 0x1011,
    ReleaseSetting = 0x1016,
    BaseISO = 0x101c,
    ExifISO = 0x1028,
    FocalLength = 0x1029,
    ShotInfo = 0x102a,
    ColourInfo2 = 0x102c,
    CameraSettings = 0x102d,
    WhiteSample = 0x1030,
    SensorInfo = 0x1031,
    CustomFunctions = 0x1033,
    PictureInfo = 0x1038,
    CanonFileInfo = 0x1093, // From ExifTool
    WhiteBalanceTable = 0x10a9,
    ColourTemperature = 0x10ae,
    ColourSpace = 0x10b4,
    RawJpgInfo = 0x10b5,

    ImageSpace = 0x1803,
    RecordId = 0x1804,
    SelfTimerTime = 0x1806,
    TargetDistanceSetting = 0x1807,
    SerialNumber = 0x180b,
    CapturedTime = 0x180e,
    ImageInfo = 0x1810,
    FlashInfo = 0x1813,
    MeasuredEv = 0x1814,
    FileNumber = 0x1817,
    ExposureInfo = 0x1818,
    CanonModelID = 0x1834,
    DecoderTable = 0x1835,
    SerialNumberFormat = 0x183b,

    RawImageData = 0x2005,
    JpegImage = 0x2007,
    JpegThumbnail = 0x2008,

    ImageDescription = 0x2804,
    CameraObject = 0x2807,
    ShootingRecord = 0x3002,
    MeasuredInfo = 0x3003,
    CameraSpecification = 0x3004,
    ImageProps = 0x300a,
    ExifInformation = 0x300b,

    // All the unknown fields.
    Unknown0006 = 0x0006,
    Unknown0036 = 0x0036,
    Unknown003f = 0x003f,
    Unknown0040 = 0x0040,
    Unknown0041 = 0x0041,
    Unknown1014 = 0x1014,
    Unknown1026 = 0x1026,
    Unknown103c = 0x103c,
    Unknown107f = 0x107f,
    Unknown1039 = 0x1039,
    Unknown10c0 = 0x10c0,
    Unknown10c1 = 0x10c1,
    Unknown10c2 = 0x10c2,
    Unknown10aa = 0x10aa,
    Unknown10ad = 0x10ad,
    Unknown10a8 = 0x10a8,
    Unknown10af = 0x10af,
    Unknown1805 = 0x1805,
    Unknown1812 = 0x1812,
    Unknown1819 = 0x1819,

    #[num_enum(default)]
    Other = 0xffff,
}

impl Tag {
    fn from_tagcode(tagcode: u16) -> Tag {
        let type_code = tagcode & TAGCODE_MASK;
        Self::from_primitive(type_code)
    }

    /// Return the record type based on the tag value.
    fn record_type(&self) -> RecordType {
        use RecordType::*;
        match u16::from(*self) & FORMAT_MASK {
            0x0000 => Byte,
            0x0800 => Ascii,
            0x1000 => Word,
            0x1800 => DWord,
            0x2000 => Byte2,
            0x2800 => Heap1,
            0x3000 => Heap2,
            _ => Unknown,
        }
    }
}

type CameraSettings = Vec<u16>;

#[derive(Debug)]
pub(crate) struct ImageSpec {
    /// Width (horizontal) in pixel.
    pub(crate) image_width: u32,
    /// Height (vertical) in pixel.
    pub(crate) image_height: u32,
    ///  Pixel aspect ratio.
    /*float32*/
    _pixel_aspect_ratio: u32,
    /// Rotation angle in degrees.
    rotation_angle: i32,
    /// Bit depth per component.
    pub(crate) component_bit_depth: u32,
    /// Bit depth for colour.
    _colour_bit_depth: u32,
    /// Colour or B&W. See CIFF spec.
    _colour_bw: u32,
}

impl ImageSpec {
    fn from_view(view: &mut io::View, at: u32, endian: Endian) -> Result<ImageSpec> {
        view.seek(SeekFrom::Start(at as u64))?;
        let image_width = view.read_endian_u32(endian)?;
        let image_height = view.read_endian_u32(endian)?;
        let pixel_aspect_ratio = view.read_endian_u32(endian)?;
        let rotation_angle = view.read_endian_i32(endian)?;
        let component_bit_depth = view.read_endian_u32(endian)?;
        let colour_bit_depth = view.read_endian_u32(endian)?;
        let colour_bw = view.read_endian_u32(endian)?;
        Ok(ImageSpec {
            image_width,
            image_height,
            _pixel_aspect_ratio: pixel_aspect_ratio,
            rotation_angle,
            component_bit_depth,
            _colour_bit_depth: colour_bit_depth,
            _colour_bw: colour_bw,
        })
    }

    /// Convert the rotation to an exif orientation.
    fn exif_orientation(&self) -> u16 {
        match self.rotation_angle {
            0 => 1,
            90 => 6,
            180 => 3,
            270 => 8,
            _ => 0,
        }
    }
}

#[derive(Debug)]
/// Record value, in heap or in rec.
pub(crate) enum Record {
    /// Inline data
    InRec([u8; 8]),
    /// InHeap at pos (.0) and length (.1)
    InHeap((u32, u32)),
}

#[repr(u16)]
#[derive(Debug, PartialEq)]
enum RecordType {
    Byte = 0x0000,
    Ascii = 0x0800,
    Word = 0x1000,
    DWord = 0x1800,
    Byte2 = 0x2000,
    Heap1 = 0x2800,
    Heap2 = 0x3000,
    Unknown,
}

#[derive(Debug)]
pub(crate) struct RecordEntry {
    type_code: Tag,
    pub(crate) data: Record,
}

pub(crate) enum RecordData {
    Byte(Vec<u8>),
    Ascii(Vec<String>),
    Word(Vec<u16>),
    DWord(Vec<u32>),
    Heap(Heap),
}

/// Tell if the `type_code` as data inline. (`InRec`)
#[inline]
fn in_record(type_code: u16) -> bool {
    type_code & STORAGELOC_MASK != 0
}

impl RecordEntry {
    /// Create a `RecordEntry` from the view, with `base` as
    /// the offset base (the Heap offset)
    fn from_view(base: u32, view: &mut io::View, endian: Endian) -> Result<RecordEntry> {
        let type_code = view.read_endian_u16(endian)?;
        let data = if in_record(type_code) {
            let mut bytes = [0_u8; 8];
            view.read_exact(&mut bytes)?;
            Record::InRec(bytes)
        } else {
            let len = view.read_endian_u32(endian)?;
            let offset = view.read_endian_u32(endian)? + base;
            Record::InHeap((offset, len))
        };

        let tag = Tag::from_tagcode(type_code);
        if tag == Tag::Other {
            log::error!("Unknown tag {:x}", type_code);
        }
        Ok(RecordEntry {
            type_code: tag,
            data,
        })
    }

    fn type_(&self) -> RecordType {
        self.type_code.record_type()
    }

    /// Return if the record a heap or not
    fn is_heap(&self) -> bool {
        self.type_() == RecordType::Heap1 || self.type_() == RecordType::Heap2
    }

    fn count(&self) -> Option<u32> {
        if let Record::InHeap((_, length)) = self.data {
            match self.type_() {
                RecordType::Byte | RecordType::Byte2 => Some(length),
                RecordType::Ascii => Some(length),
                RecordType::Word => Some(length / 2),
                RecordType::DWord => Some(length / 4),
                RecordType::Unknown => None,
                _ => Some(length),
            }
        } else {
            None
        }
    }

    fn inrec_read<T: io::FromBuf>(data: &[u8], endian: Endian) -> Result<Vec<T>> {
        let mut values = Vec::new();
        let count = 8 / std::mem::size_of::<T>();
        for i in 0..count {
            let v = match endian {
                Endian::Big => T::be_bytes(&data[i * std::mem::size_of::<T>()..]),
                Endian::Little => T::le_bytes(&data[i * std::mem::size_of::<T>()..]),
                _ => {
                    return Err(Error::IoError(std::io::Error::new(
                        std::io::ErrorKind::Other,
                        "invalid endian",
                    )))
                }
            };
            values.push(v);
        }
        Ok(values)
    }

    fn inline_data(&self, container: &Container) -> Option<RecordData> {
        if let Record::InRec(data) = self.data {
            match self.type_() {
                RecordType::Byte | RecordType::Byte2 => Some(RecordData::Byte(data.to_vec())),
                RecordType::Ascii => {
                    let s = utils::from_maybe_nul_terminated(&data);
                    Some(RecordData::Ascii(vec![s]))
                }
                RecordType::Word => {
                    let values = Self::inrec_read::<u16>(&data, container.endian()).ok()?;
                    Some(RecordData::Word(values))
                }
                RecordType::DWord => {
                    let values = Self::inrec_read::<u32>(&data, container.endian()).ok()?;
                    Some(RecordData::DWord(values))
                }
                RecordType::Unknown | RecordType::Heap1 | RecordType::Heap2 => None,
            }
        } else {
            None
        }
    }

    pub(super) fn data(&self, container: &Container) -> Option<RecordData> {
        match self.data {
            Record::InRec(_) => self.inline_data(container),
            Record::InHeap((pos, len)) => match self.type_() {
                RecordType::Byte | RecordType::Byte2 => {
                    let mut bytes = vec![0; len as usize];
                    let mut view = container.borrow_view_mut();
                    view.seek(SeekFrom::Start(pos as u64))
                        .and_then(|_| view.read_exact(&mut bytes))
                        .ok()
                        .map(|_| RecordData::Byte(bytes))
                }
                RecordType::Ascii => {
                    let mut s = vec![0; len as usize];
                    let mut view = container.borrow_view_mut();
                    view.seek(SeekFrom::Start(pos as u64)).ok()?;
                    view.read_exact(&mut s).ok()?;
                    Some(RecordData::Ascii(
                        s.split(|b| *b == 0)
                            .map(utils::from_maybe_nul_terminated)
                            .collect(),
                    ))
                }
                RecordType::Word => {
                    let count = self.count()?;
                    let mut values = vec![0_u16; count as usize];
                    let mut view = container.borrow_view_mut();
                    view.seek(SeekFrom::Start(pos as u64)).ok()?;
                    view.read_endian_u16_array(&mut values, container.endian())
                        .ok()?;
                    Some(RecordData::Word(values))
                }
                RecordType::DWord => {
                    let count = self.count()?;
                    let mut values = vec![0_u32; count as usize];
                    let mut view = container.borrow_view_mut();
                    view.seek(SeekFrom::Start(pos as u64)).ok()?;
                    view.read_endian_u32_array(&mut values, container.endian())
                        .ok()?;
                    Some(RecordData::DWord(values))
                }
                RecordType::Heap1 | RecordType::Heap2 => self.heap(container).map(RecordData::Heap),
                RecordType::Unknown => None,
            },
        }
    }

    fn heap(&self, container: &Container) -> Option<Heap> {
        if let Record::InHeap((pos, len)) = self.data {
            let mut heap = Heap::new(pos, len);
            let mut view = container.borrow_view_mut();
            heap.load(&mut view, container.endian())
                .expect("Failed to load heap");
            Some(heap)
        } else {
            None
        }
    }

    #[cfg(feature = "dump")]
    fn write_dump<W: std::io::Write + ?Sized>(
        &self,
        out: &mut W,
        indent: u32,
        container: &Container,
    ) {
        let type_ = self.type_();
        dump_writeln!(
            out,
            indent,
            "<Record '{:?}' = 0x{:x}, type: {:?}>",
            self.type_code,
            u16::from(self.type_code),
            type_
        );
        {
            let indent = indent + 1;
            match self.data {
                Record::InHeap((pos, len)) => {
                    dump_writeln!(out, indent, "In heap at {}, {} bytes", pos, len);
                }
                Record::InRec(_) => {
                    dump_writeln!(out, indent, "In record");
                }
            }
            match type_ {
                RecordType::Ascii => {
                    if let Some(RecordData::Ascii(s)) = self.data(container) {
                        s.iter().for_each(|s| {
                            if !s.is_empty() {
                                dump_writeln!(out, indent, "'{}'", s);
                            }
                        })
                    }
                }
                RecordType::Word => {
                    if let Some(RecordData::Word(values)) = self.data(container) {
                        dump_writeln!(out, indent, "{:?}", values);
                    }
                }
                RecordType::DWord => {
                    if let Some(RecordData::DWord(values)) = self.data(container) {
                        dump_writeln!(out, indent, "{:?}", values);
                    }
                }
                RecordType::Heap1 | RecordType::Heap2 => {
                    if let Some(RecordData::Heap(heap)) = self.data(container) {
                        heap.write_dump(out, indent, container);
                    }
                }
                _ => {
                    if let Some(count) = self.count() {
                        dump_writeln!(out, indent, "Count: {}", count);
                    }
                }
            }
        }
        dump_writeln!(out, indent, "</Record>");
    }
}

#[derive(Debug)]
pub(crate) struct Heap {
    pos: u32,
    pub(crate) len: u32,
    records: BTreeMap<Tag, RecordEntry>,
}

impl Heap {
    pub fn new(pos: u32, len: u32) -> Heap {
        Heap {
            pos,
            len,
            records: BTreeMap::new(),
        }
    }

    pub fn load(&mut self, view: &mut io::View, endian: Endian) -> Result<()> {
        if self.len < 4 {
            log::error!("CIFF Heap length too short");
            return Err(Error::FormatError);
        }
        if self.len as u64 + self.pos as u64 > view.len() {
            log::error!(
                "CIFF Heap too big: {} for {}",
                self.len + self.pos,
                view.len()
            );
            return Err(Error::FormatError);
        }
        view.seek(SeekFrom::Start(self.pos as u64 + self.len as u64 - 4))?;
        let heap_start = view.read_endian_u32(endian)?;
        if heap_start > self.len - 4 {
            log::error!("CIFF Heap start out of range {}", heap_start);
            return Err(Error::FormatError);
        }
        view.seek(SeekFrom::Start(self.pos as u64 + heap_start as u64))?;
        let num_records = view.read_endian_u16(endian)?;
        for _ in 0..num_records {
            let _ = RecordEntry::from_view(self.pos, view, endian)
                .map(|entry| {
                    self.records.insert(entry.type_code, entry);
                })
                .map_err(|err| {
                    log::error!("Failed to read record: {}", err);
                    err
                });
        }

        Ok(())
    }

    pub(super) fn records(&self) -> &BTreeMap<Tag, RecordEntry> {
        &self.records
    }

    #[cfg(feature = "dump")]
    fn write_dump<W: std::io::Write + ?Sized>(
        &self,
        out: &mut W,
        indent: u32,
        container: &Container,
    ) {
        dump_writeln!(out, indent, "<HEAP pos={} len={}>", self.pos, self.len);
        {
            let indent = indent + 1;
            dump_writeln!(out, indent, "Num records: {}", self.records.len());
            for record in self.records().values() {
                record.write_dump(out, indent, container);
            }
        }
        dump_writeln!(out, indent, "</HEAP");
    }
}

#[derive(Debug)]
pub(crate) struct HeapFileHeader {
    endian: Endian,
    len: u32,
    /// Type. b"HEAP"
    type_: [u8; 4],
    /// Sub-type. b"CCDR"
    sub_type: [u8; 4],
    /// Version. Higher word: 0x0001, Lower word: 0x0002.
    version: u32,
}

impl HeapFileHeader {
    /// Read the HeapFileHeader from the io view at the current location
    pub fn from_view(view: &mut io::View) -> Result<HeapFileHeader> {
        let mut byte_order = [0_u8; 2];
        view.read_exact(&mut byte_order)?;
        let endian = match &byte_order {
            b"II" => Endian::Little,
            b"MM" => Endian::Big,
            _ => return Err(Error::FormatError),
        };

        let len = view.read_endian_u32(endian)?;
        let mut type_ = [0_u8; 4];
        view.read_exact(&mut type_)?;
        let mut sub_type = [0_u8; 4];
        view.read_exact(&mut sub_type)?;
        let version = view.read_endian_u32(endian)?;
        Ok(HeapFileHeader {
            endian,
            len,
            type_,
            sub_type,
            version,
        })
    }
}

impl Dump for HeapFileHeader {
    #[cfg(feature = "dump")]
    fn write_dump<W: std::io::Write + ?Sized>(&self, out: &mut W, indent: u32) {
        dump_writeln!(out, indent, "<Header>");
        {
            let indent = indent + 1;
            dump_writeln!(out, indent, "Endian: {:?}", self.endian);
            dump_writeln!(out, indent, "Len: {}", self.len);
            dump_writeln!(
                out,
                indent,
                "Type: {:?}",
                String::from_utf8_lossy(&self.type_)
            );
            dump_writeln!(
                out,
                indent,
                "Subtype: {:?}",
                String::from_utf8_lossy(&self.sub_type)
            );
            dump_writeln!(
                out,
                indent,
                "Version: 0x{:0>4x} 0x{:0>4x}",
                (self.version & 0xffff0000) >> 16,
                self.version & 0x0000ffff
            );
        }
        dump_writeln!(out, indent, "</Header>");
    }
}
