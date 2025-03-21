// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - minolta.rs
 *
 * Copyright (C) 2023-2025 Hubert Figuière
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

//! Minolta MRW format
//!
//! This is also needed for the Sony file support for very early
//! post Minolta acquisition cameras like the Sony A100.

use std::cell::{RefCell, RefMut};
use std::collections::HashMap;
use std::io::{Read, Seek, SeekFrom};
use std::rc::Rc;

use byteorder::{BigEndian, LittleEndian, ReadBytesExt};
use once_cell::unsync::OnceCell;

use crate::colour::BuiltinMatrix;
use crate::container::{Endian, RawContainer};
use crate::decompress;
use crate::io::{View, Viewer};
use crate::metadata;
use crate::mosaic::Pattern;
use crate::rawfile::{RawFileHandleType, ThumbnailStorage};
use crate::thumbnail;
use crate::tiff::{self, exif, Ifd, IfdType};
use crate::{
    Context, DataType, Dump, Error, RawFile, RawFileHandle, RawFileImpl, RawImage, Result, Type,
    TypeId,
};

macro_rules! minolta {
    ($id:expr, $model:ident) => {
        (
            $id,
            TypeId(
                $crate::camera_ids::vendor::MINOLTA,
                $crate::camera_ids::minolta::$model,
            ),
        )
    };
    ($model:ident) => {
        TypeId(
            $crate::camera_ids::vendor::MINOLTA,
            $crate::camera_ids::minolta::$model,
        )
    };
}

pub use tiff::exif::generated::MNOTE_MINOLTA_TAG_NAMES as MNOTE_TAG_NAMES;

lazy_static::lazy_static! {
    pub(super) static ref MATRICES: [BuiltinMatrix; 9] = [
        BuiltinMatrix::new(
            minolta!(MAXXUM_5D),
            0, 0xffb,
            [ 10284, -3283, -1086, -7957, 15762, 2316, -829, 882, 6644 ]
        ),
        BuiltinMatrix::new(
            minolta!(MAXXUM_7D),
            0, 0xffb,
            [ 10239, -3104, -1099, -8037, 15727, 2451, -927, 925, 6871 ]
        ),
        BuiltinMatrix::new(
            minolta!(DIMAGE5),
            0, 0xf7d,
            [ 8983, -2942, -963, -6556, 14476, 2237, -2426, 2887, 8014 ]
        ),
        BuiltinMatrix::new(
            minolta!(DIMAGE7),
            0, 0xf7d,
            [ 9144, -2777, -998, -6676, 14556, 2281, -2470, 3019, 7744 ]
        ),
        BuiltinMatrix::new(
            minolta!(DIMAGE7I),
            0, 0xf7d,
            [ 9144, -2777, -998, -6676, 14556, 2281, -2470, 3019, 7744 ]
        ),
        BuiltinMatrix::new(
            minolta!(DIMAGE7HI),
            0, 0xf7d,
            [ 11368, -3894, -1242, -6521, 14358, 2339, -2475, 3056, 7285 ]
        ),
        BuiltinMatrix::new(
            minolta!(A1),
            0, 0xf8b,
            [ 9274, -2547, -1167, -8220, 16323, 1943, -2273, 2720, 8340 ]
        ),
        BuiltinMatrix::new(
            minolta!(A2),
            0, 0xf8f,
            [ 9097, -2726, -1053, -8073, 15506, 2762, -966, 981, 7763 ]
        ),
        BuiltinMatrix::new(
            minolta!(A200),
            0, 0,
            [ 8560, -2487, -986, -8112, 15535, 2771, -1209, 1324, 7743 ]
        ),
    ];

    static ref MODEL_ID_MAP: HashMap<&'static str, TypeId> = HashMap::from([
        minolta!("21860002", MAXXUM_5D),
        minolta!("21810002", MAXXUM_7D),
        minolta!("27730001", DIMAGE5),
        minolta!("27660001", DIMAGE7),
        minolta!("27790001", DIMAGE7I),
        minolta!("27780001", DIMAGE7HI),
        minolta!("27820001", A1),
        minolta!("27200001", A2),
        minolta!("27470002", A200),
    ]);
}

