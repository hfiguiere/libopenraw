// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - olympus.rs
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

//! Olympus ORF support

pub mod decompress;
mod matrices;

use std::collections::HashMap;
use std::rc::Rc;

use once_cell::unsync::OnceCell;

use crate::bitmap::{Bitmap, Rect};
use crate::container;
use crate::container::RawContainer;
use crate::io::Viewer;
use crate::rawfile::{ReadAndSeek, ThumbnailStorage};
use crate::tiff;
use crate::tiff::IfdType;
use crate::tiff::{exif, Ifd};
use crate::{DataType, Dump, Error, RawData, RawFile, RawFileImpl, Result, Type, TypeId};

use decompress::decompress_olympus;
use matrices::MATRICES;

#[macro_export]
macro_rules! olympus {
    ($id:expr, $model:ident) => {
        (
            $id,
            TypeId(
                $crate::camera_ids::vendor::OLYMPUS,
                $crate::camera_ids::olympus::$model,
            ),
        )
    };
    ($model:ident) => {
        TypeId(
            $crate::camera_ids::vendor::OLYMPUS,
            $crate::camera_ids::olympus::$model,
        )
    };
}

lazy_static::lazy_static! {

    static ref MAKE_TO_ID_MAP: tiff::MakeToIdMap = HashMap::from([
        olympus!("E-1             ", E1),
        olympus!("E-10        "    , E10),
        olympus!("E-3             ", E3),
        olympus!("E-30            ", E30),
        olympus!("E-5             ", E5),
        olympus!("E-300           ", E300),
        olympus!("E-330           ", E330),
        olympus!("E-400           ", E400),
        olympus!("E-410           ", E410),
        olympus!("E-420           ", E420),
        olympus!("E-450           ", E450),
        olympus!("E-500           ", E500),
        olympus!("E-510           ", E510),
        olympus!("E-520           ", E520),
        olympus!("E-600           ", E600),
        olympus!("E-620           ", E620),
        olympus!("SP350"           , SP350),
        olympus!("SP500UZ"         , SP500UZ),
        olympus!("SP510UZ"         , SP510UZ),
        olympus!("SP550UZ                ", SP550UZ),
        olympus!("SP565UZ                ", SP565UZ),
        olympus!("SP570UZ                ", SP570UZ),
        olympus!("E-P1            ", EP1),
        olympus!("E-P2            ", EP2),
        olympus!("E-P3            ", EP3),
        olympus!("E-P5            ", EP5),
        olympus!("E-P7            ", EP7),
        olympus!("E-PL1           ", EPL1),
        olympus!("E-PL2           ", EPL2),
        olympus!("E-PL3           ", EPL3),
        olympus!("E-PL5           ", EPL5),
        olympus!("E-PL6           ", EPL6),
        olympus!("E-PL7           ", EPL7),
        olympus!("E-PL8           ", EPL8),
        olympus!("E-PL9           ", EPL9),
        olympus!("E-PL10          ", EPL10),
        olympus!("E-PM1           ", EPM1),
        olympus!("E-PM2           ", EPM2),
        olympus!("XZ-1            ", XZ1),
        olympus!("XZ-10           ", XZ10),
        olympus!("XZ-2            ", XZ2),
        olympus!("E-M5            ", EM5),
        olympus!("E-M5MarkII      ", EM5II),
        olympus!("E-M5MarkIII     ", EM5III),
        olympus!("E-M1            ", EM1),
        olympus!("E-M1MarkII      ", EM1II),
        olympus!("E-M1MarkIII     ", EM1III),
        olympus!("E-M1X           ", EM1X),
        olympus!("E-M10           ", EM10),
        olympus!("E-M10MarkII     ", EM10II),
        olympus!("E-M10 Mark III  ", EM10III),
        olympus!("E-M10MarkIIIS   ", EM10IIIS),
        olympus!("E-M10MarkIV     ", EM10IV),
        olympus!("OM-1            ", OM1),
        olympus!("OM-5            ", OM5),
        olympus!("STYLUS1         ", STYLUS1),
        olympus!("STYLUS1,1s      ", STYLUS1_1S),
        olympus!("PEN-F           ", PEN_F),
        olympus!("SH-2            ", SH2),
        olympus!("TG-4            ", TG4),
        olympus!("TG-5            ", TG5),
        olympus!("TG-6            ", TG6),
        olympus!("C5060WZ", C5060WZ),
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
    thumbnails: OnceCell<ThumbnailStorage>,
}

impl OrfFile {
    pub fn factory(reader: Box<dyn ReadAndSeek>) -> Box<dyn RawFile> {
        let viewer = Viewer::new(reader, 0);
        Box::new(OrfFile {
            reader: viewer,
            type_id: OnceCell::new(),
            container: OnceCell::new(),
            thumbnails: OnceCell::new(),
        })
    }

    /// Return the CFA dir
    fn cfa_dir(&self) -> Option<&tiff::Dir> {
        self.container();
        self.container.get().unwrap().directory(0)
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
            tiff::identify_with_exif(container, &MAKE_TO_ID_MAP).unwrap_or(olympus!(UNKNOWN))
        })
    }

    /// Return a lazily loaded `tiff::Container`
    fn container(&self) -> &dyn RawContainer {
        self.container.get_or_init(|| {
            // XXX we should be faillible here.
            let view = Viewer::create_view(&self.reader, 0).expect("Created view");
            let mut container = tiff::Container::new(view, vec![IfdType::Main], self.type_());
            container
                .load(Some(OrfFile::is_magic_header))
                .expect("Olympus IFD container error");
            container
        })
    }

    fn thumbnails(&self) -> &ThumbnailStorage {
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

            ThumbnailStorage::with_thumbnails(thumbnails)
        })
    }

    fn ifd(&self, ifd_type: tiff::IfdType) -> Option<&tiff::Dir> {
        self.container();
        let container = self.container.get().unwrap();
        match ifd_type {
            tiff::IfdType::Main => container.directory(0),
            tiff::IfdType::Raw => self.cfa_dir(),
            tiff::IfdType::Exif => container.exif_dir(),
            tiff::IfdType::MakerNote => container.mnote_dir(),
            _ => None,
        }
    }

    fn load_rawdata(&self, skip_decompress: bool) -> Result<RawData> {
        self.ifd(IfdType::Raw)
            .ok_or(Error::NotFound)
            .and_then(|cfa| {
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
                    tiff::Compression::Olympus => {
                        if !skip_decompress {
                            self.decompress(data)
                        } else {
                            data
                        }
                    }
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
    fn write_dump<W: std::io::Write + ?Sized>(&self, out: &mut W, indent: u32) {
        dump_writeln!(out, indent, "<Olympus ORF File>");
        {
            let indent = indent + 1;
            self.container();
            self.container.get().unwrap().write_dump(out, indent);
        }
        dump_writeln!(out, indent, "</Olympus ORF File>");
    }
}

dumpfile_impl!(OrfFile);
