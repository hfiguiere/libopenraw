// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - olympus.rs
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

//! Olympus ORF support

pub mod decompress;
mod matrices;

use std::collections::HashMap;
use std::rc::Rc;

use once_cell::unsync::OnceCell;

use crate::bitmap::{Bitmap, Rect};
use crate::camera_ids;
use crate::camera_ids::vendor;
use crate::container;
use crate::container::RawContainer;
use crate::io::Viewer;
use crate::rawfile::ReadAndSeek;
use crate::thumbnail;
use crate::tiff;
use crate::tiff::IfdType;
use crate::tiff::{exif, Ifd};
use crate::{DataType, Dump, Error, RawData, RawFile, RawFileImpl, Result, Type, TypeId};

use decompress::decompress_olympus;
use matrices::MATRICES;

lazy_static::lazy_static! {

    static ref MAKE_TO_ID_MAP: tiff::MakeToIdMap = HashMap::from([
        ( "E-1             ", TypeId(vendor::OLYMPUS, camera_ids::olympus::E1) ),
        ( "E-10        "    , TypeId(vendor::OLYMPUS, camera_ids::olympus::E10) ),
        ( "E-3             ", TypeId(vendor::OLYMPUS, camera_ids::olympus::E3) ),
        ( "E-5             ", TypeId(vendor::OLYMPUS, camera_ids::olympus::E5) ),
        ( "E-300           ", TypeId(vendor::OLYMPUS, camera_ids::olympus::E300) ),
        ( "E-330           ", TypeId(vendor::OLYMPUS, camera_ids::olympus::E330) ),
        ( "E-400           ", TypeId(vendor::OLYMPUS, camera_ids::olympus::E400) ),
        ( "E-410           ", TypeId(vendor::OLYMPUS, camera_ids::olympus::E410) ),
        ( "E-500           ", TypeId(vendor::OLYMPUS, camera_ids::olympus::E500) ),
        ( "E-510           ", TypeId(vendor::OLYMPUS, camera_ids::olympus::E510) ),
        ( "E-620           ", TypeId(vendor::OLYMPUS, camera_ids::olympus::E620) ),
        ( "SP350"           , TypeId(vendor::OLYMPUS, camera_ids::olympus::SP350) ),
        ( "SP500UZ"         , TypeId(vendor::OLYMPUS, camera_ids::olympus::SP500UZ) ),
        ( "SP510UZ"         , TypeId(vendor::OLYMPUS, camera_ids::olympus::SP510UZ) ),
        ( "SP550UZ                ", TypeId(vendor::OLYMPUS, camera_ids::olympus::SP550UZ) ),
        ( "SP565UZ                ", TypeId(vendor::OLYMPUS, camera_ids::olympus::SP565UZ) ),
        ( "E-P1            ", TypeId(vendor::OLYMPUS, camera_ids::olympus::EP1) ),
        ( "E-P2            ", TypeId(vendor::OLYMPUS, camera_ids::olympus::EP2) ),
        ( "E-P3            ", TypeId(vendor::OLYMPUS, camera_ids::olympus::EP3) ),
        ( "E-PL1           ", TypeId(vendor::OLYMPUS, camera_ids::olympus::EPL1) ),
        ( "E-PL2           ", TypeId(vendor::OLYMPUS, camera_ids::olympus::EPL2) ),
        ( "E-PL3           ", TypeId(vendor::OLYMPUS, camera_ids::olympus::EPL3) ),
        ( "E-PL5           ", TypeId(vendor::OLYMPUS, camera_ids::olympus::EPL5) ),
        ( "E-PL6           ", TypeId(vendor::OLYMPUS, camera_ids::olympus::EPL6) ),
        ( "E-PL7           ", TypeId(vendor::OLYMPUS, camera_ids::olympus::EPL7) ),
        ( "E-PL8           ", TypeId(vendor::OLYMPUS, camera_ids::olympus::EPL8) ),
        ( "E-PL9           ", TypeId(vendor::OLYMPUS, camera_ids::olympus::EPL9) ),
        ( "E-PL10          ", TypeId(vendor::OLYMPUS, camera_ids::olympus::EPL10) ),
        ( "E-PM1           ", TypeId(vendor::OLYMPUS, camera_ids::olympus::EPM1) ),
        ( "E-PM2           ", TypeId(vendor::OLYMPUS, camera_ids::olympus::EPM2) ),
        ( "XZ-1            ", TypeId(vendor::OLYMPUS, camera_ids::olympus::XZ1) ),
        ( "XZ-10           ", TypeId(vendor::OLYMPUS, camera_ids::olympus::XZ10) ),
        ( "XZ-2            ", TypeId(vendor::OLYMPUS, camera_ids::olympus::XZ2) ),
        ( "E-M5            ", TypeId(vendor::OLYMPUS, camera_ids::olympus::EM5) ),
        ( "E-M5MarkII      ", TypeId(vendor::OLYMPUS, camera_ids::olympus::EM5II) ),
        ( "E-M5MarkIII     ", TypeId(vendor::OLYMPUS, camera_ids::olympus::EM5III) ),
        ( "E-M1            ", TypeId(vendor::OLYMPUS, camera_ids::olympus::EM1) ),
        ( "E-M1MarkII      ", TypeId(vendor::OLYMPUS, camera_ids::olympus::EM1II) ),
        ( "E-M1MarkIII     ", TypeId(vendor::OLYMPUS, camera_ids::olympus::EM1III) ),
        ( "E-M1X           ", TypeId(vendor::OLYMPUS, camera_ids::olympus::EM1X) ),
        ( "E-M10           ", TypeId(vendor::OLYMPUS, camera_ids::olympus::EM10) ),
        ( "E-M10MarkII     ", TypeId(vendor::OLYMPUS, camera_ids::olympus::EM10II) ),
        ( "E-M10 Mark III  ", TypeId(vendor::OLYMPUS, camera_ids::olympus::EM10III) ),
        ( "E-M10MarkIIIS   ", TypeId(vendor::OLYMPUS, camera_ids::olympus::EM10IIIS) ),
        ( "E-M10MarkIV     ", TypeId(vendor::OLYMPUS, camera_ids::olympus::EM10IV) ),
        ( "OM-1            ", TypeId(vendor::OLYMPUS, camera_ids::olympus::OM1) ),
        ( "STYLUS1         ", TypeId(vendor::OLYMPUS, camera_ids::olympus::STYLUS1) ),
        ( "STYLUS1,1s      ", TypeId(vendor::OLYMPUS, camera_ids::olympus::STYLUS1_1S) ),
        ( "PEN-F           ", TypeId(vendor::OLYMPUS, camera_ids::olympus::PEN_F) ),
        ( "SH-2            ", TypeId(vendor::OLYMPUS, camera_ids::olympus::SH2) ),
        ( "TG-4            ", TypeId(vendor::OLYMPUS, camera_ids::olympus::TG4) ),
        ( "TG-5            ", TypeId(vendor::OLYMPUS, camera_ids::olympus::TG5) ),
        ( "TG-6            ", TypeId(vendor::OLYMPUS, camera_ids::olympus::TG6) ),
    ]);

    /// Olympus MakerNote tag names
    pub static ref MNOTE_TAG_NAMES: HashMap<u16, &'static str> = HashMap::from([
        (0x0, "MakerNoteVersion"),
        (0x1, "MinoltaCameraSettingsOld"),
        (0x3, "MinoltaCameraSettings"),
        (0x40, "CompressedImageSize"),
        (0x81, "PreviewImageData"),
        (0x88, "PreviewImageStart"),
        (0x89, "PreviewImageLength"),
        (0x100, "ThumbnailImage"),
        (0x104, "BodyFirmwareVersion"),
        (0x200, "SpecialMode"),
        (0x201, "Quality"),
        (0x202, "Macro"),
        (0x203, "BWMode"),
        (0x204, "DigitalZoom"),
        (0x205, "FocalPlaneDiagonal"),
        (0x206, "LensDistortionParams"),
        (0x207, "CameraType"),
        (0x208, "TextInfo"),
        (0x209, "CameraID"),
        (0x20b, "EpsonImageWidth"),
        (0x20c, "EpsonImageHeight"),
        (0x20d, "EpsonSoftware"),
        (0x280, "PreviewImage"),
        (0x300, "PreCaptureFrames"),
        (0x301, "WhiteBoard"),
        (0x302, "OneTouchWB"),
        (0x303, "WhiteBalanceBracket"),
        (0x304, "WhiteBalanceBias"),
        (0x400, "SensorArea"),
        (0x401, "BlackLevel"),
        (0x403, "SceneMode"),
        (0x404, "SerialNumber"),
        (0x405, "Firmware"),
        (0xe00, "PrintIM"),
        (0xf00, "DataDump"),
        (0xf01, "DataDump2"),
        (0xf04, "ZoomedPreviewStart"),
        (0xf05, "ZoomedPreviewLength"),
        (0xf06, "ZoomedPreviewSize"),
        (0x1000, "ShutterSpeedValue"),
        (0x1001, "ISOValue"),
        (0x1002, "ApertureValue"),
        (0x1003, "BrightnessValue"),
        (0x1004, "FlashMode"),
        (0x1005, "FlashDevice"),
        (0x1006, "ExposureCompensation"),
        (0x1007, "SensorTemperature"),
        (0x1008, "LensTemperature"),
        (0x1009, "LightCondition"),
        (0x100a, "FocusRange"),
        (0x100b, "FocusMode"),
        (0x100c, "ManualFocusDistance"),
        (0x100d, "ZoomStepCount"),
        (0x100e, "FocusStepCount"),
        (0x100f, "Sharpness"),
        (0x1010, "FlashChargeLevel"),
        (0x1011, "ColorMatrix"),
        (0x1012, "BlackLevel"),
        (0x1013, "ColorTemperatureBG"),
        (0x1014, "ColorTemperatureRG"),
        (0x1015, "WBMode"),
        (0x1017, "RedBalance"),
        (0x1018, "BlueBalance"),
        (0x1019, "ColorMatrixNumber"),
        (0x101a, "SerialNumber"),
        (0x101b, "ExternalFlashAE1_0"),
        (0x101c, "ExternalFlashAE2_0"),
        (0x101d, "InternalFlashAE1_0"),
        (0x101e, "InternalFlashAE2_0"),
        (0x101f, "ExternalFlashAE1"),
        (0x1020, "ExternalFlashAE2"),
        (0x1021, "InternalFlashAE1"),
        (0x1022, "InternalFlashAE2"),
        (0x1023, "FlashExposureComp"),
        (0x1024, "InternalFlashTable"),
        (0x1025, "ExternalFlashGValue"),
        (0x1026, "ExternalFlashBounce"),
        (0x1027, "ExternalFlashZoom"),
        (0x1028, "ExternalFlashMode"),
        (0x1029, "Contrast"),
        (0x102a, "SharpnessFactor"),
        (0x102b, "ColorControl"),
        (0x102c, "ValidBits"),
        (0x102d, "CoringFilter"),
        (0x102e, "OlympusImageWidth"),
        (0x102f, "OlympusImageHeight"),
        (0x1030, "SceneDetect"),
        (0x1031, "SceneArea"),
        (0x1033, "SceneDetectData"),
        (0x1034, "CompressionRatio"),
        (0x1035, "PreviewImageValid"),
        (0x1036, "PreviewImageStart"),
        (0x1037, "PreviewImageLength"),
        (0x1038, "AFResult"),
        (0x1039, "CCDScanMode"),
        (0x103a, "NoiseReduction"),
        (0x103b, "FocusStepInfinity"),
        (0x103c, "FocusStepNear"),
        (0x103d, "LightValueCenter"),
        (0x103e, "LightValuePeriphery"),
        (0x103f, "FieldCount"),
        (0x2010, "Equipment"),
        (0x2020, "CameraSettings"),
        (0x2030, "RawDevelopment"),
        (0x2031, "RawDev2"),
        (0x2040, "ImageProcessing"),
        (0x2050, "FocusInfo"),
        (0x2100, "Olympus2100"),
        (0x2200, "Olympus2200"),
        (0x2300, "Olympus2300"),
        (0x2400, "Olympus2400"),
        (0x2500, "Olympus2500"),
        (0x2600, "Olympus2600"),
        (0x2700, "Olympus2700"),
        (0x2800, "Olympus2800"),
        (0x2900, "Olympus2900"),
        (0x3000, "RawInfo"),
        (0x4000, "MainInfo"),
        (0x5000, "UnknownInfo"),
    ]);
}

