// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - canon/crw/ciff.rs
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

pub(crate) mod container;

use std::collections::BTreeMap;
use std::convert::TryFrom;
use std::io::{Read, Seek, SeekFrom};

use byteorder::{BigEndian, LittleEndian, ReadBytesExt};
use num_enum::{IntoPrimitive, TryFromPrimitive};

use crate::container::{Endian, RawContainer};
use crate::io;
use crate::utils;
use crate::{Dump, Error, Result};
pub(crate) use container::Container;

/// Storage location bit mask
const STORAGELOC_MASK: u16 = 0xc000;
/// Format of the data
const FORMAT_MASK: u16 = 0x3800;
/// Include the format, because the last
/// part is non significant
//const TAGCODE_MASK: u16 = 0x3fff;

/// Tags for the CIFF records.
///
/// List made by a combination of the CIFF spec and
/// what exifprobe by Duane H. Hesser has.
#[derive(Copy, Clone, Debug, Ord, Eq, PartialEq, PartialOrd, TryFromPrimitive, IntoPrimitive)]
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
    FocalLength = 0x1029,
    ShotInfo = 0x102a,
    ColourInfo2 = 0x102c,
    CameraSettings = 0x102d,
    SensorInfo = 0x1031,
    CustomFunctions = 0x1033,
    PictureInfo = 0x1038,
    WhiteBalanceTable = 0x10a9,
    ColourSpace = 0x10b4,

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
    DecoderTable = 0x1835,

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
    Unknown0036 = 0x0036,
    Unknown003f = 0x003f,
    Unknown0040 = 0x0040,
    Unknown0041 = 0x0041,
    Unknown1026 = 0x1026,
    Unknown1030 = 0x1030,
    Unknown103c = 0x103c,
    Unknown107f = 0x107f,
    Unknown1039 = 0x1039,
    Unknown1093 = 0x1093,
    Unknown10c0 = 0x10c0,
    Unknown10c1 = 0x10c1,
    Unknown10c2 = 0x10c2,
    Unknown10aa = 0x10aa,
    Unknown10ad = 0x10ad,
    Unknown10a8 = 0x10a8,
    Unknown10ae = 0x10ae,
    Unknown10af = 0x10af,
    Unknown10b5 = 0x10b5,
    Unknown1819 = 0x1819,
    Unknown183b = 0x183b,
    Unknown4006 = 0x4006,
    // Seems to be the camera region
    Unknown480d = 0x480d,
    Unknown500a = 0x500a,
    // BodySenstivity? (first u32 only)
    Unknown501c = 0x501c,
    Unknown5028 = 0x5028,
    Unknown5029 = 0x5029,
    Unknown5034 = 0x5034,
    Unknown5803 = 0x5803,
    Unknown5804 = 0x5804,
    // BodyID (first u16 only)
    Unknown580b = 0x580b,
    Unknown5814 = 0x5814,
    Unknown5817 = 0x5817,
    Unknown5834 = 0x5834,

    #[num_enum(default)]
    Other = 0xffff,
}

impl Tag {
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

pub(crate) struct ImageSpec {
    /// Width (horizontal) in pixel.
    pub(crate) image_width: u32,
    /// Height (vertical) in pixel.
    pub(crate) image_height: u32,
    ///  Pixel aspect ratio.
    /*float32*/
    _pixel_aspect_ratio: u32,
    /// Rotation angle in degrees.
    _rotation_angle: i32,
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
            _rotation_angle: rotation_angle,
            component_bit_depth,
            _colour_bit_depth: colour_bit_depth,
            _colour_bw: colour_bw,
        })
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
#[derive(Debug)]
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

        let tag = Tag::try_from(type_code).map_err(|_| Error::FormatError)?;
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

    fn count(&self) -> Option<u32> {
        if let Record::InHeap((_, length)) = self.data {
            match self.type_() {
                RecordType::Byte => Some(length),
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
                    match type_ {
                        RecordType::Ascii => {
                            let mut s = vec![0; len as usize];
                            let mut view = container.borrow_view_mut();
                            let _ = view
                                .seek(SeekFrom::Start(pos as u64))
                                .and_then(|_| view.read_exact(&mut s))
                                .map(|_| {
                                    for s in s.split(|b| *b == 0) {
                                        let s = utils::from_maybe_nul_terminated(s);
                                        if !s.is_empty() {
                                            dump_writeln!(out, indent, "'{}'", s);
                                        }
                                    }
                                });
                        }
                        RecordType::Word => {
                            if let Some(count) = self.count() {
                                let mut values = vec![0_u16; count as usize];
                                let mut view = container.borrow_view_mut();
                                let _ = view
                                    .read_endian_u16_array(&mut values, container.endian())
                                    .map(|_| {
                                        dump_writeln!(out, indent, "{:?}", values);
                                    });
                            }
                        }
                        RecordType::DWord => {
                            if let Some(count) = self.count() {
                                let mut values = vec![0_u32; count as usize];
                                let mut view = container.borrow_view_mut();
                                let _ = view
                                    .read_endian_u32_array(&mut values, container.endian())
                                    .map(|_| {
                                        dump_writeln!(out, indent, "{:?}", values);
                                    });
                            }
                        }
                        RecordType::Heap1 | RecordType::Heap2 => {
                            let mut heap = Heap::new(pos, len);
                            let result =
                                heap.load(&mut container.borrow_view_mut(), container.endian());
                            // Can't map directly the result because
                            // of the borrow_view_mut()
                            if result.is_ok() {
                                heap.write_dump(out, indent, container);
                            };
                        }
                        _ => {
                            if let Some(count) = self.count() {
                                dump_writeln!(out, indent, "Count: {}", count);
                            }
                        }
                    }
                }
                Record::InRec(d) => {
                    dump_writeln!(out, indent, "In record");
                    match type_ {
                        RecordType::Ascii => {
                            let s = utils::from_maybe_nul_terminated(&d);
                            dump_writeln!(out, indent, "'{}'", s);
                        }
                        RecordType::Word => {
                            let mut io = std::io::Cursor::new(d);
                            let mut values = Vec::new();
                            for _ in 0..4 {
                                let _ = match container.endian() {
                                    Endian::Big => io.read_u16::<BigEndian>(),
                                    Endian::Little => io.read_u16::<LittleEndian>(),
                                    _ => Err(std::io::Error::new(
                                        std::io::ErrorKind::Other,
                                        "invalid endian",
                                    )),
                                }
                                .map(|v| {
                                    values.push(v);
                                });
                            }
                            dump_writeln!(out, indent, "{:?}", values);
                        }
                        RecordType::DWord => {
                            let mut io = std::io::Cursor::new(d);
                            let mut values = Vec::new();
                            for _ in 0..2 {
                                let _ = match container.endian() {
                                    Endian::Big => io.read_u32::<BigEndian>(),
                                    Endian::Little => io.read_u32::<LittleEndian>(),
                                    _ => Err(std::io::Error::new(
                                        std::io::ErrorKind::Other,
                                        "invalid endian",
                                    )),
                                }
                                .map(|v| {
                                    values.push(v);
                                });
                            }
                            dump_writeln!(out, indent, "{:?}", values);
                        }
                        RecordType::Byte | RecordType::Byte2 => {
                            dump_writeln!(out, indent, "{:?}", d)
                        }
                        _ => {}
                    }
                }
            }
        }
        dump_writeln!(out, indent, "</Record>");
    }
}

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