#[derive(Debug)]
/// The MRW file format was produced by Minolta cameras until
/// Konica-Minolta was purchased by Sony. This will not change.
///
/// Sources <http://www.dalibor.cz/software/minolta-raw-mrw-file-format>
pub(crate) struct MrwFile {
    reader: Rc<Viewer>,
    container: OnceCell<Box<MrwContainer>>,
    thumbnails: OnceCell<ThumbnailStorage>,
    #[cfg(feature = "probe")]
    probe: Option<crate::Probe>,
}

impl MrwFile {
    pub fn factory(reader: Rc<Viewer>) -> RawFileHandle {
        RawFileHandleType::new(MrwFile {
            reader,
            container: OnceCell::new(),
            thumbnails: OnceCell::new(),
            #[cfg(feature = "probe")]
            probe: None,
        })
    }
}

impl RawFileImpl for MrwFile {
    #[cfg(feature = "probe")]
    probe_imp!();

    fn identify_id(&self) -> Result<TypeId> {
        self.container()?;
        Ok(self
            .container
            .get()
            .and_then(|container| MODEL_ID_MAP.get(&container.version.as_str()))
            .unwrap_or(&minolta!(UNKNOWN)))
        .copied()
    }

    fn container(&self) -> Result<&dyn RawContainer> {
        self.container
            .get_or_try_init(|| {
                log::debug!("Creating mrw container");
                let view = Viewer::create_view(&self.reader, 0).context("Error creating view")?;
                let mut container = MrwContainer::new(view);
                container.load().context("Failed to load container")?;
                probe!(
                    self.probe,
                    "raw.container.endian",
                    &format!("{:?}", container.endian())
                );
                Ok(Box::new(container))
            })
            .map(|b| b.as_ref() as &dyn RawContainer)
    }

    /// MRW files have only one preview, 640x480, in the MakerNote.
    fn thumbnails(&self) -> Result<&ThumbnailStorage> {
        self.thumbnails.get_or_try_init(|| {
            let mut thumbnails = vec![];
            self.container()?;
            if let Some(makernote) = self.ifd(tiff::IfdType::MakerNote) {
                let ifd = self.container.get().unwrap().ifd_container();
                // Old files have the thumbnail in the Exif entry `MNOTE_MINOLTA_THUMBNAIL`.
                let buffer = if let Some(preview) = makernote.entry(exif::MNOTE_MINOLTA_THUMBNAIL) {
                    // e.data() is incorrect.
                    probe!(self.probe, "mrw.old_thumbnail", "true");
                    preview
                        .offset()
                        .map(|offset| ifd.load_buffer8(offset as u64, preview.count as u64))
                } else {
                    // Most file have offset and byte length in two tags.
                    let offset = makernote
                        .uint_value(exif::MNOTE_MINOLTA_THUMBNAIL_OFFSET)
                        .unwrap_or(0);
                    let length = makernote
                        .uint_value(exif::MNOTE_MINOLTA_THUMBNAIL_LENGTH)
                        .unwrap_or(0);
                    if offset != 0 && length != 0 {
                        Some(ifd.load_buffer8(offset as u64, length as u64))
                    } else {
                        None
                    }
                };
                if let Some(mut buffer) = buffer {
                    // For some reason the first byte isn't 0xff. Thanks Minolta.
                    // Setting the first byte to `0xff` makes it be a JPEG.
                    buffer[0] = 0xff;
                    thumbnails.push((
                        640,
                        thumbnail::ThumbDesc {
                            width: 640,
                            height: 480,
                            data_type: DataType::Jpeg,
                            data: thumbnail::Data::Bytes(buffer),
                        },
                    ));
                }
            }
            Ok(ThumbnailStorage::with_thumbnails(thumbnails))
        })
    }

