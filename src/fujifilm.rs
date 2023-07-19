// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - fujifilm.rs
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

//! Fujifilm RAF format

mod matrices;
mod raf;

use std::collections::HashMap;
use std::convert::TryFrom;
use std::rc::Rc;

use once_cell::unsync::OnceCell;

use crate::bitmap::{Point, Rect, Size};
use crate::container::RawContainer;
use crate::decompress;
use crate::io::Viewer;
use crate::mosaic::Pattern;
use crate::rawfile::{ReadAndSeek, ThumbnailStorage};
use crate::thumbnail;
use crate::thumbnail::{Data, DataOffset};
use crate::tiff;
use crate::tiff::{exif, Ifd};
use crate::{DataType, Dump, Error, RawData, RawFile, RawFileImpl, Result, Type, TypeId};

use matrices::MATRICES;

pub(crate) const RAF_MAGIC: &[u8] = b"FUJIFILMCCD-RAW ";

#[macro_export]
macro_rules! fuji {
    ($id:expr, $model:ident) => {
        (
            $id,
            TypeId(
                $crate::camera_ids::vendor::FUJIFILM,
                $crate::camera_ids::fujifilm::$model,
            ),
        )
    };
    ($model:ident) => {
        TypeId(
            $crate::camera_ids::vendor::FUJIFILM,
            $crate::camera_ids::fujifilm::$model,
        )
    };
}

