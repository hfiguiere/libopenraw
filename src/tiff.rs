// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - tiff.rs
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

//! TIFF format (Image File Directories)

mod container;
mod dir;
mod entry;
pub mod exif;
mod iterator;

use std::convert::TryFrom;

use byteorder::{BigEndian, LittleEndian};

use crate::container::{Endian, RawContainer};
use crate::decompress;
use crate::io;
use crate::jpeg;
use crate::mosaic::Pattern;
use crate::thumbnail;
use crate::{DataType, Error, RawData, Result, Type, TypeId};
pub(crate) use container::{Container, DirIterator};
pub(crate) use dir::Dir;
pub(crate) use entry::Entry;
pub(crate) use exif::TagType;
pub(crate) use iterator::Iterator;

#[repr(u32)]
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
    /// Deflate (ZIP)
    Deflate = 8,
    /// Sony ARW compression
    Arw = 32767,
    /// Nikon packed, also used by Epson ERF.
    NikonPack = 32769,
    /// Pentax packed (12be)
    PentaxPack = 32773,
    /// Nikon quantized
    NikonQuantized = 34713,
    /// DNG Lossy JPEG
    DngLossy = 34892,
    /// What everybody seems to use
    Custom = 65535,
    // XXX figure out Olympus compression value
    // Olympus compression
    Olympus = 65536,
}

impl From<u32> for Compression {
    /// 0 and any unknown value will yield `Unknown`
    fn from(v: u32) -> Compression {
        use Compression::*;

        match v {
            1 => None,
            6 => Jpeg,
            7 => LJpeg,
            32767 => Arw,
            32769 => NikonPack,
            32773 => PentaxPack,
            34713 => NikonQuantized,
            65535 => Custom,
            65536 => Olympus,
            _ => Unknown,
        }
    }
}

/// Type of IFD
#[derive(Clone, Copy, Debug, PartialEq)]
pub enum IfdType {
    /// Main IFD (see TIFF)
    Main,
    /// Raw specific IFD
    Raw,
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
    fn ifd_type(&self) -> IfdType;

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
        dir.entry(exif::EXIF_TAG_MODEL)
            // Files like Black Magic's DNG don't have a Model.
            .or_else(|| dir.entry(exif::DNG_TAG_UNIQUE_CAMERA_MODEL))
            .and_then(|e| e.string_value().and_then(|s| map.get(s.as_str()).copied()))
    })
}

/// Find the Raw IFD
pub(crate) fn tiff_locate_raw_ifd(container: &Container) -> Option<&Dir> {
    let dir = container.directory(0)?;

    if dir.is_primary() {
        log::debug!("dir0 is primary");
        return Some(dir);
    }
    dir.get_sub_ifds(container).iter().find(|d| d.is_primary())
}

fn convert_cfa_pattern(dir: &Dir, entry: &Entry) -> Option<Pattern> {
    let a = entry.value_array::<u8>(dir.endian())?;
    Pattern::try_from(a.as_slice()).ok()
}

fn convert_new_cfa_pattern(container: &Container, dir: &Dir, entry: &Entry) -> Option<Pattern> {
    if entry.count < 4 {
        return None;
    }

    let mut view = container.borrow_view_mut();
    let result = match dir.endian() {
        Endian::Big => entry.get_data::<BigEndian>(0, &mut view),
        Endian::Little => entry.get_data::<LittleEndian>(0, &mut view),
        _ => unreachable!(),
    };
    let data = result.ok()?;

    let h = data[0];
    let v = data[1];
    if h != 2 && v != 2 {
        log::error!("Invalid CFA pattern size {}x{}", h, v);
        return None;
    }

    Pattern::try_from(&data[4..=7]).ok()
}

fn get_mosaic_info(container: &Container, dir: &Dir) -> Option<Pattern> {
    dir.entry(exif::EXIF_TAG_CFA_PATTERN)
        .and_then(|e| convert_cfa_pattern(dir, e))
        .or_else(|| {
            dir.entry(exif::EXIF_TAG_NEW_CFA_PATTERN)
                .and_then(|e| convert_new_cfa_pattern(container, dir, e))
        })
}

