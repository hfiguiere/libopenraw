// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - tiff/container.rs
 *
 * Copyright (C) 2022-2025 Hubert Figuière
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

//! The IFD Container. Contains the IFD `Dir`

use std::cell::{RefCell, RefMut};
use std::collections::HashMap;
use std::io::{Read, Seek, SeekFrom};

use byteorder::{BigEndian, LittleEndian, ReadBytesExt};
use log::error;
use once_cell::unsync::OnceCell;

use crate::container;
use crate::container::{Endian, RawContainer};
use crate::decompress;
use crate::io;
use crate::io::View;
use crate::jpeg;
use crate::metadata;
use crate::thumbnail;
use crate::tiff::exif::TagMap;
use crate::tiff::{exif, Compression, Dir, Entry, Ifd, IfdType};
use crate::Type as RawType;
use crate::{DataType, Dump, Error, RawImage, Result, Type};

pub(crate) type DirIterator<'a> = std::slice::Iter<'a, Dir>;

pub(crate) type DirMap = Vec<(IfdType, Option<&'static TagMap>)>;

pub(crate) trait LoaderFixup {
    /// Check for the magic header.
    fn check_magic_header(&self, buf: &[u8]) -> Result<container::Endian> {
        Container::is_magic_header(buf)
    }

    /// Determine if a subifd shall be parsed. Returns true by default.
    /// This is useful to skip broken IFD like Sony's where they decide
    /// to "encrypt" thing as a user hostile move.
    fn parse_subifd(&self, _container: &Container) -> bool {
        true
    }
}

/// IFD Container for TIFF based file.
pub(crate) struct Container {
    /// The `io::View`.
    view: RefCell<View>,
    /// Endian of the container.
    endian: RefCell<container::Endian>,
    /// IFD.
    dirs: OnceCell<Vec<Dir>>,
    /// index to `Type` and `TagMap` map
    dir_map: DirMap,
    /// The Exif IFD
    exif_ifd: OnceCell<Option<Dir>>,
    /// The MakerNote IFD
    mnote_ifd: OnceCell<Option<Dir>>,
    /// The DNG Private IFD
    dng_private_ifd: OnceCell<Option<Dir>>,
    raw_type: RawType,
    /// Loader fixup for quirks (ie almost TIFF)
    pub(crate) loader_fixup: Option<Box<dyn LoaderFixup>>,
}

impl std::fmt::Debug for Container {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("Container")
            .field("view", &self.view)
            .field("endian", &self.endian)
            .field("dirs", &self.dirs)
            .field("dir_map", &self.dir_map)
            .field("exif_ifd", &self.exif_ifd)
            .field("mnote_ifd", &self.mnote_ifd)
            .field("dng_private_ifd", &self.dng_private_ifd)
            .field("raw_type", &self.raw_type)
            .field(
                "loader_fixup",
                if self.loader_fixup.is_some() {
                    &"Some"
                } else {
                    &"None"
                },
            )
            .finish()
    }
}
fn ifd_type_to_dirid(t: IfdType) -> Option<&'static str> {
    match t {
        IfdType::Raw => Some("Raw"),
        IfdType::Main => Some("Main"),
        IfdType::Exif => Some("Exif"),
        IfdType::GpsInfo => Some("GPSInfo"),
        IfdType::SubIfd => Some("SubIfd"),
        IfdType::Other => Some("Other"),
        // XXX figure out the best way here, we shouldn't reach this.
        IfdType::MakerNote => None,
    }
}

impl container::RawContainer for Container {
    fn endian(&self) -> container::Endian {
        *self.endian.borrow()
    }

    fn borrow_view_mut(&self) -> RefMut<'_, View> {
        self.view.borrow_mut()
    }

    fn raw_type(&self) -> RawType {
        self.raw_type
    }

    /// Return an dir metadata iterator.
    fn dir_iterator(&self) -> metadata::Iterator {
        self.dirs().iter().into()
    }
}

impl Container {
    /// Create a new container for the view.
    pub(crate) fn new(view: View, dir_map: DirMap, raw_type: RawType) -> Self {
        Self {
            view: RefCell::new(view),
            endian: RefCell::new(container::Endian::Unset),
            dirs: OnceCell::new(),
            dir_map,
            exif_ifd: OnceCell::new(),
            mnote_ifd: OnceCell::new(),
            dng_private_ifd: OnceCell::new(),
            raw_type,
            loader_fixup: None,
        }
    }

