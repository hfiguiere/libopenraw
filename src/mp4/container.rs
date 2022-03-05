/*
 * libopenraw - mp4/container.rs
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

use std::cell::RefCell;
use std::io::{Seek, SeekFrom};

use byteorder::{BigEndian, ReadBytesExt};

use crate::container;
use crate::io::View;
use crate::thumbnail;
use crate::thumbnail::Thumbnail;
use crate::{DataType, Error, Result};

/// Copy paste imports from mp4parse_capi
mod capi {

    #[derive(Debug)]
    pub struct ByteData {
        pub length: u32,
        pub data: *const u8,
    }

    impl Default for ByteData {
        fn default() -> Self {
            Self {
                length: 0,
                data: std::ptr::null(),
            }
        }
    }

    impl ByteData {
        pub(super) fn set_data(&mut self, data: &[u8]) {
            self.length = data.len() as u32;
            self.data = data.as_ptr();
        }
    }

    #[derive(Default, Debug)]
    pub struct TrackRawInfo {
        pub image_width: u16,
        pub image_height: u16,
        pub is_jpeg: bool,
        pub offset: u64,
        pub len: u64,
    }

    #[derive(Default)]
    pub struct CrawHeader {
        pub cncv: ByteData,
        pub thumb_w: u16,
        pub thumb_h: u16,
        pub thumbnail: ByteData,
        pub meta1: ByteData,
        pub meta2: ByteData,
        pub meta3: ByteData,
        pub meta4: ByteData,
    }
}

/// A container for ISO Media, aka MPEG4.
pub(crate) struct Container {
    view: RefCell<View>,
    context: mp4parse::MediaContext,
}

impl container::Container for Container {
    fn endian() -> container::Endian {
        container::Endian::Big
    }

    fn make_thumbnail(&self, desc: &thumbnail::ThumbDesc) -> Result<Thumbnail> {
        use std::io::Read;
        use thumbnail::Data;

        let data = match desc.data {
            Data::Bytes(ref b) => b.clone(),
            Data::Offset(ref offset) => {
                let mut view = self.view.borrow_mut();
                let mut data = Vec::new();
                data.resize(offset.len as usize, 0);
                view.seek(SeekFrom::Start(offset.offset))?;
                view.read_exact(data.as_mut_slice())?;
                data
            }
        };
        Ok(Thumbnail {
            width: desc.width,
            height: desc.height,
            data_type: desc.data_type,
            data,
        })
    }
}

impl Container {
    pub fn new(view: View) -> Self {
        Self {
            view: RefCell::new(view),
            context: mp4parse::MediaContext::new(),
        }
    }

    pub fn load(&mut self) -> Result<()> {
        mp4parse::read_mp4(self.view.get_mut(), &mut self.context)?;
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

        Ok(thumbnail::ThumbDesc {
            width,
            height,
            data: Data::Offset(DataOffset { offset, len }),
            data_type: DataType::Jpeg,
        })
    }

    /// Number of tracks in the ISO container
    pub(crate) fn track_count(&self) -> Result<usize> {
        let len = self.context.tracks.len();
        if len > u32::max_value as usize {
            return Err(Error::FormatError);
        }
        Ok(len)
    }

    /// Check if the track at index is a video track
    pub(crate) fn is_track_video(&self, index: usize) -> Result<bool> {
        if index >= self.context.tracks.len() {
            return Err(Error::NotFound);
        }
        let track = &self.context.tracks[index];
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
        if index >= self.context.tracks.len() {
            return Err(Error::NotFound);
        }
        let track = &self.context.tracks[index];
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
    pub(crate) fn craw_header(&self) -> Result<capi::CrawHeader> {
        let mut header = capi::CrawHeader::default();

        if self.context.craw.is_none() {
            return Err(Error::FormatError);
        }

        let craw = self.context.craw.as_ref().unwrap();
        header.cncv.set_data(&craw.cncv);
        header.thumb_w = craw.thumbnail.width;
        header.thumb_h = craw.thumbnail.height;
        header.thumbnail.set_data(&craw.thumbnail.data);
        if let Some(ref meta) = craw.meta1 {
            header.meta1.set_data(meta);
        }
        if let Some(ref meta) = craw.meta2 {
            header.meta2.set_data(meta);
        }
        if let Some(ref meta) = craw.meta3 {
            header.meta3.set_data(meta);
        }
        if let Some(ref meta) = craw.meta4 {
            header.meta4.set_data(meta);
        }
        Ok(header)
    }

    /// Return an entry from the Craw table as index.
    /// The entry contains offset, size tuple.
    fn craw_table_entry(&self, index: usize) -> Result<(u64, u64)> {
        if self.context.craw.is_none() {
            return Err(Error::FormatError);
        }
        let craw = self.context.craw.as_ref().unwrap();
        if craw.offsets.len() <= index {
            return Err(Error::NotFound);
        }
        let entry = craw.offsets[index];
        Ok(entry)
    }
}

#[cfg(test)]
mod test {}
