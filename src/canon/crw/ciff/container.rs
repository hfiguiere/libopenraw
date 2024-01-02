// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - canon/crw/ciff/container.rs
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

//! The CIFF container. This is used only by CRW files.

use std::cell::{RefCell, RefMut};
use std::collections::HashMap;
use std::io::{Read, Seek, SeekFrom};

use byteorder::{BigEndian, ByteOrder, LittleEndian};
use chrono::TimeZone;
use once_cell::unsync::OnceCell;

use crate::container::{Endian, RawContainer};
use crate::io::View;
use crate::metadata;
use crate::tiff::{exif, Dir, Entry, IfdType, TagType};
use crate::utils;
use crate::{Dump, Type};

use super::{
    CameraSettings, Heap, HeapFileHeader, ImageSpec, Record, RecordData, RecordEntry, Tag,
};

type Converter = fn(record: &RecordEntry, container: &Container, tag: u16) -> Vec<Entry>;

struct CiffToExif {
    tag: u16,
    dest: IfdType,
    converter: Option<Converter>,
}

lazy_static::lazy_static! {
    static ref CIFF_EXIF_MAP: HashMap<Tag, CiffToExif> = HashMap::from([
        ( Tag::FocalLength, CiffToExif{ tag: exif::EXIF_TAG_FOCAL_LENGTH, dest: IfdType::Exif, converter: Some(translate_focal_length) } ),
        ( Tag::FileDescription, CiffToExif{ tag: exif::EXIF_TAG_IMAGE_DESCRIPTION, dest: IfdType::Main, converter: None } ),
        ( Tag::OriginalFileName, CiffToExif{ tag: exif::EXIF_TAG_DOCUMENT_NAME, dest: IfdType::Main, converter: None } ),
        ( Tag::TargetDistanceSetting, CiffToExif{ tag: exif::EXIF_TAG_SUBJECT_DISTANCE, dest: IfdType::Exif, converter: None } ),
        // ( TAG_RAWMAKEMODEL, CiffToExif{ exif::EXIF_TAG_MAKE, IfdType::Main, &translateMakeModel } ),
        // ( TAG_RAWMAKEMODEL, CiffToExif{ exif::EXIF_TAG_MODEL, IfdType::Main, &translateMakeModel } ),
        ( Tag::OwnerName, CiffToExif{ tag: exif::EXIF_TAG_CAMERA_OWNER_NAME, dest: IfdType::Exif, converter: Some(translate_string) } ),
        ( Tag::SerialNumber, CiffToExif{ tag: exif::EXIF_TAG_BODY_SERIAL_NUMBER, dest: IfdType::Exif, converter: Some(translate_serial) } ),
        ( Tag::CapturedTime, CiffToExif{ tag: 0, dest: IfdType::Exif, converter: Some(translate_date) } ),
        ( Tag::CameraSettings, CiffToExif{ tag: 0, dest: IfdType::Exif, converter: Some(translate_camera_settings) } ),
    ]);
}

fn translate_string(record: &RecordEntry, container: &Container, tag: u16) -> Vec<Entry> {
    if let Some(RecordData::Ascii(ref string)) = record.data(container) {
        vec![Entry::from_string(tag, &string[0])]
    } else {
        vec![]
    }
}

fn translate_serial(record: &RecordEntry, container: &Container, tag: u16) -> Vec<Entry> {
    if let Some(RecordData::DWord(ref serial)) = record.data(container) {
        vec![Entry::from_string(tag, &format!("{:X}", serial[0]))]
    } else {
        vec![]
    }
}