    /// In a MRW file, all comes out of the IFD container from the `TTW`
    /// block.
    fn ifd(&self, ifd_type: tiff::IfdType) -> Option<&tiff::Dir> {
        self.container().ok()?;
        let ifd = self.container.get().unwrap().ifd_container();
        match ifd_type {
            tiff::IfdType::Raw | tiff::IfdType::Main => ifd.directory(0),
            tiff::IfdType::Exif => ifd.exif_dir(),
            tiff::IfdType::MakerNote => ifd.mnote_dir(),
            _ => None,
        }
    }

    /// The raw data is after all the blocks. The PRD give some info like
    /// the dimensions and the bits per sample.
    fn load_rawdata(&self, skip_decompress: bool) -> Result<RawImage> {
        self.container()?;
        if let Some(container) = self.container.get() {
            let rawinfo = container.minolta_prd()?;

            let cfa_offset = container.raw_data_offset();
            let cfa_len = if rawinfo.is_compressed {
                rawinfo.x * rawinfo.y + ((rawinfo.x * rawinfo.y) >> 1)
            } else {
                2 * rawinfo.x * rawinfo.y
            } as u64;
            let mut rawdata = if rawinfo.is_compressed {
                probe!(self.probe, "mrw.packed", "true");
                let raw = container.load_buffer8(cfa_offset, cfa_len);
                if skip_decompress {
                    RawImage::with_data8(
                        rawinfo.x,
                        rawinfo.y,
                        rawinfo.bps,
                        DataType::CompressedRaw,
                        raw,
                        rawinfo.mosaic,
                    )
                } else {
                    let mut unpacked = Vec::with_capacity((rawinfo.x * rawinfo.y) as usize);
                    decompress::unpack_be12to16(&raw, &mut unpacked, tiff::Compression::None)
                        .map_err(|err| {
                            log::error!("RAF failed to unpack {}", err);
                            err
                        })?;
                    RawImage::with_data16(
                        rawinfo.x,
                        rawinfo.y,
                        16,
                        DataType::Raw,
                        unpacked,
                        rawinfo.mosaic,
                    )
                }
            } else {
                let raw = container.load_buffer16_be(cfa_offset, cfa_len);
                RawImage::with_data16(
                    rawinfo.x,
                    rawinfo.y,
                    rawinfo.bps,
                    DataType::Raw,
                    raw,
                    rawinfo.mosaic,
                )
            };
            let id = self.type_id()?;
            if let Some((black, white)) = MATRICES
                .iter()
                .find(|m| m.camera == id)
                .map(|m| (m.black, m.white))
            {
                probe!(self.probe, "mrw.whites_blacks", "true");
                rawdata.set_whites([white; 4]);
                rawdata.set_blacks([black; 4]);
            }
            if let Some(wb) = container.get_wb() {
                probe!(self.probe, "mrw.wb", "true");
                rawdata.set_as_shot_neutral(&wb);
            }

            Ok(rawdata)
        } else {
            Err(Error::NotFound)
        }
    }

    fn get_builtin_colour_matrix(&self) -> Result<Vec<f64>> {
        self.builtin_colour_matrix(&*MATRICES)
    }
}

impl RawFile for MrwFile {
    fn type_(&self) -> Type {
        Type::Mrw
    }
}

impl Dump for MrwFile {
    #[cfg(feature = "dump")]
    fn write_dump<W: std::io::Write + ?Sized>(&self, out: &mut W, indent: u32) {
        dump_writeln!(out, indent, "<Minolta MRW File>");
        let _ = self.container();
        self.container.get().unwrap().write_dump(out, indent + 1);
        dump_writeln!(out, indent, "</Minolat MRW File>");
    }
}

dumpfile_impl!(MrwFile);