lazy_static::lazy_static! {
    /// Make to TypeId map for RAF files.
    static ref MAKE_TO_ID_MAP: tiff::MakeToIdMap = HashMap::from([
        fuji!("GFX 50S", GFX50S),
        fuji!("GFX50S II", GFX50S_II),
        fuji!("GFX 50R", GFX50R),
        fuji!("GFX 100", GFX100),
        fuji!("GFX100S", GFX100S),
        fuji!("FinePix F550EXR", F550EXR),
        fuji!("FinePix F700  ", F700),
        fuji!("FinePix F810   ", F810),
        fuji!("FinePix E900   ", E900),
        fuji!("FinePixS2Pro", S2PRO),
        fuji!("FinePix S3Pro  ", S3PRO),
        fuji!("FinePix S5Pro  ", S5PRO),
        fuji!("FinePix S5000 ", S5000),
        fuji!("FinePix S5600  ", S5600),
        fuji!("FinePix S6000fd", S6000FD),
        fuji!("FinePix S6500fd", S6500FD),
        fuji!("FinePix S9500  ", S9500),
        fuji!("FinePix SL1000", SL1000),
        fuji!("FinePix HS10 HS11", HS10),
        fuji!("FinePix HS30EXR", HS30EXR),
        fuji!("FinePix HS33EXR", HS33EXR),
        fuji!("FinePix HS50EXR", HS50EXR),
        fuji!("FinePix S100FS ", S100FS),
        fuji!("FinePix S200EXR", S200EXR),
        fuji!("FinePix X100", X100),
        fuji!("X10", X10),
        fuji!("X20", X20),
        fuji!("X30", X30),
        fuji!("X70", X70),
        fuji!("X-Pro1", XPRO1),
        fuji!("X-Pro2", XPRO2),
        fuji!("X-Pro3", XPRO3),
        fuji!("X-S1", XS1),
        fuji!("X-S10", XS10),
        fuji!("X-A1", XA1),
        fuji!("X-A10", XA10),
        fuji!("X-A2", XA2),
        fuji!("X-A3", XA3),
        fuji!("X-A5", XA5),
        fuji!("X-A7", XA7),
        fuji!("XQ1", XQ1),
        fuji!("XQ2", XQ2),
        fuji!("X-E1", XE1),
        fuji!("X-E2", XE2),
        fuji!("X-E2S", XE2S),
        fuji!("X-E3", XE3),
        fuji!("X-E4", XE4),
        fuji!("X-M1", XM1),
        fuji!("X-T1", XT1),
        fuji!("X-T10", XT10),
        fuji!("X-T100", XT100),
        fuji!("X-T2", XT2),
        fuji!("X-T20", XT20),
        fuji!("X-T200", XT200),
        fuji!("X-T3", XT3),
        fuji!("X-T30", XT30),
        fuji!("X-T30 II", XT30_II),
        fuji!("X-T4", XT4),
        fuji!("X-T5", XT5),
        fuji!("XF1", XF1),
        fuji!("XF10", XF10),
        fuji!("X100S", X100S),
        fuji!("X100T", X100T),
        fuji!("X100F", X100F),
        fuji!("X100V", X100V),
        fuji!("X-H1", XH1),
        fuji!("X-H2", XH2),
        fuji!("X-H2S", XH2S),
    ]);

    pub static ref MNOTE_TAG_NAMES: HashMap<u16, &'static str> = HashMap::from([
        (0x0, "Version"),
        (0x10, "InternalSerialNumber"),
        (0x1000, "Quality"),
        (0x1001, "Sharpness"),
        (0x1002, "WhiteBalance"),
        (0x1003, "Saturation"),
        (0x1004, "Contrast"),
        (0x1005, "ColorTemperature"),
        (0x1006, "Contrast"),
        (0x100a, "WhiteBalanceFineTune"),
        (0x100b, "NoiseReduction"),
        (0x100e, "NoiseReduction"),
        (0x100f, "Clarity"),
        (0x1010, "FujiFlashMode"),
        (0x1011, "FlashExposureComp"),
        (0x1020, "Macro"),
        (0x1021, "FocusMode"),
        (0x1022, "AFMode"),
        (0x1023, "FocusPixel"),
        (0x102b, "PrioritySettings"),
        (0x102d, "FocusSettings"),
        (0x102e, "AFCSettings"),
        (0x1030, "SlowSync"),
        (0x1031, "PictureMode"),
        (0x1032, "ExposureCount"),
        (0x1033, "EXRAuto"),
        (0x1034, "EXRMode"),
        (0x1040, "ShadowTone"),
        (0x1041, "HighlightTone"),
        (0x1044, "DigitalZoom"),
        (0x1045, "LensModulationOptimizer"),
        (0x1047, "GrainEffectRoughness"),
        (0x1048, "ColorChromeEffect"),
        (0x1049, "BWAdjustment"),
        (0x104c, "GrainEffectSize"),
        (0x104d, "CropMode"),
        (0x104e, "ColorChromeFXBlue"),
        (0x1050, "ShutterType"),
        (0x1100, "AutoBracketing"),
        (0x1101, "SequenceNumber"),
        (0x1103, "DriveSettings"),
        (0x1153, "PanoramaAngle"),
        (0x1154, "PanoramaDirection"),
        (0x1201, "AdvancedFilter"),
        (0x1210, "ColorMode"),
        (0x1300, "BlurWarning"),
        (0x1301, "FocusWarning"),
        (0x1302, "ExposureWarning"),
        (0x1304, "GEImageSize"),
        (0x1400, "DynamicRange"),
        (0x1401, "FilmMode"),
        (0x1402, "DynamicRangeSetting"),
        (0x1403, "DevelopmentDynamicRange"),
        (0x1404, "MinFocalLength"),
        (0x1405, "MaxFocalLength"),
        (0x1406, "MaxApertureAtMinFocal"),
        (0x1407, "MaxApertureAtMaxFocal"),
        (0x140b, "AutoDynamicRange"),
        (0x1422, "ImageStabilization"),
        (0x1425, "SceneRecognition"),
        (0x1431, "Rating"),
        (0x1436, "ImageGeneration"),
        (0x1438, "ImageCount"),
        (0x1443, "DRangePriority"),
        (0x1444, "DRangePriorityAuto"),
        (0x1445, "DRangePriorityFixed"),
        (0x1446, "FlickerReduction"),
        (0x1447, "FujiModel"),
        (0x1448, "FujiModel2"),
        (0x3803, "VideoRecordingMode"),
        (0x3804, "PeripheralLighting"),
        (0x3806, "VideoCompression"),
        (0x3820, "FrameRate"),
        (0x3821, "FrameWidth"),
        (0x3822, "FrameHeight"),
        (0x3824, "FullHDHighSpeedRec"),
        (0x4005, "FaceElementSelected"),
        (0x4100, "FacesDetected"),
        (0x4103, "FacePositions"),
        (0x4200, "NumFaceElements"),
        (0x4201, "FaceElementTypes"),
        (0x4203, "FaceElementPositions"),
        (0x4282, "FaceRecInfo"),
        (0x8000, "FileSource"),
        (0x8002, "OrderNumber"),
        (0x8003, "FrameNumber"),
        (0xb211, "Parallax"),
    ]);
}

pub(crate) struct RafFile {
    reader: Rc<Viewer>,
    container: OnceCell<raf::RafContainer>,
    thumbnails: OnceCell<ThumbnailStorage>,
}