fn translate_camera_settings(_: &RecordEntry, container: &Container, _: u16) -> Vec<Entry> {
    let mut entries = vec![];
    if let Some(camera_settings) = container.camera_settings() {
        // Macro mode
        let value = camera_settings[1];
        if value == 1 {
            let mut buf = [0_u8; 4];
            container.endian_write_u16(&mut buf, 1);
            let entry = Entry::new(exif::EXIF_TAG_FLASH, TagType::Short as i16, 1, buf);
            entries.push(entry);
        }

        // Flash mode
        let value = camera_settings[4];
        let flash = match value {
            0 => 0_u16,        // off,
            1 => 0x19_u16,     // Auto
            2 => 0x01_u16,     // On
            3 | 5 => 0x41_u16, // Red-eye
            _ => 0_u16,
        };
        let mut buf = [0_u8; 4];
        container.endian_write_u16(&mut buf, flash);
        let entry = Entry::new(exif::EXIF_TAG_FLASH, TagType::Short as i16, 1, buf);
        entries.push(entry);

        // Metering mode
        let value = camera_settings[17];
        let metering = match value {
            0 => 0, // default
            1 => 3, // Spot
            2 => 1, // Average
            3 => 5, // Evaluative
            4 => 6, // Partial
            5 => 2, // Center wighted average
            _ => 0,
        };
        let mut buf = [0_u8; 4];
        container.endian_write_u16(&mut buf, metering);
        let entry = Entry::new(exif::EXIF_TAG_FLASH, TagType::Short as i16, 1, buf);
        entries.push(entry);

        // Exposure mode
        let value = camera_settings[20];
        let exposure = match value {
            0 => 0, // Easy
            1 => 2, // Program AE
            2 => 4, // Shutter Priority
            3 => 3, // Aperture Priority
            4 => 1, // Manual
            5 => 5, // DoF
            // 6 M-Dep
            // 7 Bulb
            6..=8 => 0, // Flexible
            _ => 0,
        };

        let mut buf = [0_u8; 4];
        container.endian_write_u16(&mut buf, exposure);
        let entry = Entry::new(exif::EXIF_TAG_METERING_MODE, TagType::Short as i16, 1, buf);
        entries.push(entry);
    }

    entries
}

fn translate_focal_length(record: &RecordEntry, container: &Container, tag: u16) -> Vec<Entry> {
    if let Some(RecordData::Word(ref value)) = record.data(container) {
        let fl = value[1] as u32;
        let mut fu = 0;
        if let Some(camera_settings) = container.camera_settings() {
            fu = camera_settings[25] as u32
        }
        let mut buf = vec![0_u8; 8];
        container.endian_write_u32(&mut buf, fl);
        container.endian_write_u32(&mut buf[4..], fu);
        let entry = Entry::new_with_data(tag, TagType::Rational as i16, 1, buf);
        vec![entry]
    } else {
        vec![]
    }
}

fn translate_date(record: &RecordEntry, container: &Container, _: u16) -> Vec<Entry> {
    if let Some(RecordData::DWord(ref value)) = record.data(container) {
        if let Some(date_time) = chrono::Utc.timestamp_opt(value[0] as i64, 0).single() {
            let date = format!("{}", date_time.format("%Y:%m:%d %H:%M:%S"));

            let original = Entry::from_string(exif::EXIF_TAG_DATE_TIME_ORIGINAL, &date);
            let digitized = Entry::from_string(exif::EXIF_TAG_DATE_TIME_DIGITIZED, &date);

            return vec![original, digitized];
        }
    }

    vec![]
}

#[derive(Debug)]
pub(crate) struct Container {
    view: RefCell<View>,
    /// Endian of the container.
    endian: RefCell<Endian>,
    header: OnceCell<HeapFileHeader>,
    heap: OnceCell<Heap>,
    image_props: OnceCell<Option<Heap>>,
    image_spec: OnceCell<Option<ImageSpec>>,
    camera_props: OnceCell<Option<Heap>>,
    exif_info: OnceCell<Option<Heap>>,
    camera_settings: OnceCell<Option<CameraSettings>>,

    dirs: OnceCell<Vec<Dir>>,
}

impl RawContainer for Container {
    fn endian(&self) -> Endian {
        *self.endian.borrow()
    }

    fn borrow_view_mut(&self) -> RefMut<'_, View> {
        self.view.borrow_mut()
    }

    fn raw_type(&self) -> Type {
        Type::Crw
    }

    /// Return a dir metadata iterator.
    fn dir_iterator(&self) -> metadata::Iterator {
        self.dirs().iter().into()
    }
}

impl Container {
    /// Create a new container for the view.
    pub(crate) fn new(view: View) -> Self {
        Self {
            view: RefCell::new(view),
            endian: RefCell::new(Endian::Unset),
            header: OnceCell::new(),
            heap: OnceCell::new(),
            image_props: OnceCell::new(),
            image_spec: OnceCell::new(),
            camera_props: OnceCell::new(),
            exif_info: OnceCell::new(),
            camera_settings: OnceCell::new(),
            dirs: OnceCell::new(),
        }
    }

    fn endian_write_u32(&self, buf: &mut [u8], n: u32) {
        match self.endian() {
            Endian::Big => BigEndian::write_u32(buf, n),
            Endian::Little => LittleEndian::write_u32(buf, n),
            _ => unreachable!(),
        }
    }

    fn endian_write_u16(&self, buf: &mut [u8], n: u16) {
        match self.endian() {
            Endian::Big => BigEndian::write_u16(buf, n),
            Endian::Little => LittleEndian::write_u16(buf, n),
            _ => unreachable!(),
        }
    }