pub(crate) struct OrfFile {
    reader: Rc<Viewer>,
    type_id: OnceCell<TypeId>,
    container: OnceCell<tiff::Container>,
    thumbnails: OnceCell<Vec<(u32, thumbnail::ThumbDesc)>>,
    cfa: OnceCell<Option<Rc<tiff::Dir>>>,
}

impl OrfFile {
    pub fn factory(reader: Box<dyn ReadAndSeek>) -> Box<dyn RawFile> {
        let viewer = Viewer::new(reader, 0);
        Box::new(OrfFile {
            reader: viewer,
            type_id: OnceCell::new(),
            container: OnceCell::new(),
            thumbnails: OnceCell::new(),
            cfa: OnceCell::new(),
        })
    }

    /// Return the CFA dir
    fn cfa_dir(&self) -> Option<&Rc<tiff::Dir>> {
        self.cfa
            .get_or_init(|| {
                self.container();
                self.container.get().unwrap().directory(0)
            })
            .as_ref()
    }

    /// Will identify the magic header for Olympus and return the endian
    /// Olympus slightly change over the standard TIFF header.
    fn is_magic_header(buf: &[u8]) -> Result<container::Endian> {
        if buf.len() < 4 {
            log::error!("Olympus magic header buffer too small: {} bytes", buf.len());
            return Err(Error::BufferTooSmall);
        }
        // the subtype is 'O' or 'S' and doesn't seem to be used.
        // 'S' seems to be for the C5060WZ
        if &buf[0..3] == b"IIR" && (buf[3] == b'O' || buf[3] == b'S') {
            Ok(container::Endian::Little)
        } else if &buf[0..2] == b"MM" && buf[3] == b'R' && (buf[2] == b'O' || buf[2] == b'S') {
            Ok(container::Endian::Big)
        } else {
            log::error!("Incorrect Olympus IFD magic: {:?}", buf);
            Err(Error::FormatError)
        }
    }

