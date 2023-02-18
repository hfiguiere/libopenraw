// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - canon/crw/ciff/container.rs
 *
 * Copyright (C) 2022-2023 Hubert Figuière
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
use std::io::{Read, Seek, SeekFrom};

use once_cell::unsync::OnceCell;

use crate::container::{Endian, RawContainer};
use crate::io::View;
use crate::tiff::exif;
use crate::utils;
use crate::{Dump, Type};

use super::{CameraSettings, Heap, HeapFileHeader, ImageSpec, Record, RecordEntry};

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
    _camera_settings: OnceCell<Option<CameraSettings>>,
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
            _camera_settings: OnceCell::new(),
        }
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
            .and_then(|r| {
                if let Record::InHeap((pos, len)) = r.data {
                    let mut heap = Heap::new(pos, len);
                    let mut view = self.borrow_view_mut();
                    heap.load(&mut view, self.endian())
                        .expect("Failed to load heap");
                    Some(heap)
                } else {
                    None
                }
            })
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
    fn _camera_settings(&self, view: &mut View, endian: Endian) -> Option<&CameraSettings> {
        self._camera_settings
            .get_or_init(|| {
                self.exif_info().and_then(|heap| {
                    heap.records()
                        .get(&super::Tag::CameraSettings)
                        .and_then(|r| {
                            let count = r.count()?;
                            if let Record::InHeap((pos, _)) = r.data {
                                view.seek(SeekFrom::Start(pos as u64)).ok()?;
                                let mut settings = vec![0_u16; count as usize];
                                view.read_endian_u16_array(&mut settings, endian)
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