/// Known offsets in PRD block.
#[repr(u64)]
enum Prd {
    /// 8 chars, version string
    Version = 0,
    /// 2 bytes, Number of lines in raw data (height)
    SensorLength = 8,
    /// 2 bytes, Number of pixels per line (width)
    SensorWidth = 10,
    /// 2 bytes, length of image after Divu processing
    #[allow(unused)]
    ImageLength = 12,
    /// 2 bytes, width of image after Divu processing
    #[allow(unused)]
    ImageWidth = 14,
    /// 1 byte,  number of bits used to store each pixel.
    /// 16 mean 12 bit samples are in 16 bits while 12
    /// means that the samples are packed. Relected with
    /// `StorageType`.
    #[allow(unused)]
    DataSize = 16,
    /// 1 byte,  number of valid bits per pixel 12 bits.
    PixelSize = 17,
    /// 1 byte,  storage method
    /// Values are defined in `StorageType` enum.
    StorageType = 18,
    /// 1 byte
    #[allow(unused)]
    Unknown1 = 19,
    /// 2 bytes
    #[allow(unused)]
    Unknown2 = 20,
    /// 2 bytes, CFA pattern, See `BayerPattern` for the
    /// possible values.
    BayerPattern = 22,
}

/// Whether the raw data is unpacked or packed.
#[repr(u8)]
enum StorageType {
    /// Unpacked storage (D5, D7xx)
    #[allow(unused)]
    Unpacked = 0x52,
    /// Packed storage (A1, A2, Maxxum/Dynax)
    Packed = 0x59,
}

/// The CFA pattern.
#[repr(u16)]
enum BayerPattern {
    #[allow(unused)]
    Rggb = 0x0001,
    Gbrg = 0x0004, /* A200 */
}

/// Known offsets in WBG block.
/// The order is RGGB, or GBRG depending on the `BayerPattern`.
#[allow(unused)]
pub(crate) enum Wbg {
    DenominatorR = 0,  /* 1 byte,  log2(denominator)-6 */
    DenominatorG1 = 1, /* 1 byte,  To get actual denominator, 1<<(val+6) */
    DenominatorG2 = 2, /* 1 byte, */
    DenominatorB = 3,  /* 1 byte, */
    NominatorR = 4,    /* 2 bytes, */
    NominatorG1 = 6,   /* 2 bytes, */
    NominatorG2 = 8,   /* 2 bytes, */
    NominatorB = 10,   /* 2 bytes, */
}

/// Known offsets in RIF block.
#[allow(unused)]
enum Rif {
    Unknown1 = 0,       /* 1 byte,  */
    Saturation = 1,     /* 1 byte,  saturation setting from -3 to 3 */
    Contrast = 2,       /* 1 byte,  contrast setting from -3 to 3 */
    Sharpness = 3,      /* 1 byte,  sharpness setting from -1 (soft) to 1 (hard) */
    WhiteBalance = 4,   /* 1 byte,  white balance setting */
    SubjectProgram = 5, /* 1 byte,  subject program setting */
    FilmSpeed = 6,      /* 1 byte,  iso = 2^(value/8-1) * 3.125 */
    ColorMode = 7,      /* 1 byte,  color mode setting */
    ColorFilter = 56,   /* 1 byte,  color filter setting from -3 to 3 */
    BandwFilter = 57,   /* 1 byte,  black and white filter setting from 0 to 10 */
}

#[allow(unused)]
enum WhiteBalance {
    Auto = 0,
    Daylight = 1,
    Cloudy = 2,
    Tungsten = 3,
    Fluorescent = 4,
}

#[allow(unused)]
enum SubjectProgram {
    None = 0,
    Portrait = 1,
    Text = 2,
    NightPortrait = 3,
    Sunset = 4,
    SportsAction = 5,
}

#[allow(unused)]
enum ColorMode {
    Normal = 0,
    BlackAndWhite = 1,
    VividColor = 2,   /* D7i, D7Hi */
    Solarization = 3, /* D7i, D7Hi */
    AdobeRGB = 4,     /* D7Hi */
}

/// The extra length to add to the block length.
const DATA_BLOCK_HEADER_LENGTH: u64 = 8;

