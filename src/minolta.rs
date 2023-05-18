// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - minolta.rs
 *
 * Copyright (C) 2023 Hubert Figuière
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

use std::cell::{RefCell, RefMut};
use std::collections::HashMap;
use std::io::{Read, Seek, SeekFrom};
use std::rc::Rc;

use byteorder::{BigEndian, ReadBytesExt};
use once_cell::unsync::OnceCell;

use crate::colour::BuiltinMatrix;
use crate::container::{self, RawContainer};
use crate::decompress;
use crate::io::{View, Viewer};
use crate::mosaic::Pattern;
use crate::rawfile::ReadAndSeek;
use crate::thumbnail;
use crate::tiff::{self, exif, Ifd, IfdType};
use crate::{DataType, Dump, Error, RawData, RawFile, RawFileImpl, Result, Type, TypeId};

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

    pub static ref MNOTE_TAG_NAMES: HashMap<u16, &'static str> = HashMap::from([
        (0x0, "MakerNoteVersion"),
        (0x1, "MinoltaCameraSettingsOld"),
        (0x3, "MinoltaCameraSettings"),
        (0x4, "MinoltaCameraSettings7D"),
        (0x10, "CameraInfoA100"),
        (0x18, "ISInfoA100"),
        (0x20, "WBInfoA100"),
        (0x40, "CompressedImageSize"),
        (0x81, "PreviewImage"),
        (0x88, "PreviewImageStart"),
        (0x89, "PreviewImageLength"),
        (0x100, "SceneMode"),
        (0x101, "ColorMode"),
        (0x102, "MinoltaQuality"),
        (0x103, "MinoltaQuality"),
        (0x104, "FlashExposureComp"),
        (0x105, "Teleconverter"),
        (0x107, "ImageStabilization"),
        (0x109, "RawAndJpgRecording"),
        (0x10a, "ZoneMatching"),
        (0x10b, "ColorTemperature"),
        (0x10c, "LensType"),
        (0x111, "ColorCompensationFilter"),
        (0x112, "WhiteBalanceFineTune"),
        (0x113, "ImageStabilization"),
        (0x114, "MinoltaCameraSettings5D"),
        (0x115, "WhiteBalance"),
        (0xe00, "PrintIM"),
        (0xf00, "MinoltaCameraSettings2"),
    ]);
}

/// The MRW file format was produced by Minolta cameras until
/// Konica-Minolta was purchased by Sony. This will not change.
///
/// Sources http://www.dalibor.cz/software/minolta-raw-mrw-file-format
pub(crate) struct MrwFile {
    reader: Rc<Viewer>,
    container: OnceCell<MrwContainer>,
    thumbnails: OnceCell<Vec<(u32, thumbnail::ThumbDesc)>>,
}

impl MrwFile {
    pub fn factory(reader: Box<dyn ReadAndSeek>) -> Box<dyn RawFile> {
        let viewer = Viewer::new(reader, 0);
        Box::new(MrwFile {
            reader: viewer,
            container: OnceCell::new(),
            thumbnails: OnceCell::new(),
        })
    }
}

impl RawFileImpl for MrwFile {
    fn identify_id(&self) -> TypeId {
        self.container();
        *self
            .container
            .get()
            .and_then(|container| MODEL_ID_MAP.get(&container.version.as_str()))
            .unwrap_or(&minolta!(UNKNOWN))
    }

    fn container(&self) -> &dyn RawContainer {
        self.container.get_or_init(|| {
            log::debug!("Creating mrw container");
            // XXX we should be faillible here.
            let view = Viewer::create_view(&self.reader, 0).expect("Created view");
            let mut container = MrwContainer::new(view);
            container.load().expect("Failed to load container");

            container
        })
    }