    pub fn read_u16_array(
        &self,
        view: &mut View,
        array: &mut [u16],
        count: usize,
    ) -> std::io::Result<usize> {
        assert!(array.len() >= count);
        match *self.endian.borrow() {
            container::Endian::Little => {
                for item in array.iter_mut().take(count) {
                    *item = view.read_u16::<LittleEndian>()?
                }
                Ok(count)
            }
            container::Endian::Big => {
                for item in array.iter_mut().take(count) {
                    *item = view.read_u16::<BigEndian>()?
                }
                Ok(count)
            }
            container::Endian::Unset => {
                unreachable!("endian unset");
            }
        }
    }

    pub(crate) fn load(&mut self, loader_fixup: Option<Box<dyn LoaderFixup>>) -> Result<()> {
        self.loader_fixup = loader_fixup;
        let mut view = self.view.borrow_mut();
        view.seek(SeekFrom::Start(0))?;
        let mut buf = [0_u8; 4];
        view.read_exact(&mut buf)?;
        if let Some(loader_fixup) = &self.loader_fixup {
            self.endian.replace(loader_fixup.check_magic_header(&buf)?);
        } else {
            self.endian.replace(Self::is_magic_header(&buf)?);
        }

        Ok(())
    }

    /// Read the dir at the offset
    pub(crate) fn dir_at(
        &self,
        view: &mut View,
        offset: u32,
        base_offset: u32,
        t: IfdType,
        id: Option<&'static str>,
        tag_names: Option<&'static HashMap<u16, &'static str>>,
    ) -> Result<Dir> {
        let mut dir = match *self.endian.borrow() {
            container::Endian::Little => Dir::read::<LittleEndian>(view, offset, base_offset, t),
            container::Endian::Big => Dir::read::<BigEndian>(view, offset, base_offset, t),
            _ => {
                error!("Endian unset to read directory");
                Err(Error::NotFound)
            }
        };
        if let Some(id) = id {
            dir = dir.map(|mut dir| {
                dir.id = id.bytes().chain(std::iter::once(0_u8)).collect();
                dir
            });
        }
        if let Some(tag_names) = tag_names {
            dir.map(|mut dir| {
                dir.tag_names = tag_names;
                dir
            })
        } else {
            dir
        }
    }

    /// Get the directories. They get loaded once as needed.
    pub(crate) fn dirs(&self) -> &Vec<Dir> {
        self.dirs.get_or_init(|| {
            let mut dirs = vec![];

            let mut index = 0_usize;
            let mut dir_offset = {
                let mut view = self.view.borrow_mut();
                view.seek(SeekFrom::Start(4)).expect("Seek failed");
                view.read_endian_u32(self.endian()).unwrap_or(0)
            };
            while dir_offset != 0 {
                let t = if index < self.dir_map.len() {
                    self.dir_map[index]
                } else {
                    (IfdType::Other, None)
                };
                if let Ok(dir) = if t.0 == IfdType::MakerNote {
                    Dir::create_maker_note(self, dir_offset)
                } else {
                    self.dir_at(
                        &mut self.view.borrow_mut(),
                        dir_offset,
                        0,
                        t.0,
                        ifd_type_to_dirid(t.0),
                        t.1,
                    )
                } {
                    let next_offset = dir.next_ifd();
                    dirs.push(dir);
                    index += 1;
                    if next_offset != 0 && next_offset <= dir_offset {
                        error!("Trying to read dirs backwards from {dir_offset} to {next_offset}");
                        // We should be ok to deal with this. ARW for
                        // DSLR-A550 and NEX-3 do that.
                    }
                    dir_offset = next_offset;
                } else {
                    error!("Couldn't read directory");
                    break;
                }
            }

            dirs
        })
    }

    /// Get the indexed `tiff::Dir` from the container
    pub fn directory(&self, idx: usize) -> Option<&Dir> {
        let dirs = self.dirs();
        if dirs.len() <= idx {
            return None;
        }

        Some(&dirs[idx])
    }

    /// Will identify the magic header and return the endian
    fn is_magic_header(buf: &[u8]) -> Result<container::Endian> {
        if buf.len() < 4 {
            error!("IFD magic header buffer too small: {} bytes", buf.len());
            return Err(Error::BufferTooSmall);
        }

        if &buf[0..4] == b"II\x2a\x00" {
            Ok(container::Endian::Little)
        } else if &buf[0..4] == b"MM\x00\x2a" {
            Ok(container::Endian::Big)
        } else {
            error!("Incorrect IFD magic: {:?}", buf);
            Err(Error::FormatError)
        }
    }