    fn dirs(&self) -> &Vec<Dir> {
        self.dirs.get_or_init(|| {
            self.header();

            let mut main_ifd = Dir::new(self.endian(), IfdType::Main);
            if let Some(image_spec) = self.image_spec() {
                let mut buffer = [0u8; 4];

                self.endian_write_u32(&mut buffer, image_spec.image_width);
                let entry = Entry::new(
                    exif::EXIF_TAG_IMAGE_WIDTH,
                    exif::TagType::Long as i16,
                    1,
                    buffer,
                );
                main_ifd.entries.insert(exif::EXIF_TAG_IMAGE_WIDTH, entry);

                self.endian_write_u32(&mut buffer, image_spec.image_height);
                let entry = Entry::new(
                    exif::EXIF_TAG_IMAGE_LENGTH,
                    exif::TagType::Long as i16,
                    1,
                    buffer,
                );
                main_ifd.entries.insert(exif::EXIF_TAG_IMAGE_LENGTH, entry);

                self.endian_write_u16(&mut buffer, image_spec.component_bit_depth as u16);
                let entry = Entry::new(
                    exif::EXIF_TAG_BITS_PER_SAMPLE,
                    exif::TagType::Short as i16,
                    1,
                    buffer,
                );
                main_ifd
                    .entries
                    .insert(exif::EXIF_TAG_BITS_PER_SAMPLE, entry);

                self.endian_write_u16(&mut buffer, image_spec.exif_orientation());
                let entry = Entry::new(
                    exif::EXIF_TAG_ORIENTATION,
                    exif::TagType::Long as i16,
                    1,
                    buffer,
                );
                main_ifd.entries.insert(exif::EXIF_TAG_ORIENTATION, entry);
            }

            if let Some(make) = self.make_or_model(exif::EXIF_TAG_MAKE) {
                let entry = Entry::from_string(exif::EXIF_TAG_MAKE, &make);
                main_ifd.entries.insert(exif::EXIF_TAG_MAKE, entry);
            }
            if let Some(model) = self.make_or_model(exif::EXIF_TAG_MODEL) {
                let entry = Entry::from_string(exif::EXIF_TAG_MODEL, &model);
                main_ifd.entries.insert(exif::EXIF_TAG_MODEL, entry);
            }

            let mut exif_ifd = Dir::new(self.endian(), IfdType::Exif);
            if let Some(image_props) = self.image_props() {
                image_props.records().iter().for_each(|(_, record)| {
                    let entries = self.translate_record_entry(record, IfdType::Exif);
                    for entry in entries {
                        exif_ifd.entries.insert(entry.id, entry);
                    }
                })
            }
            if let Some(exif_info) = self.exif_info() {
                exif_info.records().iter().for_each(|(_, record)| {
                    let entries = self.translate_record_entry(record, IfdType::Exif);
                    for entry in entries {
                        exif_ifd.entries.insert(entry.id, entry);
                    }
                })
            }

            vec![main_ifd, exif_ifd]
        })
    }

    fn translate_record_entry(&self, record: &RecordEntry, ifd_type: IfdType) -> Vec<Entry> {
        if record.is_heap() {
            if let Some(heap) = record.heap(self) {
                return heap
                    .records()
                    .iter()
                    .flat_map(|(_, record)| self.translate_record_entry(record, ifd_type))
                    .collect();
            }
        }
        if let Some(ciff_to_exif) = CIFF_EXIF_MAP.get(&record.type_code) {
            if ciff_to_exif.dest == ifd_type {
                if let Some(converter) = ciff_to_exif.converter {
                    return converter(record, self, ciff_to_exif.tag);
                }
            }
        }
        vec![]
    }

    pub(crate) fn main_ifd(&self) -> Option<&Dir> {
        Some(&self.dirs()[0])
    }

    pub(crate) fn exif_ifd(&self) -> Option<&Dir> {
        Some(&self.dirs()[1])
    }

    fn header(&self) -> &HeapFileHeader {
        self.header.get_or_init(|| {
            let mut view = self.borrow_view_mut();
            let header = HeapFileHeader::from_view(&mut view).expect("Coudln't read file header");
            self.endian.replace(header.endian);
            header
        })
    }

    pub(crate) fn heap(&self) -> &Heap {
        self.heap.get_or_init(|| {
            let header = self.header();
            if header.type_ != *b"HEAP" && header.sub_type != *b"CCDR" {
                log::error!("Wrong CIFF header types");
            }
            let mut view = self.borrow_view_mut();
            // XXX check that header.len < view.len()
            let heaplen = view.len() - header.len as u64;
            // XXX heaplen should be < u32::MAX
            let mut heap = Heap::new(header.len, heaplen as u32);
            heap.load(&mut view, self.endian())
                .expect("Failed to load heap");

            heap
        })
    }