impl RafFile {
    pub fn factory(reader: Box<dyn ReadAndSeek>) -> Box<dyn RawFile> {
        let viewer = Viewer::new(reader, 0);
        Box::new(RafFile {
            reader: viewer,
            container: OnceCell::new(),
            thumbnails: OnceCell::new(),
        })
    }
}

impl RawFileImpl for RafFile {
    fn identify_id(&self) -> TypeId {
        self.container();
        let container = self.container.get().unwrap();
        let model = container.get_model();
        MAKE_TO_ID_MAP
            .get(&model)
            .copied()
            .unwrap_or(fuji!(UNKNOWN))
    }

    fn container(&self) -> &dyn RawContainer {
        self.container.get_or_init(|| {
            // XXX we should be faillible here.
            let view = Viewer::create_view(&self.reader, 0).expect("Created view");
            let mut container = raf::RafContainer::new(view);
            container.load().expect("Raf container error");
            container
        })
    }

    fn thumbnails(&self) -> &ThumbnailStorage {
        self.thumbnails.get_or_init(|| {
            let mut thumbnails = Vec::new();
            self.container();
            let container = self.container.get().unwrap();
            if let Some(jpeg) = container.jpeg_preview() {
                let width = jpeg.width();
                let height = jpeg.height();
                let dim = std::cmp::max(width, height) as u32;

                thumbnails.push((
                    dim,
                    thumbnail::ThumbDesc {
                        width: width as u32,
                        height: height as u32,
                        data_type: DataType::Jpeg,
                        data: Data::Offset(DataOffset {
                            offset: container.jpeg_offset() as u64,
                            len: container.jpeg_len() as u64,
                        }),
                    },
                ));

                jpeg.exif()
                    .and_then(|exif| {
                        exif.directory(1).and_then(|dir| {
                            let offset =
                                dir.value::<u32>(exif::EXIF_TAG_JPEG_INTERCHANGE_FORMAT)?;
                            let len =
                                dir.value::<u32>(exif::EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LENGTH)?;
                            let bytes = exif.load_buffer8(offset as u64, len as u64);
                            let mut byte_slice = bytes.as_slice();
                            let mut decoder = jpeg_decoder::Decoder::new(&mut byte_slice);
                            decoder
                                .read_info()
                                .map_err(|e| {
                                    log::error!("JPEG decoding error {}", e);
                                })
                                .ok()?;
                            let (width, height) =
                                decoder.info().map(|info| (info.width, info.height))?;
                            let dim = std::cmp::max(width, height) as u32;
                            thumbnails.push((
                                dim,
                                thumbnail::ThumbDesc {
                                    width: width as u32,
                                    height: height as u32,
                                    data_type: DataType::Jpeg,
                                    data: Data::Bytes(bytes),
                                },
                            ));
                            Some(())
                        })
                    })
                    .or_else(|| {
                        log::error!("Failed to get thumbnail from Exif.");
                        None
                    });
            }
            ThumbnailStorage::with_thumbnails(thumbnails)
        })
    }

    fn ifd(&self, ifd_type: tiff::IfdType) -> Option<&tiff::Dir> {
        self.container();
        let raw_container = self.container.get().unwrap();
        match ifd_type {
            tiff::IfdType::Main => raw_container
                .jpeg_preview()
                .and_then(|jpeg| jpeg.exif())
                .and_then(|exif| exif.directory(0)),
            tiff::IfdType::Exif => raw_container
                .jpeg_preview()
                .and_then(|jpeg| jpeg.exif())
                .and_then(|exif| exif.exif_dir()),
            tiff::IfdType::MakerNote => raw_container
                .jpeg_preview()
                .and_then(|jpeg| jpeg.exif())
                .and_then(|exif| exif.mnote_dir()),
            _ => None,
        }
    }

