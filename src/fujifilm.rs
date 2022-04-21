/*
 * libopenraw - fuji.rs
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

//! Fujifilm RAF format

mod matrices;
mod raf;

use std::collections::HashMap;
use std::convert::TryFrom;
use std::rc::Rc;

use once_cell::unsync::OnceCell;

use crate::bitmap::{Point, Rect, Size};
use crate::camera_ids::{fujifilm, vendor};
use crate::container::GenericContainer;
use crate::decompress;
use crate::io::Viewer;
use crate::rawfile::ReadAndSeek;
use crate::thumbnail;
use crate::thumbnail::{Data, DataOffset};
use crate::tiff;
use crate::tiff::{exif, Ifd};
use crate::{DataType, Dump, Error, RawData, RawFile, RawFileImpl, Result, Type, TypeId};

use matrices::MATRICES;

pub(crate) const RAF_MAGIC: &[u8] = b"FUJIFILMCCD-RAW ";

lazy_static::lazy_static! {
    /// Make to TypeId map for RAF files.
    static ref MAKE_TO_ID_MAP: tiff::MakeToIdMap = HashMap::from([
        ( "GFX 50S", TypeId(vendor::FUJIFILM, fujifilm::GFX50S)),
        ( "GFX50S II", TypeId(vendor::FUJIFILM, fujifilm::GFX50S_II)),
        ( "GFX 50R", TypeId(vendor::FUJIFILM, fujifilm::GFX50R)),
        ( "GFX 100", TypeId(vendor::FUJIFILM, fujifilm::GFX100)),
        ( "GFX100S", TypeId(vendor::FUJIFILM, fujifilm::GFX100S)),
        ( "FinePix F550EXR", TypeId(vendor::FUJIFILM, fujifilm::F550EXR)),
        ( "FinePix F700  ", TypeId(vendor::FUJIFILM, fujifilm::F700)),
        ( "FinePix F810   ", TypeId(vendor::FUJIFILM, fujifilm::F810)),
        ( "FinePix E900   ", TypeId(vendor::FUJIFILM, fujifilm::E900)),
        ( "FinePixS2Pro", TypeId(vendor::FUJIFILM, fujifilm::S2PRO)),
        ( "FinePix S3Pro  ", TypeId(vendor::FUJIFILM, fujifilm::S3PRO)),
        ( "FinePix S5Pro  ", TypeId(vendor::FUJIFILM, fujifilm::S5PRO)),
        ( "FinePix S5000 ", TypeId(vendor::FUJIFILM, fujifilm::S5000)),
        ( "FinePix S5600  ", TypeId(vendor::FUJIFILM, fujifilm::S5600)),
        ( "FinePix S9500  ", TypeId(vendor::FUJIFILM, fujifilm::S9500)),
        ( "FinePix S6500fd", TypeId(vendor::FUJIFILM, fujifilm::S6500FD)),
        ( "FinePix HS10 HS11", TypeId(vendor::FUJIFILM, fujifilm::HS10)),
        ( "FinePix HS30EXR", TypeId(vendor::FUJIFILM, fujifilm::HS30EXR)),
        ( "FinePix HS33EXR", TypeId(vendor::FUJIFILM, fujifilm::HS33EXR)),
        ( "FinePix S100FS ", TypeId(vendor::FUJIFILM, fujifilm::S100FS)),
        ( "FinePix S200EXR", TypeId(vendor::FUJIFILM, fujifilm::S200EXR)),
        ( "FinePix X100", TypeId(vendor::FUJIFILM, fujifilm::X100)),
        ( "X10", TypeId(vendor::FUJIFILM, fujifilm::X10)),
        ( "X20", TypeId(vendor::FUJIFILM, fujifilm::X20)),
        ( "X30", TypeId(vendor::FUJIFILM, fujifilm::X30)),
        ( "X70", TypeId(vendor::FUJIFILM, fujifilm::X70)),
        ( "X-Pro1", TypeId(vendor::FUJIFILM, fujifilm::XPRO1)),
        ( "X-Pro2", TypeId(vendor::FUJIFILM, fujifilm::XPRO2)),
        ( "X-Pro3", TypeId(vendor::FUJIFILM, fujifilm::XPRO3)),
        ( "X-S1", TypeId(vendor::FUJIFILM, fujifilm::XS1)),
        ( "X-S10", TypeId(vendor::FUJIFILM, fujifilm::XS10)),
        ( "X-A1", TypeId(vendor::FUJIFILM, fujifilm::XA1)),
        ( "X-A2", TypeId(vendor::FUJIFILM, fujifilm::XA2)),
        ( "X-A3", TypeId(vendor::FUJIFILM, fujifilm::XA3)),
        ( "X-A5", TypeId(vendor::FUJIFILM, fujifilm::XA5)),
        ( "X-A7", TypeId(vendor::FUJIFILM, fujifilm::XA7)),
        ( "XQ1", TypeId(vendor::FUJIFILM, fujifilm::XQ1)),
        ( "XQ2", TypeId(vendor::FUJIFILM, fujifilm::XQ2)),
        ( "X-E1", TypeId(vendor::FUJIFILM, fujifilm::XE1)),
        ( "X-E2", TypeId(vendor::FUJIFILM, fujifilm::XE2)),
        ( "X-E2S", TypeId(vendor::FUJIFILM, fujifilm::XE2S)),
        ( "X-E3", TypeId(vendor::FUJIFILM, fujifilm::XE3)),
        ( "X-E4", TypeId(vendor::FUJIFILM, fujifilm::XE4)),
        ( "X-M1", TypeId(vendor::FUJIFILM, fujifilm::XM1)),
        ( "X-T1", TypeId(vendor::FUJIFILM, fujifilm::XT1)),
        ( "X-T10", TypeId(vendor::FUJIFILM, fujifilm::XT10)),
        ( "X-T100", TypeId(vendor::FUJIFILM, fujifilm::XT100)),
        ( "X-T2", TypeId(vendor::FUJIFILM, fujifilm::XT2)),
        ( "X-T20", TypeId(vendor::FUJIFILM, fujifilm::XT20)),
        ( "X-T200", TypeId(vendor::FUJIFILM, fujifilm::XT200)),
        ( "X-T3", TypeId(vendor::FUJIFILM, fujifilm::XT3)),
        ( "X-T30", TypeId(vendor::FUJIFILM, fujifilm::XT30)),
        ( "X-T30 II", TypeId(vendor::FUJIFILM, fujifilm::XT30_II)),
        ( "X-T4", TypeId(vendor::FUJIFILM, fujifilm::XT4)),
        ( "XF1", TypeId(vendor::FUJIFILM, fujifilm::XF1)),
        ( "XF10", TypeId(vendor::FUJIFILM, fujifilm::XF10)),
        ( "X100S", TypeId(vendor::FUJIFILM, fujifilm::X100S)),
        ( "X100T", TypeId(vendor::FUJIFILM, fujifilm::X100T)),
        ( "X100F", TypeId(vendor::FUJIFILM, fujifilm::X100F)),
        ( "X100V", TypeId(vendor::FUJIFILM, fujifilm::X100V)),
        ( "X-H1", TypeId(vendor::FUJIFILM, fujifilm::XH1)),
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
        (0x1047, "GrainEffect"),
        (0x1048, "ColorChromeEffect"),
        (0x1049, "BWAdjustment"),
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
    thumbnails: OnceCell<Vec<(u32, thumbnail::ThumbDesc)>>,
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

    fn is_xtrans() -> bool {
        false
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
            .unwrap_or(TypeId(vendor::FUJIFILM, 0))
    }

    fn container(&self) -> &dyn GenericContainer {
        self.container.get_or_init(|| {
            // XXX we should be faillible here.
            let view = Viewer::create_view(&self.reader, 0).expect("Created view");
            let mut container = raf::RafContainer::new(view);
            container.load().expect("Raf container error");
            container
        })
    }

    fn thumbnails(&self) -> &Vec<(u32, thumbnail::ThumbDesc)> {
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

            thumbnails
        })
    }

    fn ifd(&self, ifd_type: tiff::Type) -> Option<Rc<tiff::Dir>> {
        self.container();
        let raw_container = self.container.get().unwrap();
        match ifd_type {
            tiff::Type::Main => raw_container
                .jpeg_preview()
                .and_then(|jpeg| jpeg.exif())
                .and_then(|exif| exif.directory(0)),
            tiff::Type::Exif => raw_container
                .jpeg_preview()
                .and_then(|jpeg| jpeg.exif())
                .and_then(|exif| exif.exif_dir()),
            tiff::Type::MakerNote => raw_container
                .jpeg_preview()
                .and_then(|jpeg| jpeg.exif())
                .and_then(|exif| exif.mnote_dir()),
            _ => None,
        }
    }

    fn load_rawdata(&self) -> Result<RawData> {
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
                log::debug!("RAF raw props {:x}", raw_props);
                // let layout = raw_props & 0xff000000 >> 24 >> 7;
                let compressed = ((raw_props & 0xff0000) >> 18 & 8) != 0;

                // XXX the cfa is actually stored in a TIFF and 2048
                // seems to be the value of the next_ifd.
                // XXX use a tiff container instead.
                let cfa_offset = raw_container.cfa_offset() as u64 + 2048;
                let cfa_len = raw_container.cfa_len() as u64 - 2048;
                let mut rawdata = if !compressed {
                    let unpacked = decompress::unpack(
                        raw_container,
                        raw_size.width,
                        raw_size.height,
                        12,
                        tiff::Compression::None,
                        cfa_offset,
                        cfa_len as usize,
                    )
                    .map_err(|err| {
                        log::error!("RAF failed to unpack {}", err);
                        err
                    })?;
                    RawData::new16(raw_size.width, raw_size.height, 16, DataType::Raw, unpacked)
                } else {
                    // XXX decompress is not supported yet
                    let raw = raw_container.load_buffer8(cfa_offset, cfa_len);
                    RawData::new8(
                        raw_size.width,
                        raw_size.height,
                        16,
                        DataType::CompressedRaw,
                        raw,
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
    fn print_dump(&self, indent: u32) {
        dump_println!(indent, "<Fujifilm RAF File>");
        self.container().print_dump(indent + 1);
        dump_println!(indent, "</Fujfilm RAF File>");
    }
}