    /// Decompress the Olympus RawData. Could be unpack.
    fn decompress(&self, mut data: RawData) -> RawData {
        let width = data.width();
        let height = data.height();

        if let Some(d) = data
            .data8()
            .or_else(|| data.data16_as_u8())
            .and_then(|data8| {
                if data8.len() == (height * ((width * 12 / 8) + ((width + 2) / 10))) as usize {
                    let mut buf = data8;
                    crate::decompress::unpack_from_reader(
                        &mut buf,
                        width,
                        height,
                        12,
                        tiff::Compression::Olympus,
                        data8.len(),
                    )
                    .ok()
                } else {
                    // Olympus decompression
                    decompress_olympus(data8, width as usize, height as usize)
                        .map_err(|err| {
                            log::error!("Decompression failed {}", err);
                            err
                        })
                        .ok()
                }
            })
        {
            data.set_data16(d);
            data.set_compression(tiff::Compression::None);
            data.set_data_type(DataType::Raw);
        }
        data
    }
}

impl RawFileImpl for OrfFile {
    fn identify_id(&self) -> TypeId {
        *self.type_id.get_or_init(|| {
            self.container();
            let container = self.container.get().unwrap();
            tiff::identify_with_exif(container, &MAKE_TO_ID_MAP)
                .unwrap_or(TypeId(vendor::OLYMPUS, 0))
        })
    }