#[derive(Debug)]
/// Datablock of the MRW file.
struct DataBlock {
    /// Offset from the begining of the file.
    offset: u64,
    /// Name of the block. 3 letter code in a u32 big endian. Starts with `NUL`.
    name: [u8; 4],
    /// Length of the block.
    length: u64,
}

impl DataBlock {
    /// Load the datablock from the container.
    fn load(offset: u64, container: &MrwContainer) -> Result<DataBlock> {
        let mut name = [0_u8; 4];
        container.borrow_view_mut().read_exact(&mut name)?;
        let length = match container.endian() {
            Endian::Big => container.borrow_view_mut().read_u32::<BigEndian>()? as u64,
            Endian::Little => container.borrow_view_mut().read_u32::<LittleEndian>()? as u64,
            _ => unreachable!("Endian unset"),
        };
        Ok(DataBlock {
            offset,
            name,
            length,
        })
    }

    /// The block ID that is more like a BE u32, as a string.
    /// So we strip the first byte that is `\0`
    fn name(&self) -> &[u8] {
        &self.name[1..]
    }

    fn uint16_value(&self, offset: u64, container: &MrwContainer) -> Option<u16> {
        let mut view = container.borrow_view_mut();
        view.seek(SeekFrom::Start(
            offset + self.offset + DATA_BLOCK_HEADER_LENGTH,
        ))
        .ok()?;
        match container.endian() {
            Endian::Big => view.read_u16::<BigEndian>().ok(),
            Endian::Little => view.read_u16::<LittleEndian>().ok(),
            _ => unreachable!("Endian unset"),
        }
    }

    fn uint8_value(&self, offset: u64, container: &MrwContainer) -> Option<u8> {
        let mut view = container.borrow_view_mut();
        view.seek(SeekFrom::Start(
            offset + self.offset + DATA_BLOCK_HEADER_LENGTH,
        ))
        .ok()?;
        view.read_u8().ok()
    }
}

/// Raw info extracted from the PRD block.
pub(crate) struct RawInfo {
    pub x: u32,
    pub y: u32,
    pub bps: u16,
    pub is_compressed: bool,
    pub mosaic: Pattern,
}

#[derive(Debug)]
pub(crate) struct MrwContainer {
    endian: Endian,
    /// The `io::View`.
    view: RefCell<View>,
    /// Version (ie camera) of the file.
    version: String,
    mrm: Option<DataBlock>,
    /// Raw picture dimensions
    prd: Option<DataBlock>,
    /// TIFF IFD
    ttw: Option<DataBlock>,
    /// The IFD container from `ttw`.
    ifd: OnceCell<tiff::Container>,
    /// White balance
    wbg: Option<DataBlock>,
    /// Image processing settings
    rif: Option<DataBlock>,
    /// Padding. Ignore.
    pad: Option<DataBlock>,
}

impl MrwContainer {
    pub(crate) fn new(view: View) -> Self {
        Self {
            endian: Endian::Big,
            view: RefCell::new(view),
            version: String::default(),
            mrm: None,
            prd: None,
            ttw: None,
            ifd: OnceCell::new(),
            wbg: None,
            rif: None,
            pad: None,
        }
    }

    /// Set the endian. This is only useful for Sony A100 that
    /// has a Little Endian MRW data block.
    pub(crate) fn set_endian(&mut self, endian: Endian) {
        self.endian = endian
    }

