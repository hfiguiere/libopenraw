// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - mp4/container.rs
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

//! ISO Media container (MP4)

use std::cell::{RefCell, RefMut};
use std::io::{Seek, SeekFrom};
use std::rc::Rc;

use byteorder::{BigEndian, ReadBytesExt};
use mp4parse::craw;
use once_cell::unsync::OnceCell;

use crate::container;
#[cfg(feature = "dump")]
use crate::container::RawContainer;
use crate::io::{View, Viewer};
#[cfg(feature = "dump")]
use crate::jpeg;
use crate::thumbnail;
use crate::tiff;
use crate::Type as RawType;
use crate::{DataType, Dump, Error, Result};

/// Copy paste imports from mp4parse_capi
mod capi {
    #[derive(Default, Debug)]
    pub struct TrackRawInfo {
        pub image_width: u16,
        pub image_height: u16,
        pub is_jpeg: bool,
        pub offset: u64,
        pub len: u64,
    }

    impl crate::Dump for TrackRawInfo {
        #[cfg(feature = "dump")]
        fn write_dump<W: std::io::Write + ?Sized>(&self, out: &mut W, indent: u32) {
            dump_writeln!(out, indent, "<TrackRawInfo>");
            {
                let indent = indent + 1;

                dump_writeln!(
                    out,
                    indent,
                    "Image: Width {} x {}",
                    self.image_width,
                    self.image_height
                );
                dump_writeln!(
                    out,
                    indent,
                    "{}: {} bytes @{}",
                    if self.is_jpeg { "JPEG" } else { "Raw" },
                    self.len,
                    self.offset
                );
            }
            dump_writeln!(out, indent, "</TrackRawInfo>");
        }
    }
}

/// Type to hold the IFD and its `Viewer`.
type IfdHolder = (Rc<Viewer>, Rc<tiff::Container>);

/// A container for ISO Media, aka MPEG4.
pub(crate) struct Container {
    view: RefCell<View>,
    context: RefCell<mp4parse::MediaContext>,
    /// The metadata IFDs, and their viewer.
    meta_ifds: OnceCell<Vec<Option<IfdHolder>>>,
    raw_type: RawType,
}

impl container::RawContainer for Container {
    fn endian(&self) -> container::Endian {
        container::Endian::Big
    }

    fn borrow_view_mut(&self) -> RefMut<'_, View> {
        self.view.borrow_mut()
    }

    fn raw_type(&self) -> RawType {
        self.raw_type
    }
}

impl Container {
    /// New IFD read from `View`
    // XXX implement the reading offset. Currently assume 0.
    pub fn new(view: View, raw_type: RawType) -> Self {
        Self {
            view: RefCell::new(view),
            context: RefCell::default(),
            meta_ifds: OnceCell::new(),
            raw_type,
        }
    }

    pub(crate) fn load(&mut self) -> Result<()> {
        let context = mp4parse::read_mp4(self.view.get_mut())?;
        self.context.replace(context);
        Ok(())
    }

    /// Get the preview description for thumbnailing
    pub(crate) fn preview_desc(&self) -> Result<thumbnail::ThumbDesc> {
        use thumbnail::{Data, DataOffset};

        let preview_offset = self.craw_table_entry(1)?;

        // box (24) + content (8) + prvw box (8) + unknown (4)
        // We need to skip the "boxes" (ISO container)
        // And skip a short (16bits) value.
        let offset = preview_offset.0 + 44 + 2;

        let mut view = self.view.borrow_mut();
        view.seek(SeekFrom::Start(offset))?;
        let width = view.read_u16::<BigEndian>()? as u32;
        let height = view.read_u16::<BigEndian>()? as u32;
        view.seek(SeekFrom::Current(2))?;
        let len = view.read_u32::<BigEndian>()? as u64;

        if width == 0 || height == 0 || len == 0 {
            return Err(Error::NotFound);
        }

        log::debug!(
            "Found preview: {}x{} @{} len: {}",
            width,
            height,
            offset + 10,
            len
        );

        Ok(thumbnail::ThumbDesc {
            width,
            height,
            // Added 10 to the offset because we read 10 bytes
            data: Data::Offset(DataOffset {
                offset: offset + 10,
                len,
            }),
            data_type: DataType::Jpeg,
        })
    }