/// Get the raw data
pub(crate) fn tiff_get_rawdata(
    container: &Container,
    dir: &Dir,
    file_type: Type,
) -> Result<RawData> {
    let mut offset = 0_u32;

    let mut bpc = dir
        .value::<u16>(exif::EXIF_TAG_BITS_PER_SAMPLE)
        .or_else(|| {
            log::error!("Unable to get bits per sample");
            None
        })
        .unwrap_or(0);

    let mut tile_bytes: Option<Vec<u32>> = None;
    let mut tile_offsets: Option<Vec<u32>> = None;
    let mut tile_size: Option<(u32, u32)> = None;
    let byte_len = dir
        .value::<u32>(exif::EXIF_TAG_STRIP_OFFSETS)
        .and_then(|v| {
            offset = v;
            let entry = dir.entry(exif::EXIF_TAG_STRIP_BYTE_COUNTS).or_else(|| {
                log::debug!("byte len not found");
                // XXX this might trigger the or_else below
                None
            })?;
            // In case of overflow, we'll return None
            entry
                .value_array::<u32>(dir.endian())
                .map(|a| a.iter().try_fold(0_u32, |sum, &x| sum.checked_add(x)))?
        })
        .or_else(|| {
            tile_bytes = dir
                .entry(exif::TIFF_TAG_TILE_BYTECOUNTS)
                .and_then(|e| e.value_array::<u32>(dir.endian()));
            // In case of overflow, we'll return 0 via the unwrap().
            let tile_bytes_total = tile_bytes
                .as_ref()
                .and_then(|a| a.iter().try_fold(0_u32, |sum, &x| sum.checked_add(x)))
                .unwrap_or(0);
            tile_offsets = dir
                .entry(exif::TIFF_TAG_TILE_OFFSETS)
                .and_then(|e| e.value_array::<u32>(dir.endian()));
            // the tile are individual JPEGS....
            let x = dir.uint_value(exif::TIFF_TAG_TILE_WIDTH).unwrap_or(0);
            let y = dir.uint_value(exif::TIFF_TAG_TILE_LENGTH).unwrap_or(0);
            tile_size = Some((x, y));
            Some(tile_bytes_total)
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
    // This will fail if the multiply overflow.
    let pixel_count = x
        .checked_mul(y)
        // if v overflow when muliplied my 2, then it's too big.
        .and_then(|v| if v > u32::MAX / 2 { None } else { Some(v) })
        .ok_or(Error::FormatError)?;
    // Check for excessively large byte_len
    // pixel_count * 2 should be a proper limit.
    if byte_len > pixel_count * 2 {
        log::error!("TIFF: Raw byte length too large for pixel count.");
        return Err(Error::FormatError);
    }
    if byte_len as u64 > container.borrow_view_mut().len() {
        log::error!("TIFF: Raw byte length too large for file size.");
        return Err(Error::FormatError);
    }
    let photom_int = dir
        .uint_value(exif::EXIF_TAG_PHOTOMETRIC_INTERPRETATION)
        .and_then(|v| exif::PhotometricInterpretation::try_from(v).ok())
        .unwrap_or(exif::PhotometricInterpretation::CFA);

    let compression = dir
        .uint_value(exif::EXIF_TAG_COMPRESSION)
        .map(Compression::from)
        .map(|compression| {
            if file_type == Type::Orf && byte_len < pixel_count.checked_mul(2).unwrap_or(0) {
                log::debug!("ORF, setting bpc to 12 and data to compressed.");
                bpc = 12;
                Compression::Olympus
            } else {
                compression
            }
        })
        .unwrap_or(Compression::None);

    let data_type = match compression {
        Compression::None | Compression::NikonPack | Compression::PentaxPack => DataType::Raw,
        _ => DataType::CompressedRaw,
    };
    // Here a D100 would have compression = NikonQuantized.
    // But it'll trickle down the Nikon code.

    // Get the mosaic info
    let mosaic_pattern = get_mosaic_info(container, dir).or_else(|| {
        log::debug!("Trying mosaic data in Exif");
        container
            .exif_dir()
            .as_ref()
            .and_then(|exif| get_mosaic_info(container, exif))
    });
    log::debug!("mosaic_pattern {:?}", mosaic_pattern);

    // More that 32bits per component is invalid: corrupt file likely.
    if bpc > 32 {
        log::error!("TIFF: bpc {} is invalid", bpc);
        return Err(Error::FormatError);
    }
    let actual_bpc = bpc;
    if (bpc == 12 || bpc == 14)
        && (compression == Compression::None)
        && byte_len == (pixel_count * 2)
    {
        // it's 12 or 14 bpc, but we have 16 bpc data.
        log::debug!("setting bpc from {} to 16", bpc);
        bpc = 16;
    }
    let mut rawdata = if data_type == DataType::CompressedRaw {
        if tile_bytes.is_some() && tile_offsets.is_some() {
            let tile_bytes = tile_bytes.as_ref().unwrap();
            let tile_offsets = tile_offsets.as_ref().unwrap();
            let data = std::iter::zip(tile_offsets, tile_bytes)
                .map(|(offset, byte_len)| {
                    // If we exceed file boundaries, return empty buffers
                    if *offset as u64 > container.borrow_view_mut().len() {
                        log::error!("TIFF: trying to load tile past EOF");
                        vec![]
                    } else if *byte_len as u64 + *offset as u64 > container.borrow_view_mut().len()
                    {
                        log::error!("TIFF: byte length for tile exceed file size");
                        vec![]
                    } else {
                        container.load_buffer8(*offset as u64, *byte_len as u64)
                    }
                })
                .collect();
            RawData::new_tiled(
                x,
                y,
                actual_bpc,
                data_type,
                data,
                tile_size.unwrap(),
                mosaic_pattern.unwrap_or_default(),
            )
        } else {
            let data = container.load_buffer8(offset as u64, byte_len as u64);
            RawData::new8(
                x,
                y,
                actual_bpc,
                data_type,
                data,
                mosaic_pattern.unwrap_or_default(),
            )
        }
    } else if bpc == 16 {
        let data = if container.endian() == Endian::Big {
            container.load_buffer16_be(offset as u64, byte_len as u64)
        } else {
            container.load_buffer16_le(offset as u64, byte_len as u64)
        };
        RawData::new16(
            x,
            y,
            actual_bpc,
            data_type,
            data,
            mosaic_pattern.unwrap_or_default(),
        )
    } else if bpc == 10 || bpc == 12 || bpc == 14 {
        let data = decompress::unpack(
            container,
            x,
            y,
            bpc,
            compression,
            offset as u64,
            byte_len as usize,
        )?;
        RawData::new16(
            x,
            y,
            actual_bpc,
            data_type,
            data,
            mosaic_pattern.unwrap_or_default(),
        )
    } else if bpc == 8 {
        let data = container.load_buffer8(offset as u64, byte_len as u64);
        // XXX is this efficient?
        RawData::new16(
            x,
            y,
            bpc,
            data_type,
            data.iter().map(|v| *v as u16).collect(),
            mosaic_pattern.unwrap_or_default(),
        )
    } else {
        log::error!("Invalid RAW format, unsupported bpc {}", bpc);
        return Err(Error::InvalidFormat);
    };

    // XXX maybe we don't need the if
    rawdata.set_compression(if data_type == DataType::CompressedRaw {
        compression
    } else {
        Compression::None
    });
    rawdata.set_photometric_interpretation(photom_int);
    if rawdata.white() == 0 {
        let white: u32 = (1_u32 << actual_bpc) - 1;
        rawdata.set_white(white as u16);
    }

    Ok(rawdata)
}

/// Get the thumbnails out of a TIFF
pub(crate) fn tiff_thumbnails(container: &Container) -> Vec<(u32, thumbnail::ThumbDesc)> {
    let mut thumbnails = Vec::new();

    let dirs = container.dirs();
    for dir in dirs {
        if dir.ifd_type() == IfdType::Raw {
            continue;
        }
        ifd_locate_thumbnail(container, dir, &mut thumbnails);

        let subdirs = dir.get_sub_ifds(container);
        for subdir in subdirs {
            ifd_locate_thumbnail(container, subdir, &mut thumbnails);
        }
    }

    log::debug!("Found {} thumbnails", thumbnails.len());

    thumbnails
}

pub(crate) fn ifd_locate_thumbnail(
    container: &dyn RawContainer,
    dir: &Dir,
    thumbnails: &mut Vec<(u32, thumbnail::ThumbDesc)>,
) {
    let mut data_type = DataType::Unknown;
    let subtype = if let Some(subtype) = dir.value::<u32>(exif::EXIF_TAG_NEW_SUBFILE_TYPE) {
        subtype
    } else {
        // XXX check if we are in the Raw IFD. We don't know this here.
        1
    };
    if subtype == 1 {
        let photom_int = dir
            .value::<u16>(exif::EXIF_TAG_PHOTOMETRIC_INTERPRETATION)
            .unwrap_or(exif::PhotometricInterpretation::Rgb as u16);
        let mut x = dir.uint_value(exif::EXIF_TAG_IMAGE_WIDTH).unwrap_or(0);
        let mut y = dir.uint_value(exif::EXIF_TAG_IMAGE_LENGTH).unwrap_or(0);
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
                if x != 0 && y != 0 {
                    let pixel_count = x.checked_mul(y).and_then(|count| count.checked_mul(3));
                    if pixel_count.is_none() || byte_count >= pixel_count.unwrap() {
                        // We ignore this, it's 8RGB in a Canon CR2 file.
                        // See bug 72270
                        log::debug!("8RGB as JPEG or invalid pixel count. Will ignore.");
                        data_type = DataType::Unknown
                    } else {
                        data_type = DataType::Jpeg;
                    }
                } else {
                    data_type = DataType::Jpeg;
                    if x == 0 || y == 0 {
                        if let Ok(view) =
                            io::Viewer::create_subview(&container.borrow_view_mut(), offset as u64)
                        {
                            let jpeg = jpeg::Container::new(view, container.raw_type());
                            x = jpeg.width() as u32;
                            y = jpeg.height() as u32;
                            log::debug!("Found JPEG dimensions x={} y={}", x, y);
                        } else {
                            // XXX load the JFIF stream and get the dimensions.
                            log::error!("Couldn't get JPEG dimensions.");
                        }
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
            if dim > 0 {
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
                thumbnails.push((dim, desc));
            }
        }
    }
}