    /// Load the container. Will read the datablocks, but not their
    /// content.
    pub(crate) fn load(&mut self) -> Result<()> {
        let block = DataBlock::load(0, self)?;
        let end = block.length;
        self.mrm = Some(block);
        let mut position = DATA_BLOCK_HEADER_LENGTH;
        // Datablock can be in different order, except for 'MRM'
        while position < end {
            self.borrow_view_mut().seek(SeekFrom::Start(position))?;
            let block = DataBlock::load(position, self)?;
            position += block.length + DATA_BLOCK_HEADER_LENGTH;
            match block.name() {
                b"PRD" => self.prd = Some(block),
                b"TTW" => self.ttw = Some(block),
                b"WBG" => self.wbg = Some(block),
                b"RIF" => self.rif = Some(block),
                b"PAD" => self.pad = Some(block),
                _ => {}
            }
        }
        if let Some(prd) = &self.prd {
            self.borrow_view_mut().seek(SeekFrom::Start(
                prd.offset + DATA_BLOCK_HEADER_LENGTH + Prd::Version as u64,
            ))?;
            let mut version = vec![0_u8; 8];
            self.borrow_view_mut().read_exact(&mut version)?;
            self.version = String::from_utf8_lossy(&version).to_string();
        }
        Ok(())
    }

    /// Get the IFD container from the `TTW` block.
    /// All the Exif offsets are relative to the begining of the container.
    fn ifd_container(&self) -> &tiff::Container {
        self.ifd.get_or_init(|| {
            let ttw = self.ttw.as_ref().expect("no TTW in the file");
            let view = Viewer::create_subview(&self.view.borrow(), ttw.offset + 8)
                .expect("Couldn't create view");
            let mut ifd = tiff::Container::new(view, vec![(IfdType::Main, None)], Type::Mrw);
            ifd.load(None).expect("Failed to load IFD container");

            ifd
        })
    }

    pub(crate) fn minolta_prd(&self) -> Result<RawInfo> {
        let prd = self.prd.as_ref().ok_or(Error::NotFound)?;
        let y = prd
            .uint16_value(Prd::SensorLength as u64, self)
            .ok_or(Error::NotFound)? as u32;
        let x = prd
            .uint16_value(Prd::SensorWidth as u64, self)
            .ok_or(Error::NotFound)? as u32;
        let bps = prd
            .uint8_value(Prd::PixelSize as u64, self)
            .ok_or(Error::NotFound)? as u16;

        let is_compressed = prd
            .uint8_value(Prd::StorageType as u64, self)
            .map(|storage| storage == StorageType::Packed as u8)
            .unwrap_or(false);
        let mosaic = prd
            .uint16_value(Prd::BayerPattern as u64, self)
            .map(|pattern| {
                if pattern == BayerPattern::Gbrg as u16 {
                    Pattern::Gbrg
                } else {
                    Pattern::Rggb
                }
            })
            .unwrap_or(Pattern::Rggb);

        Ok(RawInfo {
            x,
            y,
            bps,
            is_compressed,
            mosaic,
        })
    }

    pub(crate) fn get_wb(&self) -> Option<[f64; 4]> {
        self.wbg.as_ref().and_then(|wbg| {
            let rd = wbg
                .uint8_value(Wbg::DenominatorR as u64, self)
                .map(|d| 1 << (6 + d))? as f64;
            let gd = wbg
                .uint8_value(Wbg::DenominatorG1 as u64, self)
                .map(|d| 1 << (6 + d))? as f64;
            let bd = wbg
                .uint8_value(Wbg::DenominatorB as u64, self)
                .map(|d| 1 << (6 + d))? as f64;
            let rn = wbg.uint16_value(Wbg::NominatorR as u64, self)? as f64;
            let gn = wbg.uint16_value(Wbg::NominatorG1 as u64, self)? as f64;
            let bn = wbg.uint16_value(Wbg::NominatorB as u64, self)? as f64;

            Some([rd / rn, gd / gn, bd / bn, f64::NAN])
        })
    }

    /// The offset where to find the raw data.
    fn raw_data_offset(&self) -> u64 {
        DATA_BLOCK_HEADER_LENGTH + self.mrm.as_ref().map(|mrm| mrm.length).unwrap_or(0)
    }

    #[cfg(feature = "dump")]
    fn dump_datablock_property_u16<W: std::io::Write + ?Sized>(
        &self,
        out: &mut W,
        indent: u32,
        block: &DataBlock,
        name: &str,
        prop: u64,
    ) {
        if let Some(value) = block.uint16_value(prop, self) {
            dump_writeln!(out, indent, "{name}@{prop} = {value}");
        }
    }