    /// MRW files have only one preview, 640x480, in the MakerNote.
    fn thumbnails(&self) -> &Vec<(u32, thumbnail::ThumbDesc)> {
        self.thumbnails.get_or_init(|| {
            let mut thumbnails = vec![];
            if let Some(makernote) = self.ifd(tiff::IfdType::MakerNote) {
                let ifd = self.container.get().unwrap().ifd_container();
                // Old files have the thummail in the Exif entry `MNOTE_MINOLTA_THUMBNAIL`.
                let buffer = if let Some(preview) = makernote.entry(exif::MNOTE_MINOLTA_THUMBNAIL) {
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
            thumbnails
        })
    }

    /// In a MRW file, all comes out of the IFD container from the `TTW`
    /// block.
    fn ifd(&self, ifd_type: tiff::IfdType) -> Option<&tiff::Dir> {
        self.container();
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
    fn load_rawdata(&self, skip_decompress: bool) -> Result<RawData> {
        self.container();
        if let Some(container) = self.container.get() {
            let prd = container.prd.as_ref().ok_or(Error::NotFound)?;
            let y = prd
                .uint16_value(Prd::SensorLength as u64, container)
                .ok_or(Error::NotFound)? as u32;
            let x = prd
                .uint16_value(Prd::SensorWidth as u64, container)
                .ok_or(Error::NotFound)? as u32;
            let bps = prd
                .uint8_value(Prd::PixelSize as u64, container)
                .ok_or(Error::NotFound)? as u16;

            let is_compressed = prd
                .uint8_value(Prd::StorageType as u64, container)
                .map(|storage| storage == StorageType::Packed as u8)
                .unwrap_or(false);
            let mosaic = prd
                .uint16_value(Prd::BayerPattern as u64, container)
                .map(|pattern| {
                    if pattern == BayerPattern::Gbrg as u16 {
                        Pattern::Gbrg
                    } else {
                        Pattern::Rggb
                    }
                })
                .unwrap_or(Pattern::Rggb);

            let cfa_offset = container.raw_data_offset();
            let cfa_len = if is_compressed {
                x * y + ((x * y) >> 1)
            } else {
                2 * x * y
            } as u64;
            let mut rawdata = if is_compressed {
                let raw = container.load_buffer8(cfa_offset, cfa_len);
                if skip_decompress {
                    RawData::new8(x, y, bps, DataType::CompressedRaw, raw, mosaic)
                } else {
                    let mut unpacked = Vec::with_capacity((x * y) as usize);
                    decompress::unpack_be12to16(&raw, &mut unpacked, tiff::Compression::None)
                        .map_err(|err| {
                            log::error!("RAF failed to unpack {}", err);
                            err
                        })?;
                    RawData::new16(x, y, 16, DataType::Raw, unpacked, mosaic)
                }
            } else {
                let raw = container.load_buffer16_be(cfa_offset, cfa_len);
                RawData::new16(x, y, bps, DataType::Raw, raw, mosaic)
            };
            if let Some((black, white)) = MATRICES
                .iter()
                .find(|m| m.camera == self.type_id())
                .map(|m| (m.black, m.white))
            {
                rawdata.set_white(white);
                rawdata.set_black(black);
            }
            Ok(rawdata)
        } else {
            Err(Error::NotFound)
        }
    }

    fn get_builtin_colour_matrix(&self) -> Result<Vec<f64>> {
        MATRICES
            .iter()
            .find(|m| m.camera == self.type_id())
            .map(|m| Vec::from(m.matrix))
            .ok_or(Error::NotFound)
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
        self.container();
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
enum Wbg {
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

/// Datablock of the MRW file.
struct DataBlock {
    /// Offset from the begining of the file.
    offset: u64,
    /// Nmae of the block. 3 letter code in a u32 big endian. Starts with `NUL`.
    name: [u8; 4],
    /// Length of the block.
    length: u64,
}

impl DataBlock {
    /// Load the datablock from the container.
    fn load(offset: u64, container: &MrwContainer) -> Result<DataBlock> {
        let mut name = [0_u8; 4];
        container.borrow_view_mut().read_exact(&mut name)?;
        let length = container.borrow_view_mut().read_u32::<BigEndian>()? as u64;
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
        view.read_u16::<BigEndian>().ok()
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

struct MrwContainer {
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
    fn new(view: View) -> Self {
        Self {
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

    /// Load the container. Will read the datablocks, but not their
    /// content.
    fn load(&mut self) -> Result<()> {
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
            let mut ifd = tiff::Container::new(view, vec![IfdType::Main], Type::Mrw);
            ifd.load(None).expect("Failed to load IFD container");

            ifd
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
    fn endian(&self) -> container::Endian {
        container::Endian::Big
    }

    fn borrow_view_mut(&self) -> RefMut<'_, View> {
        self.view.borrow_mut()
    }

    fn raw_type(&self) -> Type {
        Type::Mrw
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