    /// Get the metadata at `idx`
    pub(crate) fn metadata_block(&self, idx: u32) -> Option<IfdHolder> {
        fn make_ifd_holder(
            data: Option<&Vec<u8>>,
            t: tiff::IfdType,
            raw_type: RawType,
        ) -> Option<IfdHolder> {
            data.and_then(|d| {
                if d.len() >= 4 {
                    // XXX so many copies
                    let length = d.len();
                    let cursor = Box::new(std::io::Cursor::new(d.clone()));
                    let viewer = Viewer::new(cursor, length as u64);
                    if let Ok(view) = Viewer::create_view(&viewer, 0) {
                        let mut ifd = tiff::Container::new(view, vec![t], raw_type);
                        ifd.load(None).expect("ifd load");
                        return Some((viewer, Rc::new(ifd)));
                    }
                }
                None
            })
        }

        let len = self
            .meta_ifds
            .get_or_init(|| {
                if let Ok(craw) = self.craw_header() {
                    vec![
                        make_ifd_holder(craw.meta1.as_ref(), tiff::IfdType::Main, self.raw_type),
                        make_ifd_holder(craw.meta2.as_ref(), tiff::IfdType::Exif, self.raw_type),
                        make_ifd_holder(
                            craw.meta3.as_ref(),
                            tiff::IfdType::MakerNote,
                            self.raw_type,
                        ),
                        make_ifd_holder(craw.meta4.as_ref(), tiff::IfdType::Other, self.raw_type),
                    ]
                } else {
                    vec![None; 4]
                }
            })
            .len();

        if len < idx as usize {
            None
        } else {
            self.meta_ifds
                .get()
                .and_then(|v| v[idx as usize].as_ref().cloned())
        }
    }

    /// Number of tracks in the ISO container
    pub(crate) fn track_count(&self) -> Result<usize> {
        let len = self.context.borrow().tracks.len();
        if len > u32::max_value as usize {
            return Err(Error::FormatError);
        }
        Ok(len)
    }

    /// Check if the track at index is a video track
    pub(crate) fn is_track_video(&self, index: usize) -> Result<bool> {
        let tracks = &self.context.borrow().tracks;
        if index >= tracks.len() {
            return Err(Error::NotFound);
        }
        let track = &tracks[index];
        if track.track_type == mp4parse::TrackType::Unknown {
            return Err(Error::NotFound);
        }
        // Assume we have a track_id.
        match track.track_id {
            Some(_) => {}
            None => return Err(Error::NotFound),
        };

        Ok(track.track_type == mp4parse::TrackType::Video)
    }

    /// Get the track at index if it is a CRaw.
    pub(crate) fn raw_track(&self, index: usize) -> Result<capi::TrackRawInfo> {
        let mut track_info = capi::TrackRawInfo::default();
        let tracks = &self.context.borrow().tracks;
        if index >= tracks.len() {
            return Err(Error::NotFound);
        }
        let track = &tracks[index];
        match track.track_type {
            mp4parse::TrackType::Video => {}
            _ => return Err(Error::NotFound),
        }

        let video = match track.stsd {
            // We assume there is only one.
            Some(ref data) => &data.descriptions[0],
            None => return Err(Error::FormatError),
        };

        let raw = match *video {
            mp4parse::SampleEntry::CanonCRAW(ref x) => x,
            _ => return Err(Error::FormatError),
        };

        track_info.image_width = raw.width;
        track_info.image_height = raw.height;
        track_info.is_jpeg = raw.is_jpeg;
        // assume there is an offset and samples size is constant
        track_info.len = if let Some(ref stsz) = track.stsz {
            if stsz.sample_size > 0 {
                stsz.sample_size as u64
            } else {
                stsz.sample_sizes[0] as u64
            }
        } else {
            0
        };
        track_info.offset = if let Some(ref stco) = track.stco {
            stco.offsets[0]
        } else {
            0
        };

        Ok(track_info)
    }

    /// Get the Craw header
    pub(crate) fn craw_header(&self) -> Result<std::cell::Ref<craw::CrawHeader>> {
        let craw = std::cell::Ref::map(self.context.borrow(), |context| &context.craw);
        if craw.is_none() {
            return Err(Error::FormatError);
        }

        Ok(std::cell::Ref::map(craw, |craw| craw.as_ref().unwrap()))
    }

    /// Return an entry from the Craw table as index.
    /// The entry contains offset, size tuple.
    fn craw_table_entry(&self, index: usize) -> Result<(u64, u64)> {
        let craw = &self.context.borrow().craw;
        if craw.is_none() {
            return Err(Error::FormatError);
        }
        let craw = craw.as_ref().unwrap();
        if craw.offsets.len() <= index {
            return Err(Error::NotFound);
        }
        let entry = craw.offsets[index];
        Ok(entry)
    }
}

impl Dump for craw::CanonThumbnail {
    #[cfg(feature = "dump")]
    fn write_dump<W: std::io::Write + ?Sized>(&self, out: &mut W, indent: u32) {
        dump_writeln!(
            out,
            indent,
            "Thumbnail: {}x{} {} bytes",
            self.width,
            self.height,
            self.data.len()
        );
    }
}

