//! Module for parsing Canon CR3 files that are ISO Base Media Format
//!  aka video/mp4 streams.

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

use super::{
    be_u16, be_u32, be_u64, read_buf, skip, skip_box_content, skip_box_remain, BMFFBox, Error,
};
use boxes::BoxType;
use std::io::Read;

pub const HEADER_UUID: [u8; 16] = [
    0x85, 0xc0, 0xb6, 0x87, 0x82, 0x0f, 0x11, 0xe0, 0x81, 0x11, 0xf4, 0xce, 0x46, 0x2b, 0x6a, 0x48,
];

#[allow(dead_code)]
pub const XPACKET_UUID: [u8; 16] = [
    0xbe, 0x7a, 0xcf, 0xcb, 0x97, 0xa9, 0x42, 0xe8, 0x9c, 0x71, 0x99, 0x94, 0x91, 0xe3, 0xaf, 0xac,
];

#[allow(dead_code)]
pub const PREVIEW_UUID: [u8; 16] = [
    0xea, 0xf4, 0x2b, 0x5e, 0x1c, 0x98, 0x4b, 0x88, 0xb9, 0xfb, 0xb7, 0xdc, 0x40, 0x6e, 0x4d, 0x16,
];

/// Canon Thumbnail
#[derive(Debug, Default)]
pub struct CanonThumbnail {
    pub width: u16,
    pub height: u16,
    pub data: Vec<u8>,
}

/// Canon CRAW data ('crx ' brand files)
#[derive(Debug, Default)]
pub struct CrawHeader {
    pub cncv: Vec<u8>,
    pub offsets: Vec<(u64, u64)>,
    pub meta1: Option<Vec<u8>>,
    pub meta2: Option<Vec<u8>>,
    pub meta3: Option<Vec<u8>>,
    pub meta4: Option<Vec<u8>>,
    pub thumbnail: CanonThumbnail,
}

#[derive(Debug, Clone)]
pub struct CanonCRAWEntry {
    data_reference_index: u16,
    pub width: u16,
    pub height: u16,
    pub is_jpeg: bool,
}

/// Parse the CRAW entry inside the video sample entry.
pub fn read_craw_entry<T: Read>(
    src: &mut BMFFBox<T>,
    width: u16,
    height: u16,
    data_reference_index: u16,
) -> super::Result<super::SampleEntry> {
    skip(src, 54)?;
    let mut is_jpeg = false;
    {
        let mut iter = src.box_iter();
        while let Some(mut b) = iter.next_box()? {
            debug!("Box size {}", b.head.size);
            match b.head.name {
                BoxType::QTJPEGAtom => {
                    is_jpeg = true;
                }
                BoxType::CanonCMP1 => {}
                _ => {
                    debug!("Unsupported box '{:?}' in CRAW", b.head.name);
                }
            }
            skip_box_remain(&mut b)?;
        }
    }
    skip_box_remain(src)?;
    check_parser_state!(src.content);

    Ok(super::SampleEntry::CanonCRAW(CanonCRAWEntry {
        data_reference_index: data_reference_index,
        width: width,
        height: height,
        is_jpeg: is_jpeg,
    }))
}

pub fn parse_craw_header<T: Read>(f: &mut BMFFBox<T>) -> super::Result<CrawHeader> {
    let mut header = CrawHeader::default();
    let mut iter = f.box_iter();
    while let Some(mut b) = iter.next_box()? {
        match b.head.name {
            BoxType::CanonCompressorVersion => {
                let size = b.head.size - b.head.offset;
                let data = read_buf(&mut b, size)?;
                header.cncv = data.to_vec();
                skip_box_remain(&mut b)?;
            }
            BoxType::CanonTableOffset => {
                let count = be_u32(&mut b)?;
                for _i in 0..count {
                    skip(&mut b, 4)?; // index. We do not care.
                    let offset = be_u64(&mut b)?;
                    let size = be_u64(&mut b)?;
                    if offset == 0 || size == 0 {
                        break;
                    }
                    header.offsets.push((offset, size));
                }
                skip_box_remain(&mut b)?;
            }
            BoxType::CanonMeta1
            | BoxType::CanonMeta2
            | BoxType::CanonMeta3
            | BoxType::CanonMeta4 => {
                let len = b.head.size - b.head.offset;
                let data = read_buf(&mut b, len)?;
                let data = data.to_vec();
                match b.head.name {
                    BoxType::CanonMeta1 => header.meta1 = Some(data),
                    BoxType::CanonMeta2 => header.meta2 = Some(data),
                    BoxType::CanonMeta3 => header.meta3 = Some(data),
                    BoxType::CanonMeta4 => header.meta4 = Some(data),
                    _ => unreachable!(),
                }
            }
            BoxType::CanonThumbnail => {
                skip(&mut b, 4)?;
                let width = be_u16(&mut b)?;
                let height = be_u16(&mut b)?;
                let jpeg_size = be_u32(&mut b)?;
                skip(&mut b, 4)?;
                if (jpeg_size as u64) + b.head.offset + 16u64 > b.head.size {
                    return Err(Error::InvalidData("short box size for JPEG data"));
                }
                let data = read_buf(&mut b, jpeg_size as u64)?;
                header.thumbnail = CanonThumbnail {
                    width: width,
                    height: height,
                    data: data.to_vec(),
                };
                skip_box_remain(&mut b)?;
            }
            _ => skip_box_content(&mut b)?,
        }
    }

    debug!("{:?}", header);
    Ok(header)
}