    /// Return a lazily loaded `tiff::Container`
    fn container(&self) -> &dyn RawContainer {
        self.container.get_or_init(|| {
            // XXX we should be faillible here.
            let view = Viewer::create_view(&self.reader, 0).expect("Created view");
            let mut container = tiff::Container::new(view, vec![], self.type_());
            container
                .load(Some(OrfFile::is_magic_header))
                .expect("Olympus IFD container error");
            container
        })
    }

    fn thumbnails(&self) -> &Vec<(u32, thumbnail::ThumbDesc)> {
        self.thumbnails.get_or_init(|| {
            self.container();
            let container = self.container.get().unwrap();
            let mut thumbnails = tiff::tiff_thumbnails(container);
            self.maker_note_ifd().and_then(|mnote| {
                mnote.entry(exif::ORF_TAG_THUMBNAIL_IMAGE).map(|e| {
                    container.add_thumbnail_from_entry(e, mnote.mnote_offset, &mut thumbnails)
                });
                mnote
                    .ifd_in_entry(container, exif::ORF_TAG_CAMERA_SETTINGS)
                    .map(|dir| {
                        if dir
                            .value::<u32>(exif::ORF_TAG_CS_PREVIEW_IMAGE_VALID)
                            .unwrap_or(0)
                            != 0
                        {
                            let start = dir
                                .value::<u32>(exif::ORF_TAG_CS_PREVIEW_IMAGE_START)
                                .unwrap_or(0)
                                + mnote.mnote_offset;
                            let len = dir
                                .value::<u32>(exif::ORF_TAG_CS_PREVIEW_IMAGE_LENGTH)
                                .unwrap_or(0);
                            if start != 0 && len != 0 {
                                let _ = container.add_thumbnail_from_stream(
                                    start,
                                    len,
                                    &mut thumbnails,
                                );
                            } else {
                                log::error!(
                                    "ORF thumbnail valid but invalid start {} or len {}",
                                    start,
                                    len
                                );
                            }
                        }
                    })
            });

            thumbnails
        })
    }

