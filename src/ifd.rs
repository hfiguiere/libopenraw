/*
 * libopenraw - ifd.rs
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

//! TIFF format (Image File Directories)

mod container;
mod dir;
mod entry;
pub mod exif;

use std::collections::HashMap;
use std::rc::Rc;

use byteorder::{BigEndian, LittleEndian};

use crate::container::{Endian, GenericContainer};
use crate::decompress;
use crate::thumbnail;
use crate::{DataType, Error, RawData, Result, TypeId};
pub(crate) use container::Container;
pub(crate) use dir::Dir;
pub(crate) use entry::Entry;

#[repr(u16)]
#[derive(Clone, Copy, Debug, PartialEq)]
/// TIFF (a RAW) compression values
pub enum Compression {
    /// Unknown - this value is never a valid one
    Unknown = 0,
    /// No compression
    None = 1,
    /// JPEG compression
    Jpeg = 6,
    /// Losless JPEG compression (like in DNG)
    LJpeg = 7,
    /// Sony ARW compression
    Arw = 32767,
    /// Nikon packed
    NikonPack = 32769,
    /// Nikon quantized
    NikonQuantized = 34713,
    /// What everybody seems to use
    Custom = 65535,
    // XXX figure out Olympus compression value
    // Olympus compression
    //Olympus = 65536
}

impl From<u16> for Compression {
    /// 0 and any unknown value will yiel `Unknown`
    fn from(v: u16) -> Compression {
        use Compression::*;

        match v {
            1 => None,
            6 => Jpeg,
            7 => LJpeg,
            32767 => Arw,
            32769 => NikonPack,
            34712 => NikonQuantized,
            65535 => Custom,
            // 65536 => Olympus,
            _ => Unknown,
        }
    }
}

/// Type of IFD
#[derive(Clone, Copy)]
pub enum Type {
    /// Main IFD (see TIFF)
    Main,
    /// CFA specific IFD
    Cfa,
    /// Exif IFD
    Exif,
    /// MakerNote IFD
    MakerNote,
    /// Sub IFD
    SubIfd,
    /// Any other IFD
    Other,
}

/// Trait for Ifd
pub trait Ifd {
    /// Return the type if IFD
    fn ifd_type(&self) -> Type;

    fn endian(&self) -> Endian;

    /// The number of entries
    fn num_entries(&self) -> usize;

    /// Return the entry for the `tag`.
    fn entry(&self, tag: u16) -> Option<&Entry>;

    /// Get the value for entry, at index.
    fn entry_value<T>(&self, entry: &Entry, index: u32) -> Option<T>
    where
        T: exif::ExifValue,
    {
        match self.endian() {
            Endian::Big => entry.value_at_index::<T, BigEndian>(index),
            Endian::Little => entry.value_at_index::<T, LittleEndian>(index),
            _ => unreachable!("Endian unset"),
        }
    }

    /// Get value for tag.
    fn value<T>(&self, tag: u16) -> Option<T>
    where
        T: exif::ExifValue,
    {
        self.entry(tag).and_then(|e| self.entry_value::<T>(e, 0))
    }
}

pub type MakeToIdMap = std::collections::HashMap<&'static str, TypeId>;

/// Identify a files using the Exif data
pub(crate) fn identify_with_exif(container: &Container, map: &MakeToIdMap) -> Option<TypeId> {
    container.directory(0).and_then(|dir| {
        if let Some(e) = dir.entry(exif::EXIF_TAG_MODEL) {
            e.value::<String, LittleEndian>()
                .and_then(|s| map.get(s.as_str()).copied())
        } else {
            None
        }
    })
}

/// Find the CFA IFD
pub(crate) fn tiff_locate_cfa_ifd(container: &Container) -> Option<Rc<Dir>> {
    let dir = container.directory(0)?;

    if dir.is_primary() {
        log::debug!("dir0 is primary");
        return Some(dir);
    }
    dir.get_sub_ifds(container)
        .and_then(|subifds| subifds.iter().find(|d| d.is_primary()).cloned())
}

/// Get the raw data
pub(crate) fn tiff_get_rawdata(container: &Container, dir: &Rc<Dir>) -> Result<RawData> {
    let mut offset = 0_u32;

    let mut bpc = dir
        .value::<u16>(exif::EXIF_TAG_BITS_PER_SAMPLE)
        .or_else(|| {
            log::error!("Unable to get bits per sample");
            None
        })
        .unwrap_or(0);

    let byte_len = dir
        .value::<u32>(exif::EXIF_TAG_STRIP_OFFSETS)
        .and_then(|v| {
            offset = v;
            let entry = dir.entry(exif::EXIF_TAG_STRIP_BYTE_COUNTS).or_else(|| {
                log::debug!("byte len not found");
                // XXX this might trigger the or_else below
                None
            })?;
            entry
                .value_array::<u32>(dir.endian())
                .map(|a| a.iter().sum())
        })
        .or_else(|| {
            // the tile are individual JPEGS....
            // XXX todo
            log::error!("Unimplemented");
            Some(0_u32)
        })
        .ok_or(Error::NotFound)?;

    let x = dir
        .uint_value(exif::EXIF_TAG_IMAGE_WIDTH)
        .or_else(|| {
            log::debug!("x not found");
            None
        })
        .ok_or(Error::NotFound)?;
    let y = dir
        .uint_value(exif::EXIF_TAG_IMAGE_LENGTH)
        .or_else(|| {
            log::debug!("y not found");
            None
        })
        .ok_or(Error::NotFound)?;
    let photom_int = dir
        .uint_value(exif::EXIF_TAG_PHOTOMETRIC_INTERPRETATION)
        .and_then(|v| exif::PhotometricInterpretation::try_from(v).ok())
        .unwrap_or(exif::PhotometricInterpretation::CFA);

    let compression = dir
        .value::<u16>(exif::EXIF_TAG_COMPRESSION)
        .map(Compression::from)
        .unwrap_or(Compression::None);

    println!("compression {:?}, bpc {}", compression, bpc);
    let data_type = match compression {
        Compression::None => DataType::Raw,
        Compression::NikonPack => DataType::Raw,
        _ => DataType::CompressedRaw,
    };

    // XXX get mosaic info

    let actual_bpc = bpc;
    if (bpc == 12 || bpc == 14) && (compression == Compression::None) && byte_len == (x * y * 2) {
        // it's 12 or 14 bpc, but we have 16 bpc data.
        log::debug!("setting bpc from {} to 16", bpc);
        bpc = 16;
    }
    let mut rawdata = if data_type == DataType::CompressedRaw {
        let data = container.load_buffer8(offset as u64, byte_len as u64);
        RawData::new8(x, y, actual_bpc, data_type, data)
    } else if bpc == 16 {
        let data = container.load_buffer16(offset as u64, byte_len as u64);
        RawData::new16(x, y, actual_bpc, data_type, data)
    } else if bpc == 12 || bpc == 8 {
        // XXX unpack data
        let data = decompress::unpack(
            container,
            x,
            y,
            bpc,
            compression,
            offset as u64,
            byte_len as usize,
        )?;
        RawData::new16(x, y, actual_bpc, data_type, data)
    } else {
        log::error!("Invalid RAW format");
        return Err(Error::InvalidFormat);
    };
    // XXX set mosaic_info

    // XXX maybe we don't need the if
    rawdata.set_compression(if data_type == DataType::CompressedRaw {
        compression
    } else {
        Compression::None
    });
    rawdata.set_photometric_interpretation(photom_int);
    if rawdata.white() == 0 {
        rawdata.set_white((1 << actual_bpc) - 1);
    }

    Ok(rawdata)
}

/// Get the thumbnails out of a TIFF
pub(crate) fn tiff_thumbnails(container: &Container) -> HashMap<u32, thumbnail::ThumbDesc> {
    let mut thumbnails = HashMap::new();

    let dirs = container.dirs();
    for dir in dirs {
        ifd_locate_thumbnail(dir, &mut thumbnails);

        if let Some(subdirs) = dir.get_sub_ifds(container) {
            for subdir in subdirs {
                ifd_locate_thumbnail(&subdir, &mut thumbnails);
            }
        }
    }

    log::debug!("Found {} thumbnails", thumbnails.len());

    thumbnails
}

pub(crate) fn ifd_locate_thumbnail(
    dir: &Rc<Dir>,
    thumbnails: &mut HashMap<u32, thumbnail::ThumbDesc>,
) {
    let mut data_type = DataType::Unknown;
    let subtype = if let Some(subtype) = dir.value::<u32>(exif::EXIF_TAG_NEW_SUBFILE_TYPE) {
        subtype
    } else {
        // XXX check if we are in the CFA IFD. We don't know this here.
        1
    };
    if subtype == 1 {
        let photom_int = dir
            .value::<u16>(exif::EXIF_TAG_PHOTOMETRIC_INTERPRETATION)
            .unwrap_or(exif::PhotometricInterpretation::Rgb as u16);
        let x = dir.uint_value(exif::EXIF_TAG_IMAGE_WIDTH).unwrap_or(0);
        let y = dir.uint_value(exif::EXIF_TAG_IMAGE_LENGTH).unwrap_or(0);
        let compression = dir.value::<u16>(exif::EXIF_TAG_COMPRESSION).unwrap_or(0);
        let mut byte_count = dir
            .value::<u32>(exif::EXIF_TAG_STRIP_BYTE_COUNTS)
            .unwrap_or(0);
        let mut offset = 0;
        let mut got_it = false;
        if let Some(v) = dir.value::<u32>(exif::EXIF_TAG_STRIP_OFFSETS) {
            offset = v;
            got_it = true;
        }
        if !got_it || compression == 6 || compression == 7 {
            if !got_it {
                byte_count = dir
                    .value::<u32>(exif::EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LENGTH)
                    .unwrap_or(0);
                if let Some(v) = dir.value::<u32>(exif::EXIF_TAG_JPEG_INTERCHANGE_FORMAT) {
                    got_it = true;
                    offset = v;
                }
            }
            if got_it {
                // workaround for CR2 files where 8RGB data is marked
                // as JPEG. Check the real data size.
                if x == 0 && y == 0 {
                    if byte_count >= (x * y * 3) {
                        // We ignore this, it's 8RGB in a Canon CR2 file.
                        // See bug 72270
                        log::debug!("8RGB as JPEG. Will ignore.");
                        data_type = DataType::Unknown
                    } else {
                        data_type = DataType::Jpeg;
                    }
                } else {
                    data_type = DataType::Jpeg;
                    if x == 0 || y == 0 {
                        // XXX load the JFIF stream and get the dimensions.
                        log::warn!("Couldn't get JPEG dimensions. (unimplemented)");
                    } else {
                        log::debug!("JPEG (supposed) dimensions x={} y={}", x, y);
                    }
                }
            }
        } else if photom_int == exif::PhotometricInterpretation::YCbCr as u16 {
            log::warn!("Unsupported YCbCr photometric interpretation in non JPEG.");
        } else {
            log::debug!("Found strip offsets");
            if x != 0 && y != 0 {
                // See bug 72270 - some CR2 have 16 bpc RGB thumbnails.
                // by default it is RGB8. Unless stated otherwise.
                let mut is_rgb8 = true;
                if let Some(arr) = dir
                    .entry(exif::EXIF_TAG_BITS_PER_SAMPLE)
                    .and_then(|e| e.value_array::<u16>(dir.endian()))
                    .or_else(|| {
                        log::debug!("Error getting bpc");
                        None
                    })
                {
                    for bpc in arr {
                        is_rgb8 = bpc == 8;
                        if !is_rgb8 {
                            log::debug!("bpc != 8 {}", bpc);
                            break;
                        }
                    }
                }
                if is_rgb8 {
                    data_type = DataType::PixmapRgb8;
                }
            }
        }
        if data_type != DataType::Unknown {
            let dim = std::cmp::max(x, y);
            // XXX compute
            // offset += offset();
            let desc = thumbnail::ThumbDesc {
                width: x,
                height: y,
                data_type,
                data: thumbnail::Data::Offset(thumbnail::DataOffset {
                    offset: offset as u64,
                    len: byte_count as u64,
                }),
            };
            thumbnails.insert(dim, desc);
        }
    }
}