impl Dump for craw::CrawHeader {
    #[cfg(feature = "dump")]
    fn write_dump<W: std::io::Write + ?Sized>(&self, out: &mut W, indent: u32) {
        dump_writeln!(out, indent, "<CRaw header>");
        {
            let indent = indent + 1;
            dump_writeln!(out, indent, "'cncv': {} bytes", self.cncv.len());
            dump_writeln!(out, indent, "<offsets>");
            for (i, offset) in self.offsets.iter().enumerate() {
                dump_writeln!(
                    out,
                    indent + 1,
                    "(@{}, {} bytes){}",
                    offset.0,
                    offset.1,
                    if i == 1 { ": preview" } else { "" }
                );
            }
            dump_writeln!(out, indent, "</offsets>");
            dump_writeln!(
                out,
                indent,
                "Meta1: {} bytes",
                self.meta1.as_ref().map(|v| v.len()).unwrap_or(0)
            );
            dump_writeln!(
                out,
                indent,
                "Meta2: {} bytes",
                self.meta2.as_ref().map(|v| v.len()).unwrap_or(0)
            );
            dump_writeln!(
                out,
                indent,
                "Meta3: {} bytes",
                self.meta3.as_ref().map(|v| v.len()).unwrap_or(0)
            );
            dump_writeln!(
                out,
                indent,
                "Meta4: {} bytes",
                self.meta4.as_ref().map(|v| v.len()).unwrap_or(0)
            );
            self.thumbnail.write_dump(out, indent);
        }
        dump_writeln!(out, indent, "</CRaw header>");
    }
}

impl Dump for Container {
    #[cfg(feature = "dump")]
    fn write_dump<W: std::io::Write + ?Sized>(&self, out: &mut W, indent: u32) {
        dump_writeln!(
            out,
            indent,
            "<MP4 Iso Container @{}>",
            self.view.borrow().offset()
        );
        {
            let indent = indent + 1;
            if let Ok(craw_header) = self.craw_header() {
                craw_header.write_dump(out, indent);
            } else {
                dump_writeln!(out, indent, "ERROR: Craw Header not found");
            }
            if let Ok(preview_desc) = self.preview_desc() {
                // XXX get the data_type
                // XXX shall the be ThumbDesc impl ?
                dump_writeln!(
                    out,
                    indent,
                    "<Preview {} x {}, JPEG {} bytes>",
                    preview_desc.width,
                    preview_desc.height,
                    preview_desc.data.len()
                );
                {
                    let indent = indent + 1;
                    match preview_desc.data {
                        thumbnail::Data::Offset(ref offset) => {
                            if preview_desc.data_type == DataType::Jpeg {
                                if let Ok(view) =
                                    Viewer::create_subview(&self.borrow_view_mut(), offset.offset)
                                {
                                    let jpeg = jpeg::Container::new(view, self.raw_type);
                                    jpeg.write_dump(out, indent);
                                } else {
                                    dump_writeln!(out, indent, "Error loading preview");
                                }
                            } else {
                                dump_writeln!(out, indent, "Not JPEG");
                            }
                        }
                        _ => dump_writeln!(
                            out,
                            indent,
                            "Inline data {} bytes",
                            preview_desc.data.len()
                        ),
                    }
                }
                dump_writeln!(out, indent, "</Preview>");
            }

            let track_count = self.track_count().unwrap_or(0);
            dump_writeln!(out, indent, "Track count: {}", track_count);

            for i in 0..track_count {
                let is_video = self.is_track_video(i).unwrap_or(false);
                dump_writeln!(
                    out,
                    indent,
                    "<Track {}: {}>",
                    i,
                    if is_video {
                        if i == 2 {
                            "main Raw"
                        } else {
                            "is video"
                        }
                    } else {
                        "is not video"
                    }
                );
                if is_video {
                    let indent = indent + 1;

                    if let Ok(raw_track) = self.raw_track(i) {
                        raw_track.write_dump(out, indent);
                    }
                }
                dump_writeln!(out, indent, "</Track {}>", i);
            }

            for i in 0..4 {
                dump_writeln!(out, indent, "<Metadata Block {}>", i);
                {
                    let indent = indent + 1;
                    if let Some(holder) = self.metadata_block(i) {
                        holder.1.write_dump(out, indent);
                    }
                }
                dump_writeln!(out, indent, "</Metadata Block {}>", i);
            }
        }
        dump_writeln!(out, indent, "</MP4 Iso Container>");
    }
}