    fn ifd(&self, ifd_type: tiff::IfdType) -> Option<Rc<tiff::Dir>> {
        match ifd_type {
            tiff::IfdType::Raw => self.cfa_dir().cloned(),
            tiff::IfdType::Exif => {
                self.container();
                self.container.get().unwrap().exif_dir()
            }
            tiff::IfdType::MakerNote => {
                self.container();
                self.container.get().unwrap().mnote_dir()
            }
            _ => None,
        }
    }

    fn load_rawdata(&self) -> Result<RawData> {
        self.ifd(IfdType::Raw)
            .ok_or(Error::NotFound)
            .and_then(|ref cfa| {
                self.container();
                tiff::tiff_get_rawdata(self.container.get().unwrap(), cfa, self.type_())
            })
            .map(|mut data| {
                let width = data.width();
                let height = data.height();
                let compression = if data.data_size() < width as usize * height as usize * 2 {
                    data.set_compression(tiff::Compression::Olympus);
                    data.set_data_type(DataType::CompressedRaw);
                    tiff::Compression::Olympus
                } else {
                    data.compression()
                };
                let mut data = match compression {
                    tiff::Compression::Olympus => self.decompress(data),
                    _ => data,
                };
                data.set_bpc(12);
                data.set_active_area(Some(Rect {
                    x: 0,
                    y: 0,
                    width,
                    height,
                }));
                data.set_white((1 << 12) - 1);
                data
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

impl RawFile for OrfFile {
    fn type_(&self) -> Type {
        Type::Orf
    }
}

impl Dump for OrfFile {
    #[cfg(feature = "dump")]
    fn print_dump(&self, indent: u32) {
        dump_println!(indent, "<Olympus ORF File>");
        {
            let indent = indent + 1;
            self.container().print_dump(indent);
        }
        dump_println!(indent, "</Olympus ORF File>");
    }
}