    #[cfg(feature = "dump")]
    fn dump_datablock_property_u8<W: std::io::Write + ?Sized>(
        &self,
        out: &mut W,
        indent: u32,
        block: &DataBlock,
        name: &str,
        prop: u64,
    ) {
        if let Some(value) = block.uint8_value(prop, self) {
            dump_writeln!(out, indent, "{name}@{prop} = {value}");
        }
    }
}

impl RawContainer for MrwContainer {
    fn endian(&self) -> Endian {
        self.endian
    }

    fn borrow_view_mut(&self) -> RefMut<'_, View> {
        self.view.borrow_mut()
    }

    fn raw_type(&self) -> Type {
        Type::Mrw
    }

    fn dir_iterator(&self) -> metadata::Iterator {
        self.ifd_container().dirs().iter().into()
    }
}

impl Dump for MrwContainer {
    #[cfg(feature = "dump")]
    fn write_dump<W: std::io::Write + ?Sized>(&self, out: &mut W, indent: u32) {
        if let Some(mrm) = &self.mrm {
            dump_writeln!(out, indent, "<MRM @{}, len={}>", mrm.offset, mrm.length);
            {
                let indent = indent + 1;
                dump_writeln!(out, indent, "version = {:?}", self.version);
                if let Some(block) = &self.prd {
                    dump_writeln!(out, indent, "<PRD @{}, len={}>", block.offset, block.length);
                    {
                        let indent = indent + 1;
                        self.dump_datablock_property_u16(
                            out,
                            indent,
                            block,
                            "SensorLength",
                            Prd::SensorLength as u64,
                        );
                        self.dump_datablock_property_u16(
                            out,
                            indent,
                            block,
                            "SensorWidth",
                            Prd::SensorWidth as u64,
                        );
                        self.dump_datablock_property_u16(
                            out,
                            indent,
                            block,
                            "ImageLength",
                            Prd::ImageLength as u64,
                        );
                        self.dump_datablock_property_u16(
                            out,
                            indent,
                            block,
                            "ImageWidth",
                            Prd::ImageWidth as u64,
                        );
                        self.dump_datablock_property_u8(
                            out,
                            indent,
                            block,
                            "DataSize",
                            Prd::DataSize as u64,
                        );
                        self.dump_datablock_property_u8(
                            out,
                            indent,
                            block,
                            "PixelSize",
                            Prd::PixelSize as u64,
                        );
                        self.dump_datablock_property_u8(
                            out,
                            indent,
                            block,
                            "StorageType",
                            Prd::StorageType as u64,
                        );
                        self.dump_datablock_property_u16(
                            out,
                            indent,
                            block,
                            "BayerPattern",
                            Prd::BayerPattern as u64,
                        );
                    }
                    dump_writeln!(out, indent, "</PRD>");
                }
                if let Some(block) = &self.wbg {
                    dump_writeln!(
                        out,
                        indent,
                        "<WBG @{}, len={} />",
                        block.offset,
                        block.length
                    );
                }
                if let Some(block) = &self.rif {
                    dump_writeln!(
                        out,
                        indent,
                        "<RIF @{}, len={} />",
                        block.offset,
                        block.length
                    );
                }
                if let Some(block) = &self.ttw {
                    dump_writeln!(out, indent, "<TTW @{}, len={}>", block.offset, block.length);
                    {
                        let indent = indent + 1;
                        self.ifd_container().write_dump(out, indent);
                    }
                    dump_writeln!(out, indent, "</TTW>");
                }
                if let Some(block) = &self.pad {
                    dump_writeln!(
                        out,
                        indent,
                        "<PAD @{}, len={} />",
                        block.offset,
                        block.length
                    );
                }
            }
            dump_writeln!(out, indent, "<MRM>");
        }
    }
}