    /// Lazily load the Exif dir and return it.
    pub(crate) fn exif_dir(&self) -> Option<&Dir> {
        self.exif_ifd
            .get_or_init(|| {
                self.directory(0)
                    .and_then(|dir| dir.get_exif_ifd(self))
                    .or_else(|| {
                        log::warn!("Coudln't find Exif IFD");
                        None
                    })
            })
            .as_ref()
    }

    /// Lazily load the MakerNote and return it.
    pub(crate) fn mnote_dir(&self) -> Option<&Dir> {
        self.mnote_ifd
            .get_or_init(|| {
                log::debug!("Loading MakerNote");
                self.exif_dir()
                    .and_then(|dir| dir.get_mnote_ifd(self))
                    .or_else(|| {
                        log::warn!("Couldn't find MakerNote");
                        None
                    })
            })
            .as_ref()
    }

    /// Lazily load the MakerNote and return it.
    pub(crate) fn dng_private_dir(&self) -> Option<&Dir> {
        self.dng_private_ifd
            .get_or_init(|| {
                log::debug!("Loading MakerNote (DNG Private)");
                self.directory(0)?
                    .mnote_ifd_for_tag(self, exif::DNG_TAG_DNG_PRIVATE)
            })
            .as_ref()
    }

    /// Find the Raw IFD
    pub(crate) fn locate_raw_ifd(&self) -> Option<&Dir> {
        let dir = self.directory(0)?;

        if dir.is_primary() {
            log::debug!("dir0 is primary");
            return Some(dir);
        }
        dir.get_sub_ifds(self).iter().find(|d| d.is_primary())
    }

    /// Get the raw data
    pub(crate) fn rawdata(&self, dir: &Dir, file_type: Type) -> Result<RawImage> {
        self.rawdata_with_endian(dir, file_type, self.endian())
    }