    fn heap_from_heap(&self, heap: &Heap, tag: super::Tag) -> Option<Heap> {
        heap.records()
            .get(&tag)
            .and_then(|r| r.heap(self))
            .or_else(|| {
                log::error!("CIFF: Heap for {:?} not found", tag);
                None
            })
    }

    fn image_props(&self) -> Option<&Heap> {
        self.image_props
            .get_or_init(|| {
                let heap = self.heap();
                self.heap_from_heap(heap, super::Tag::ImageProps)
            })
            .as_ref()
    }

    pub(crate) fn image_spec(&self) -> Option<&ImageSpec> {
        self.image_spec
            .get_or_init(|| {
                self.image_props().and_then(|heap| {
                    heap.records().get(&super::Tag::ImageInfo).and_then(|r| {
                        if let Record::InHeap((pos, _)) = r.data {
                            let mut view = self.borrow_view_mut();
                            ImageSpec::from_view(&mut view, pos, self.endian()).ok()
                        } else {
                            None
                        }
                    })
                })
            })
            .as_ref()
    }

    fn camera_props(&self) -> Option<&Heap> {
        self.camera_props
            .get_or_init(|| {
                self.image_props()
                    .and_then(|heap| self.heap_from_heap(heap, super::Tag::CameraObject))
                    .or_else(|| {
                        log::error!("CIFF: CameraProps not found");
                        None
                    })
            })
            .as_ref()
    }

    pub(crate) fn exif_info(&self) -> Option<&Heap> {
        self.exif_info
            .get_or_init(|| {
                self.image_props()
                    .and_then(|heap| self.heap_from_heap(heap, super::Tag::ExifInformation))
            })
            .as_ref()
    }

    pub(crate) fn make_or_model(&self, tag: u16) -> Option<String> {
        match tag {
            exif::EXIF_TAG_MAKE | exif::EXIF_TAG_MODEL => {}
            _ => return None,
        }
        self.camera_props().and_then(|heap| {
            heap.records()
                .get(&super::Tag::RawMakeModel)
                .and_then(|r| {
                    if let Record::InHeap((pos, len)) = r.data {
                        let mut s = vec![0; len as usize];
                        let mut view = self.borrow_view_mut();
                        view.seek(SeekFrom::Start(pos as u64)).ok()?;
                        view.read_exact(&mut s).ok()?;
                        let mut s = s.split(|b| *b == 0);
                        let mut v = s.next()?;
                        if tag == exif::EXIF_TAG_MODEL {
                            v = s.next()?;
                        }
                        Some(utils::from_maybe_nul_terminated(v))
                    } else {
                        log::error!("CIFF: Record RAWMAKEMODEL not in heap");
                        None
                    }
                })
                .or_else(|| {
                    log::error!("CIFF: Record RAWMAKEMODEL not found");
                    None
                })
        })
    }
    fn camera_settings(&self) -> Option<&CameraSettings> {
        self.camera_settings
            .get_or_init(|| {
                self.exif_info().and_then(|heap| {
                    heap.records()
                        .get(&super::Tag::CameraSettings)
                        .and_then(|r| {
                            let count = r.count()?;
                            if let Record::InHeap((pos, _)) = r.data {
                                let mut view = self.borrow_view_mut();
                                view.seek(SeekFrom::Start(pos as u64)).ok()?;
                                let mut settings = vec![0_u16; count as usize];
                                view.read_endian_u16_array(&mut settings, self.endian())
                                    .map_err(|err| {
                                        log::error!("CIFF: Not enough data for camera settings");
                                        err
                                    })
                                    .ok()?;
                                Some(settings)
                            } else {
                                None
                            }
                        })
                })
            })
            .as_ref()
    }

    pub(crate) fn raw_data_record(&self) -> Option<&RecordEntry> {
        self.heap().records().get(&super::Tag::RawImageData)
    }
}

impl Dump for Container {
    #[cfg(feature = "dump")]
    fn write_dump<W: std::io::Write + ?Sized>(&self, out: &mut W, indent: u32) {
        dump_writeln!(out, indent, "<CIFF Container>");
        {
            let indent = indent + 1;

            let header = self.header();
            header.write_dump(out, indent);

            let heap = self.heap();
            heap.write_dump(out, indent, self);
        }
        dump_writeln!(out, indent, "</CIFF Container>");
    }
}
