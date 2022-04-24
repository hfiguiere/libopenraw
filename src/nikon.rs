/*
 * libopenraw - nikon.rs
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

//! Nikon specific code.

mod diffiterator;
mod huffman;
mod matrices;

use std::collections::HashMap;
use std::io::{Read, Seek, SeekFrom};
use std::rc::Rc;

use byteorder::ReadBytesExt;
use once_cell::unsync::OnceCell;

use crate::bitmap::Bitmap;
use crate::camera_ids::{nikon, vendor};
use crate::container::RawContainer;
use crate::decompress;
use crate::io::Viewer;
use crate::rawfile::ReadAndSeek;
use crate::thumbnail;
use crate::tiff;
use crate::tiff::exif;
use crate::tiff::{Dir, Ifd};
use crate::{DataType, Dump, Error, RawData, RawFile, RawFileImpl, Result, Type, TypeId};

use diffiterator::{CfaIterator, DiffIterator};
use matrices::MATRICES;

lazy_static::lazy_static! {
    /// Nikon1 MakerNote tag names
    pub static ref MNOTE_TAG_NAMES: HashMap<u16, &'static str> = HashMap::from([
        (0x1, "MakerNoteVersion"),
        (0x2, "ISO"),
        (0x3, "ColorMode"),
        (0x4, "Quality"),
        (0x5, "WhiteBalance"),
        (0x6, "Sharpness"),
        (0x7, "FocusMode"),
        (0x8, "FlashSetting"),
        (0x9, "FlashType"),
        (0xb, "WhiteBalanceFineTune"),
        (0xc, "WB_RBLevels"),
        (0xd, "ProgramShift"),
        (0xe, "ExposureDifference"),
        (0xf, "ISOSelection"),
        (0x10, "DataDump"),
        (0x11, "PreviewIFD"),
        (0x12, "FlashExposureComp"),
        (0x13, "ISOSetting"),
        (0x14, "ColorBalanceA"),
        (0x16, "ImageBoundary"),
        (0x17, "ExternalFlashExposureComp"),
        (0x18, "FlashExposureBracketValue"),
        (0x19, "ExposureBracketValue"),
        (0x1a, "ImageProcessing"),
        (0x1b, "CropHiSpeed"),
        (0x1c, "ExposureTuning"),
        (0x1d, "SerialNumber"),
        (0x1e, "ColorSpace"),
        (0x1f, "VRInfo"),
        (0x20, "ImageAuthentication"),
        (0x21, "FaceDetect"),
        (0x22, "ActiveD-Lighting"),
        (0x23, "PictureControlData"),
        (0x24, "WorldTime"),
        (0x25, "ISOInfo"),
        (0x2a, "VignetteControl"),
        (0x2b, "DistortInfo"),
        (0x2c, "UnknownInfo"),
        (0x32, "UnknownInfo2"),
        (0x34, "ShutterMode"),
        (0x35, "HDRInfo"),
        (0x37, "MechanicalShutterCount"),
        (0x39, "LocationInfo"),
        (0x3d, "BlackLevel"),
        (0x3e, "ImageSizeRAW"),
        (0x45, "CropArea"),
        (0x4e, "NikonSettings"),
        (0x4f, "ColorTemperatureAuto"),
        (0x51, "MakerNotes0x51"),
        (0x80, "ImageAdjustment"),
        (0x81, "ToneComp"),
        (0x82, "AuxiliaryLens"),
        (0x83, "LensType"),
        (0x84, "Lens"),
        (0x85, "ManualFocusDistance"),
        (0x86, "DigitalZoom"),
        (0x87, "FlashMode"),
        (0x88, "AFInfo"),
        (0x89, "ShootingMode"),
        (0x8b, "LensFStops"),
        (0x8c, "ContrastCurve"),
        (0x8d, "ColorHue"),
        (0x8f, "SceneMode"),
        (0x90, "LightSource"),
        (0x91, "ShotInfoD40"),
        (0x92, "HueAdjustment"),
        (0x93, "NEFCompression"),
        (0x94, "SaturationAdj"),
        (0x95, "NoiseReduction"),
        (0x96, "NEFLinearizationTable"),
        (0x97, "ColorBalance0100"),
        (0x98, "LensData0100"),
        (0x99, "RawImageCenter"),
        (0x9a, "SensorPixelSize"),
        (0x9c, "SceneAssist"),
        (0x9d, "DateStampMode"),
        (0x9e, "RetouchHistory"),
        (0xa0, "SerialNumber"),
        (0xa2, "ImageDataSize"),
        (0xa5, "ImageCount"),
        (0xa6, "DeletedImageCount"),
        (0xa7, "ShutterCount"),
        (0xa8, "FlashInfo0100"),
        (0xa9, "ImageOptimization"),
        (0xaa, "Saturation"),
        (0xab, "VariProgram"),
        (0xac, "ImageStabilization"),
        (0xad, "AFResponse"),
        (0xb0, "MultiExposure"),
        (0xb1, "HighISONoiseReduction"),
        (0xb3, "ToningEffect"),
        (0xb6, "PowerUpTime"),
        (0xb7, "AFInfo2"),
        (0xb8, "FileInfo"),
        (0xb9, "AFTune"),
        (0xbb, "RetouchInfo"),
        (0xbd, "PictureControlData"),
        (0xbf, "SilentPhotography"),
        (0xc3, "BarometerInfo"),
        (0xe00, "PrintIM"),
        (0xe01, "NikonCaptureData"),
        (0xe09, "NikonCaptureVersion"),
        (0xe0e, "NikonCaptureOffsets"),
        (0xe10, "NikonScanIFD"),
        (0xe13, "NikonCaptureEditVersions"),
        (0xe1d, "NikonICCProfile"),
        (0xe1e, "NikonCaptureOutput"),
        (0xe22, "NEFBitDepth"),
    ]);

    /// Nikon2 MakerNote tag names
    pub static ref MNOTE_TAG_NAMES_2: HashMap<u16, &'static str> = HashMap::from([
        (0x3, "Quality"),
        (0x4, "ColorMode"),
        (0x5, "ImageAdjustment"),
        (0x6, "CCDSensitivity"),
        (0x7, "WhiteBalance"),
        (0x8, "Focus"),
        (0xa, "DigitalZoom"),
        (0xb, "Converter"),
    ]);

    static ref MAKE_TO_ID_MAP: tiff::MakeToIdMap = HashMap::from([
        ("NIKON D1 ", TypeId(vendor::NIKON, nikon::D1)),
        ("NIKON D100 ", TypeId(vendor::NIKON, nikon::D100)),
        ("NIKON D1X", TypeId(vendor::NIKON, nikon::D1X)),
        ("NIKON D200", TypeId(vendor::NIKON, nikon::D200)),
        ("NIKON D2H", TypeId(vendor::NIKON, nikon::D2H)),
        ("NIKON D2Hs", TypeId(vendor::NIKON, nikon::D2HS)),
        ("NIKON D2X", TypeId(vendor::NIKON, nikon::D2X)),
        ("NIKON D2Xs", TypeId(vendor::NIKON, nikon::D2XS )),
        ("NIKON D3", TypeId(vendor::NIKON, nikon::D3)),
        ("NIKON D3S", TypeId(vendor::NIKON, nikon::D3S)),
        ("NIKON D3X", TypeId(vendor::NIKON, nikon::D3X)),
        ("NIKON D300", TypeId(vendor::NIKON, nikon::D300)),
        ("NIKON D300S", TypeId(vendor::NIKON, nikon::D300S)),
        ("NIKON D3000", TypeId(vendor::NIKON, nikon::D3000)),
        ("NIKON D3100", TypeId(vendor::NIKON, nikon::D3100)),
        ("NIKON D3200", TypeId(vendor::NIKON, nikon::D3200)),
        ("NIKON D3300", TypeId(vendor::NIKON, nikon::D3300)),
        ("NIKON D3400", TypeId(vendor::NIKON, nikon::D3400)),
        ("NIKON D3500", TypeId(vendor::NIKON, nikon::D3500)),
        ("NIKON D4", TypeId(vendor::NIKON, nikon::D4)),
        ("NIKON D4S", TypeId(vendor::NIKON, nikon::D4S)),
        ("NIKON D40", TypeId(vendor::NIKON, nikon::D40)),
        ("NIKON D40X", TypeId(vendor::NIKON, nikon::D40X)),
        ("NIKON D5", TypeId(vendor::NIKON, nikon::D5)),
        ("NIKON D50", TypeId(vendor::NIKON, nikon::D50)),
        ("NIKON D500", TypeId(vendor::NIKON, nikon::D500)),
        ("NIKON D5000", TypeId(vendor::NIKON, nikon::D5000)),
        ("NIKON D5100", TypeId(vendor::NIKON, nikon::D5100)),
        ("NIKON D5200", TypeId(vendor::NIKON, nikon::D5200)),
        ("NIKON D5300", TypeId(vendor::NIKON, nikon::D5300)),
        ("NIKON D5500", TypeId(vendor::NIKON, nikon::D5500)),
        ("NIKON D5600", TypeId(vendor::NIKON, nikon::D5600)),
        ("NIKON D6", TypeId(vendor::NIKON, nikon::D6)),
        ("NIKON D60",   TypeId(vendor::NIKON, nikon::D60)),
        ("NIKON D600", TypeId(vendor::NIKON, nikon::D600)),
        ("NIKON D610", TypeId(vendor::NIKON, nikon::D610)),
        ("NIKON D70", TypeId(vendor::NIKON, nikon::D70)),
        ("NIKON D70s", TypeId(vendor::NIKON, nikon::D70S)),
        ("NIKON D700", TypeId(vendor::NIKON, nikon::D700)),
        ("NIKON D7000", TypeId(vendor::NIKON, nikon::D7000)),
        ("NIKON D7100", TypeId(vendor::NIKON, nikon::D7100)),
        ("NIKON D7200", TypeId(vendor::NIKON, nikon::D7200)),
        ("NIKON D750", TypeId(vendor::NIKON, nikon::D750)),
        ("NIKON D780", TypeId(vendor::NIKON, nikon::D780)),
        ("NIKON D80", TypeId(vendor::NIKON, nikon::D80)),
        ("NIKON D800", TypeId(vendor::NIKON, nikon::D800)),
        ("NIKON D800E", TypeId(vendor::NIKON, nikon::D800E)),
        ("NIKON D810", TypeId(vendor::NIKON, nikon::D810)),
        ("NIKON D90", TypeId(vendor::NIKON, nikon::D90)),
        ("NIKON Df", TypeId(vendor::NIKON, nikon::DF)),
        ("NIKON Z 6", TypeId(vendor::NIKON, nikon::Z6)),
        ("NIKON Z 6_2", TypeId(vendor::NIKON, nikon::Z6_2)),
        ("NIKON Z 7", TypeId(vendor::NIKON, nikon::Z7)),
        ("NIKON Z 7_2", TypeId(vendor::NIKON, nikon::Z7_2)),
        ("NIKON Z 50", TypeId(vendor::NIKON, nikon::Z50)),
        ("NIKON Z 5", TypeId(vendor::NIKON, nikon::Z5)),
        ("NIKON Z 9", TypeId(vendor::NIKON, nikon::Z9)),
        ("NIKON Z fc", TypeId(vendor::NIKON, nikon::ZFC)),
        ("E5400", TypeId(vendor::NIKON, nikon::E5400)),
        ("E5700", TypeId(vendor::NIKON, nikon::E5700)),
        ("E8400", TypeId(vendor::NIKON, nikon::E8400)),
        ("E8800", TypeId(vendor::NIKON, nikon::E8800)),
        ("COOLPIX B700", TypeId(vendor::NIKON, nikon::COOLPIX_B700)),
        ("COOLPIX P330", TypeId(vendor::NIKON, nikon::COOLPIX_P330)),
        ("COOLPIX P340", TypeId(vendor::NIKON, nikon::COOLPIX_P340)),
        ("COOLPIX P950", TypeId(vendor::NIKON, nikon::COOLPIX_P950)),
        ("COOLPIX P1000", TypeId(vendor::NIKON, nikon::COOLPIX_P1000)),
        ("COOLPIX P6000", TypeId(vendor::NIKON, nikon::COOLPIX_P6000)),
        ("COOLPIX P7000", TypeId(vendor::NIKON, nikon::COOLPIX_P7000)),
        ("COOLPIX P7100", TypeId(vendor::NIKON, nikon::COOLPIX_P7100)),
        ("COOLPIX P7700", TypeId(vendor::NIKON, nikon::COOLPIX_P7700)),
        ("COOLPIX A", TypeId(vendor::NIKON, nikon::COOLPIX_A)),
        ("COOLPIX A1000", TypeId(vendor::NIKON, nikon::COOLPIX_A1000)),
        ("NIKON 1 J1", TypeId(vendor::NIKON, nikon::NIKON1_J1)),
        ("NIKON 1 J2", TypeId(vendor::NIKON, nikon::NIKON1_J2)),
        ("NIKON 1 J3", TypeId(vendor::NIKON, nikon::NIKON1_J3)),
        ("NIKON 1 J4", TypeId(vendor::NIKON, nikon::NIKON1_J4)),
        ("NIKON 1 J5", TypeId(vendor::NIKON, nikon::NIKON1_J5)),
        ("NIKON 1 V1", TypeId(vendor::NIKON, nikon::NIKON1_V1)),
        ("NIKON 1 V2", TypeId(vendor::NIKON, nikon::NIKON1_V2)),
        ("NIKON 1 V3", TypeId(vendor::NIKON, nikon::NIKON1_V3)),
        ("NIKON 1 S1", TypeId(vendor::NIKON, nikon::NIKON1_S1)),
        ("NIKON 1 S2", TypeId(vendor::NIKON, nikon::NIKON1_S2)),
        ("NIKON 1 AW1", TypeId(vendor::NIKON, nikon::NIKON1_AW1)),
    ]);
}

struct CompressionInfo {
    vpred: [[u16; 2]; 2],
    curve: Vec<u16>,
    huffman: Option<&'static [huffman::HuffmanNode]>,
}

impl CompressionInfo {
    fn new() -> Self {
        Self {
            vpred: [[0; 2]; 2],
            curve: vec![0; 0x8000],
            huffman: None,
        }
    }
}

pub struct NefFile {
    reader: Rc<Viewer>,
    type_id: OnceCell<TypeId>,
    container: OnceCell<tiff::Container>,
    thumbnails: OnceCell<Vec<(u32, thumbnail::ThumbDesc)>>,
}

impl NefFile {
    pub fn factory(reader: Box<dyn ReadAndSeek>) -> Box<dyn RawFile> {
        let viewer = Viewer::new(reader, 0);
        Box::new(NefFile {
            reader: viewer,
            type_id: OnceCell::new(),
            container: OnceCell::new(),
            thumbnails: OnceCell::new(),
        })
    }

    fn is_nrw(&self) -> bool {
        // XXX cache?
        self.ifd(tiff::Type::MakerNote)
            .and_then(|mnote| {
                mnote
                    .value::<String>(exif::MNOTE_NIKON_QUALITY)
                    .map(|value| value == "NRW")
            })
            .unwrap_or(false)
    }

    /// The Raw file is from a D100.
    fn is_d100(&self) -> bool {
        self.type_id() == TypeId(vendor::NIKON, nikon::D100)
    }

    /// Unpack Nikon.
    fn unpack_nikon(&self, rawdata: RawData) -> Result<RawData> {
        let mut width = rawdata.width();
        if self.is_d100() {
            width += 6;
        }
        let height = rawdata.height();
        let bpc = rawdata.bpc();
        let block_size: usize = match bpc {
            12 => ((width / 2 * 3) + width / 10) as usize,
            _ => {
                log::warn!("Invalid BPC {}", bpc);
                return Err(Error::InvalidFormat);
            }
        };
        log::debug!("block_size {} width {} ", block_size, width);

        let mut data = rawdata.data8().ok_or(Error::NotFound)?;

        let mut block = Vec::new();
        block.resize(block_size, 0);
        let out_size = width as usize * height as usize;
        let mut out_data = Vec::with_capacity(out_size);
        let mut fetched = 0_usize;
        let mut written = 0_usize;

        let byte_len = std::cmp::min(data.len(), block_size * height as usize);
        while fetched < byte_len {
            data.read_exact(block.as_mut_slice())?;
            fetched += block.len();

            written +=
                decompress::unpack_be12to16(&block, &mut out_data, tiff::Compression::NikonPack)?;
        }
        log::debug!("Unpacked {} pixels", written);

        let mut rawdata = rawdata.replace_data(out_data);
        rawdata.set_data_type(DataType::Raw);

        Ok(rawdata)
    }

    fn get_compression_curve(&self, rawdata: &mut RawData) -> Result<CompressionInfo> {
        self.ifd(tiff::Type::MakerNote)
            .ok_or_else(|| {
                log::error!("No MakerNote");
                Error::NotFound
            })
            .and_then(|mnote| {
                let curve_entry =
                    mnote
                        .entry(exif::MNOTE_NIKON_NEFDECODETABLE2)
                        .ok_or_else(|| {
                            log::error!("DecodeTable2 not found");
                            Error::NotFound
                        })?;
                let pos = curve_entry.offset().ok_or(Error::NotFound)? + mnote.mnote_offset;
                let bpc = rawdata.bpc();

                let container = self.container.get().unwrap();
                let mut view = container.borrow_view_mut();
                view.seek(SeekFrom::Start(pos as u64))?;
                let header0 = view.read_u8()?;
                let header1 = view.read_u8()?;

                if header0 == 0x49 {
                    // some interesting stuff at 2110
                    // XXX we need to implement this.
                    log::warn!("NEF: header0 is 0x49 - case not yet handled.");
                    view.seek(SeekFrom::Current(2110))?;
                }

                let mut curve = CompressionInfo::new();
                for i in 0..2 {
                    for j in 0..2 {
                        curve.vpred[i][j] = container.read_u16(&mut view)?;
                    }
                }

                let mut header_ok = false;
                // header0 == 0x44 || 0x49 -> lossy
                // header0 == 0x46 -> lossless
                if header0 == 0x44 || header0 == 0x49 {
                    if bpc == 12 {
                        curve.huffman = Some(&diffiterator::LOSSY_12BIT);
                        log::debug!("12 bits lossy {}", bpc);
                        header_ok = true;
                    } else if bpc == 14 {
                        curve.huffman = Some(&diffiterator::LOSSY_14BIT);
                        log::debug!("14 bits lossy {}", bpc);
                        header_ok = true;
                    }
                } else if header0 == 0x46 {
                    if bpc == 14 {
                        curve.huffman = Some(&diffiterator::LOSSLESS_14BIT);
                        log::debug!("14 bits lossless");
                        header_ok = true;
                    } else if bpc == 12 {
                        // curve.huffman = Some(&diffiterator::LOSSLESS_12BIT);
                        log::debug!("12 bits lossless");
                        log::error!("12 bits lossless isn't yet supported");
                        // header_ok = true;
                        return Err(Error::NotSupported);
                    }
                }
                if !header_ok {
                    log::error!("Wrong header, found {}-{}", header0, header1);
                    return Err(Error::FormatError);
                }

                let nelems = container.read_u16(&mut view).unwrap_or(0);
                log::debug!("Num elems {}", nelems);

                let mut ceiling: u16 = 1 << bpc & 0x7fff;
                let mut step = 0_usize;
                if nelems > 1 {
                    step = (ceiling / (nelems - 1)) as usize;
                }
                log::debug!("ceiling {}, step {}", ceiling, step);

                if header0 == 0x44 && header1 == 0x20 && step > 0 {
                    for i in 0..nelems {
                        let value = container.read_u16(&mut view)?;
                        curve.curve[i as usize * step as usize] = value;
                    }
                    for i in 0..ceiling as usize {
                        curve.curve[i] = ((curve.curve[i - i % step] as usize * (step - i % step)
                            + curve.curve[i - i % step + step] as usize * (i % step))
                            / step) as u16;
                    }
                    // split flag is at offset 562.
                    // XXX
                } else if header0 != 0x46 && nelems <= 0x4001 {
                    let num_read =
                        container.read_u16_array(&mut view, &mut curve.curve, nelems as usize)?;
                    // XXX It's likly not possible for the short read to appear.
                    // As an error might be thrown first.
                    if num_read < nelems as usize {
                        log::error!("NEF short read of {} elems, expected {}", num_read, nelems);
                        return Err(Error::UnexpectedEOF);
                    }
                    ceiling = nelems;
                }

                let black = curve.curve[0];
                let white = curve.curve[ceiling as usize - 1];
                for i in ceiling..0x8000 {
                    // XXX there is a more rusty way to do that
                    curve.curve[i as usize] = white;
                }

                rawdata.set_white(white);
                rawdata.set_black(black);

                Ok(curve)
            })
    }

    fn decompress_nikon_quantized(&self, mut rawdata: RawData) -> Result<RawData> {
        self.get_compression_curve(&mut rawdata)
            .map_err(|err| {
                log::error!("Get compression curve failed {}", err);
                err
            })
            .and_then(|curve| {
                let rows = rawdata.height() as usize;
                let raw_columns = rawdata.width() as usize;
                // XXX not always true
                let columns = raw_columns - 1;

                let data8 = rawdata.data8();
                if data8.is_none() {
                    return Err(Error::FormatError);
                }
                let diffs =
                    DiffIterator::new(curve.huffman.unwrap(), rawdata.data8().as_ref().unwrap());
                let mut iter = CfaIterator::new(diffs, raw_columns, curve.vpred);

                let mut new_data = vec![0; rows * columns];
                for i in 0..rows {
                    for j in 0..raw_columns {
                        let t = iter.get().map_err(|err| {
                            log::error!("Error get");
                            err
                        })?;
                        if j < columns {
                            let shift: u16 = 16 - rawdata.bpc();
                            new_data[i * columns + j] = curve.curve[t as usize & 0x3fff] << shift;
                        }
                    }
                }

                let mut rawdata = rawdata.replace_data(new_data);
                rawdata.set_width(columns as u32);
                rawdata.set_white((1 << rawdata.bpc()) - 1);
                rawdata.set_data_type(DataType::Raw);
                Ok(rawdata)
            })
    }
}

impl RawFileImpl for NefFile {
    fn identify_id(&self) -> TypeId {
        *self.type_id.get_or_init(|| {
            self.container();
            let container = self.container.get().unwrap();
            tiff::identify_with_exif(container, &MAKE_TO_ID_MAP).unwrap_or(TypeId(vendor::NIKON, 0))
        })
    }

    fn container(&self) -> &dyn RawContainer {
        self.container.get_or_init(|| {
            // XXX we should be faillible here.
            let view = Viewer::create_view(&self.reader, 0).expect("Created view");
            let mut container = tiff::Container::new(view, vec![tiff::Type::Main], self.type_());
            container.load(None).expect("NEF container error");
            container
        })
    }

    fn thumbnails(&self) -> &Vec<(u32, thumbnail::ThumbDesc)> {
        self.thumbnails.get_or_init(|| {
            self.container();
            let container = self.container.get().unwrap();
            let mut thumbnails = tiff::tiff_thumbnails(container);

            // Get the preview in the makernote
            if let Some(mnote) = self.ifd(tiff::Type::MakerNote) {
                mnote
                    .ifd_in_entry(container, exif::MNOTE_NIKON_PREVIEW_IFD)
                    .and_then(|dir| {
                        let start = dir.value::<u32>(exif::MNOTE_NIKON_PREVIEWIFD_START)?
                            + mnote.mnote_offset;
                        let len = dir.value::<u32>(exif::MNOTE_NIKON_PREVIEWIFD_LENGTH)?;
                        container
                            .add_thumbnail_from_stream(start, len, &mut thumbnails)
                            .ok()
                    });
            }
            thumbnails
        })
    }

    fn ifd(&self, ifd_type: tiff::Type) -> Option<Rc<Dir>> {
        self.container();
        let container = self.container.get().unwrap();
        match ifd_type {
            tiff::Type::Main => container.directory(0),
            tiff::Type::Cfa => tiff::tiff_locate_cfa_ifd(container),
            tiff::Type::Exif => self
                .ifd(tiff::Type::Main)
                .and_then(|dir| dir.get_exif_ifd(container)),
            tiff::Type::MakerNote => self
                .ifd(tiff::Type::Exif)
                .and_then(|dir| dir.get_mnote_ifd(container)),
            _ => None,
        }
    }

    fn load_rawdata(&self) -> Result<RawData> {
        self.ifd(tiff::Type::Cfa)
            .ok_or_else(|| {
                log::error!("CFA not found");
                Error::NotFound
            })
            .and_then(|ref dir| {
                tiff::tiff_get_rawdata(self.container.get().unwrap(), dir)
                    .map_err(|err| {
                        log::error!("NEF get rawdata failed {}", err);
                        err
                    })
                    .and_then(|rawdata| {
                        let compression = rawdata.compression();
                        if self.is_nrw() {
                            // XXX decompression not yet supported
                            log::error!("NRW compression unsupported");
                            Ok(rawdata)
                        } else if self.is_d100() {
                            self.unpack_nikon(rawdata)
                        } else if compression == tiff::Compression::None {
                            Ok(rawdata)
                        } else if compression == tiff::Compression::NikonQuantized {
                            log::debug!("Nikon quantized");
                            self.decompress_nikon_quantized(rawdata).map_err(|err| {
                                log::error!("NEF quantized {}", err);
                                err
                            })
                        } else {
                            log::error!("Invalid compression {:?}", compression);
                            Err(Error::InvalidFormat)
                        }
                    })
            })
    }

    fn get_builtin_colour_matrix(&self) -> Result<Vec<f64>> {
        MATRICES
            .iter()
            .find(|m| m.camera == self.type_id())
            .map(|m| Vec::from(m.matrix))
            .ok_or(Error::NotFound)
    }
}

impl RawFile for NefFile {
    fn type_(&self) -> Type {
        Type::Nef
    }
}

impl Dump for NefFile {
    #[cfg(feature = "dump")]
    fn print_dump(&self, indent: u32) {
        dump_println!(indent, "<Nikon NEF File>");
        {
            let indent = indent + 1;
            self.container().print_dump(indent);
        }
        dump_println!(indent, "</Nikon NEF File>");
    }
}