    /// Get the raw data with a specific endian for the data.
    pub(crate) fn rawdata_with_endian(
        &self,
        dir: &Dir,
        file_type: Type,
        endian: Endian,
    ) -> Result<RawImage> {
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
        // Check for excessively large byte_len: pixel_count * 2 should be
        // a proper limit. But we can't make this an error. iPhone 15 Pro
        // files trigger it. The actual file length will catch excessive
        // length.
        if byte_len > pixel_count * 2 {
            log::warn!(
                "TIFF: Raw byte length {byte_len} too large for pixel count {pixel_count} * 2."
            );
        }
        if byte_len as u64 > self.borrow_view_mut().len() {
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

        let linearization_table = dir
            .entry(exif::DNG_TAG_LINEARIZATION_TABLE)
            .and_then(|entry| entry.value_array::<u16>(dir.endian()));
        let data_type = match compression {
            Compression::None | Compression::NikonPack | Compression::PentaxPack => DataType::Raw,
            _ => DataType::CompressedRaw,
        };
        // Here a D100 would have compression = NikonQuantized.
        // But it'll trickle down the Nikon code.

        // Get the mosaic info
        let mosaic_pattern = super::get_mosaic_info(dir).or_else(|| {
            log::debug!("Trying mosaic data in Exif");
            self.exif_dir()
                .as_ref()
                .and_then(|exif| super::get_mosaic_info(exif))
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
                        if *offset as u64 > self.borrow_view_mut().len() {
                            log::error!("TIFF: trying to load tile past EOF");
                            vec![]
                        } else if *byte_len as u64 + *offset as u64 > self.borrow_view_mut().len() {
                            log::error!("TIFF: byte length for tile exceed file size");
                            vec![]
                        } else {
                            self.load_buffer8(*offset as u64, *byte_len as u64)
                        }
                    })
                    .collect();
                RawImage::new_tiled(
                    x,
                    y,
                    actual_bpc,
                    data_type,
                    data,
                    tile_size.unwrap(),
                    mosaic_pattern.unwrap_or_default(),
                )
            } else {
                let data = self.load_buffer8(offset as u64, byte_len as u64);
                RawImage::with_data8(
                    x,
                    y,
                    actual_bpc,
                    data_type,
                    data,
                    mosaic_pattern.unwrap_or_default(),
                )
            }
        } else if bpc == 16 {
            let data = if endian == Endian::Big {
                self.load_buffer16_be(offset as u64, byte_len as u64)
            } else {
                self.load_buffer16_le(offset as u64, byte_len as u64)
            };
            RawImage::with_data16(
                x,
                y,
                actual_bpc,
                data_type,
                data,
                mosaic_pattern.unwrap_or_default(),
            )
        } else if bpc == 10 || bpc == 12 || bpc == 14 {
            let data = decompress::unpack(
                self,
                x,
                y,
                bpc,
                compression,
                offset as u64,
                byte_len as usize,
            )?;
            RawImage::with_data16(
                x,
                y,
                actual_bpc,
                data_type,
                data,
                mosaic_pattern.unwrap_or_default(),
            )
        } else if bpc == 8 {
            let data = self.load_buffer8(offset as u64, byte_len as u64);
            // XXX is this efficient?
            RawImage::with_data16(
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

        rawdata.set_linearization_table(linearization_table);
        // XXX maybe we don't need the if
        rawdata.set_compression(if data_type == DataType::CompressedRaw {
            compression
        } else {
            Compression::None
        });
        rawdata.set_photometric_interpretation(photom_int);
        if rawdata.whites()[0] == 0 {
            let white: u32 = (1_u32 << actual_bpc) - 1;
            rawdata.set_whites([white as u16; 4]);
        }

        Ok(rawdata)
    }

    /// Get the thumbnails out of a TIFF
    pub(crate) fn thumbnails(&self) -> Vec<(u32, thumbnail::ThumbDesc)> {
        let mut thumbnails = Vec::new();

        let dirs = self.dirs();
        for dir in dirs {
            if dir.ifd_type() == IfdType::Raw {
                continue;
            }
            dir.locate_thumbnail(self, &mut thumbnails);

            let subdirs = dir.get_sub_ifds(self);
            for subdir in subdirs {
                subdir.locate_thumbnail(self, &mut thumbnails);
            }
        }

        log::debug!("Found {} thumbnails", thumbnails.len());

        thumbnails
    }

    /// Add the thumbnail from data in the container
    pub(crate) fn add_thumbnail_from_stream(
        &self,
        offset: u32,
        len: u32,
        list: &mut Vec<(u32, thumbnail::ThumbDesc)>,
    ) -> Result<usize> {
        let view = io::Viewer::create_subview(&self.borrow_view_mut(), offset as u64)?;
        let jpeg = jpeg::Container::new(view, self.raw_type);
        let width = jpeg.width() as u32;
        let height = jpeg.height() as u32;
        let dim = std::cmp::max(width, height);
        // "Olympus" MakerNote carries a 160 px thubnail we might already have.
        // We don't check it is the same.
        if !list.iter().any(|t| t.0 == dim) {
            use crate::thumbnail::{Data, DataOffset};

            list.push((
                dim,
                thumbnail::ThumbDesc {
                    width,
                    height,
                    data_type: DataType::Jpeg,
                    data: Data::Offset(DataOffset {
                        offset: offset as u64,
                        len: len as u64,
                    }),
                },
            ));
            Ok(1)
        } else {
            Ok(0)
        }
    }

    /// Add a thumbnail from the entry
    pub(crate) fn add_thumbnail_from_entry(
        &self,
        e: &Entry,
        offset: u32,
        list: &mut Vec<(u32, thumbnail::ThumbDesc)>,
    ) -> Result<usize> {
        if let Some(val_offset) = e.offset() {
            let val_offset = val_offset + offset;
            self.add_thumbnail_from_stream(val_offset, e.count, list)
        } else {
            log::error!("Entry for thumbnail has no offset");
            Err(Error::NotFound)
        }
    }
}

impl Dump for Container {
    #[cfg(feature = "dump")]
    fn write_dump<W>(&self, out: &mut W, indent: u32)
    where
        W: std::io::Write + ?Sized,
    {
        let dirs = self.dirs();
        dump_writeln!(
            out,
            indent,
            "<TIFF Container endian={} {} directories @{}>",
            match self.endian() {
                container::Endian::Little => "II",
                container::Endian::Big => "MM",
                _ => "Unknown",
            },
            dirs.len(),
            self.view.borrow().offset()
        );
        {
            let indent = indent + 1;
            for dir in dirs {
                dir.write_dump(out, indent);
                if let Some(exif_dir) = dir.get_exif_ifd(self) {
                    exif_dir.write_dump(out, indent + 1);
                    if let Some(mnote_dir) = exif_dir.get_mnote_ifd(self) {
                        mnote_dir.write_dump(out, indent + 2);
                    }
                }
                if let Some(dng_private_data) = dir.get_dng_private_ifd(self) {
                    dng_private_data.write_dump(out, indent + 1);
                }
                if let Some(gps_dir) = dir.get_gpsinfo_ifd(self) {
                    gps_dir.write_dump(out, indent + 1);
                }
                let subdirs = dir.get_sub_ifds(self);
                for subdir in subdirs {
                    subdir.write_dump(out, indent + 1);
                }
            }
        }
        dump_writeln!(out, indent, "</TIFF Container>");
    }
}