    fn load_rawdata(&self, _skip_decompress: bool) -> Result<RawData> {
        self.container();
        let raw_container = self.container.get().unwrap();
        raw_container
            .meta_container()
            .ok_or(Error::NotFound)
            .and_then(|container| {
                // Dimensions are encapsulated into two u16 with an u32
                let raw_size = container
                    .value(raf::TAG_SENSOR_DIMENSION)
                    // Fujifilm HS10 doesn't have sensor dimension
                    .or_else(|| container.value(raf::TAG_IMG_HEIGHT_WIDTH))
                    .and_then(|v| Size::try_from(v).ok())
                    .ok_or_else(|| {
                        log::error!("Wrong RAF dimensions.");
                        Error::FormatError
                    })?;
                let active_area = container
                    .value(raf::TAG_IMG_TOP_LEFT)
                    .and_then(|topleft| Point::try_from(topleft).ok())
                    .and_then(|topleft| {
                        container
                            .value(raf::TAG_IMG_HEIGHT_WIDTH)
                            .and_then(|size| Size::try_from(size).ok())
                            .map(|size| Rect::new(topleft, size))
                    });

                let raw_props = container
                    .value(raf::TAG_RAW_INFO)
                    .and_then(|v| match v {
                        raf::Value::Int(props) => Some(props),
                        _ => {
                            log::error!("Wrong RAF raw props");
                            None
                        }
                    })
                    .ok_or(Error::FormatError)?;

                // It's an XTrans if there is a `TAG_CFA_PATTERN` and it has 36 elements.
                let pattern = container.value(raf::TAG_CFA_PATTERN).and_then(|v| match v {
                    raf::Value::Bytes(pattern) => Some(pattern),
                    _ => None,
                });

                let mosaic = if let Some(pattern) = pattern {
                    // In the RAF file the pattern is inverted.
                    Pattern::try_from(
                        pattern
                            .iter()
                            .rev()
                            .copied()
                            .collect::<Vec<u8>>()
                            .as_slice(),
                    )
                    .map_err(|_| Error::FormatError)?
                } else {
                    Pattern::Gbrg
                };

                log::debug!("RAF raw props {:x}", raw_props);
                // let layout = raw_props & 0xff000000 >> 24 >> 7;
                let compressed = ((raw_props & 0xff0000) >> 18 & 8) != 0;
                log::debug!("compressed {compressed}");

                let mut cfa_offset: u64 = 0;
                let mut cfa_len: u64 = 0;
                let mut bps: u16 = 12;

                if let Some(cfa_container) = raw_container.cfa_container() {
                    if let Some(dir) = cfa_container
                        .directory(0)
                        .and_then(|dir| dir.ifd_in_entry(cfa_container, raf::FUJI_TAG_RAW_SUBIFD))
                    {
                        cfa_offset = dir
                            .value::<u32>(raf::FUJI_TAG_RAW_OFFSET)
                            .map(|v| v + raw_container.cfa_offset())
                            .unwrap_or(0) as u64;
                        cfa_len = dir.value::<u32>(raf::FUJI_TAG_RAW_BYTE_LEN).unwrap_or(0) as u64;
                        bps = dir.value::<u32>(raf::FUJI_TAG_RAW_BPS).unwrap_or(0) as u16;
                    }
                } else {
                    cfa_offset = raw_container.cfa_offset() as u64 + 2048;
                    cfa_len = raw_container.cfa_len() as u64 - 2048;
                    bps = if compressed { 12 } else { 16 };
                }
                // Invalid value.
                if cfa_offset == 0 || cfa_len == 0 {
                    return Err(Error::NotFound);
                }

                let mut rawdata = if !compressed {
                    let unpacked = decompress::unpack(
                        raw_container,
                        raw_size.width,
                        raw_size.height,
                        bps,
                        tiff::Compression::None,
                        cfa_offset,
                        cfa_len as usize,
                    )
                    .map_err(|err| {
                        log::error!("RAF failed to unpack {}", err);
                        err
                    })?;
                    RawData::new16(
                        raw_size.width,
                        raw_size.height,
                        16,
                        DataType::Raw,
                        unpacked,
                        mosaic,
                    )
                } else {
                    // XXX decompress is not supported yet
                    let raw = raw_container.load_buffer8(cfa_offset, cfa_len);
                    RawData::new8(
                        raw_size.width,
                        raw_size.height,
                        bps,
                        DataType::CompressedRaw,
                        raw,
                        mosaic,
                    )
                };

                rawdata.set_active_area(active_area);

                Ok(rawdata)
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

impl RawFile for RafFile {
    fn type_(&self) -> Type {
        Type::Raf
    }
}

impl Dump for RafFile {
    #[cfg(feature = "dump")]
    fn write_dump<W: std::io::Write + ?Sized>(&self, out: &mut W, indent: u32) {
        dump_writeln!(out, indent, "<Fujifilm RAF File>");
        self.container();
        self.container.get().unwrap().write_dump(out, indent + 1);
        dump_writeln!(out, indent, "</Fujfilm RAF File>");
    }
}

dumpfile_impl!(RafFile);
