// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - pentax.rs
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

//! Pentax camera support.

use std::collections::HashMap;
use std::rc::Rc;

use once_cell::unsync::OnceCell;

use crate::bitmap::{Point, Rect, Size};
use crate::camera_ids::{pentax, ricoh, vendor};
use crate::colour::BuiltinMatrix;
use crate::container::RawContainer;
use crate::io::Viewer;
use crate::rawfile::ReadAndSeek;
use crate::thumbnail;
use crate::tiff;
use crate::tiff::{exif, Dir, Ifd};
use crate::{Dump, Error, RawData, RawFile, RawFileImpl, Result, Type, TypeId};

lazy_static::lazy_static! {
    pub static ref MNOTE_TAG_NAMES: HashMap<u16, &'static str> = HashMap::from([
        (0x0, "PentaxVersion"),
        (0x1, "PentaxModelType"),
        (0x2, "PreviewImageSize"),
        (0x3, "PreviewImageLength"),
        (0x4, "PreviewImageStart"),
        (0x5, "PentaxModelID"),
        (0x6, "Date"),
        (0x7, "Time"),
        (0x8, "Quality"),
        (0x9, "PentaxImageSize"),
        (0xb, "PictureMode"),
        (0xc, "FlashMode"),
        (0xd, "FocusMode"),
        (0xe, "AFPointSelected"),
        (0xf, "AFPointsInFocus"),
        (0x10, "FocusPosition"),
        (0x12, "ExposureTime"),
        (0x13, "FNumber"),
        (0x14, "ISO"),
        (0x15, "LightReading"),
        (0x16, "ExposureCompensation"),
        (0x17, "MeteringMode"),
        (0x18, "AutoBracketing"),
        (0x19, "WhiteBalance"),
        (0x1a, "WhiteBalanceMode"),
        (0x1b, "BlueBalance"),
        (0x1c, "RedBalance"),
        (0x1d, "FocalLength"),
        (0x1e, "DigitalZoom"),
        (0x1f, "Saturation"),
        (0x20, "Contrast"),
        (0x21, "Sharpness"),
        (0x22, "WorldTimeLocation"),
        (0x23, "HometownCity"),
        (0x24, "DestinationCity"),
        (0x25, "HometownDST"),
        (0x26, "DestinationDST"),
        (0x27, "DSPFirmwareVersion"),
        (0x28, "CPUFirmwareVersion"),
        (0x29, "FrameNumber"),
        (0x2d, "EffectiveLV"),
        (0x32, "ImageEditing"),
        (0x33, "PictureMode"),
        (0x34, "DriveMode"),
        (0x35, "SensorSize"),
        (0x37, "ColorSpace"),
        (0x38, "ImageAreaOffset"),
        (0x39, "RawImageSize"),
        (0x3c, "AFPointsInFocus"),
        (0x3d, "DataScaling"),
        (0x3e, "PreviewImageBorders"),
        (0x3f, "LensRec"),
        (0x40, "SensitivityAdjust"),
        (0x41, "ImageEditCount"),
        (0x47, "CameraTemperature"),
        (0x48, "AELock"),
        (0x49, "NoiseReduction"),
        (0x4d, "FlashExposureComp"),
        (0x4f, "ImageTone"),
        (0x50, "ColorTemperature"),
        (0x53, "ColorTempDaylight"),
        (0x54, "ColorTempShade"),
        (0x55, "ColorTempCloudy"),
        (0x56, "ColorTempTungsten"),
        (0x57, "ColorTempFluorescentD"),
        (0x58, "ColorTempFluorescentN"),
        (0x59, "ColorTempFluorescentW"),
        (0x5a, "ColorTempFlash"),
        (0x5c, "ShakeReductionInfo"),
        (0x5d, "ShutterCount"),
        (0x60, "FaceInfo"),
        (0x62, "RawDevelopmentProcess"),
        (0x67, "Hue"),
        (0x68, "AWBInfo"),
        (0x69, "DynamicRangeExpansion"),
        (0x6b, "TimeInfo"),
        (0x6c, "HighLowKeyAdj"),
        (0x6d, "ContrastHighlight"),
        (0x6e, "ContrastShadow"),
        (0x6f, "ContrastHighlightShadowAdj"),
        (0x70, "FineSharpness"),
        (0x71, "HighISONoiseReduction"),
        (0x72, "AFAdjustment"),
        (0x73, "MonochromeFilterEffect"),
        (0x74, "MonochromeToning"),
        (0x76, "FaceDetect"),
        (0x77, "FaceDetectFrameSize"),
        (0x79, "ShadowCorrection"),
        (0x7a, "ISOAutoParameters"),
        (0x7b, "CrossProcess"),
        (0x7d, "LensCorr"),
        (0x7e, "WhiteLevel"),
        (0x7f, "BleachBypassToning"),
        (0x80, "AspectRatio"),
        (0x82, "BlurControl"),
        (0x85, "HDR"),
        (0x87, "ShutterType"),
        (0x88, "NeutralDensityFilter"),
        (0x8b, "ISO"),
        (0x92, "IntervalShooting"),
        (0x95, "SkinToneCorrection"),
        (0x96, "ClarityControl"),
        (0x200, "BlackPoint"),
        (0x201, "WhitePoint"),
        (0x203, "ColorMatrixA"),
        (0x204, "ColorMatrixB"),
        (0x205, "CameraSettings"),
        (0x206, "AEInfo"),
        (0x207, "LensInfo"),
        (0x208, "FlashInfo"),
        (0x209, "AEMeteringSegments"),
        (0x20a, "FlashMeteringSegments"),
        (0x20b, "SlaveFlashMeteringSegments"),
        (0x20d, "WB_RGGBLevelsDaylight"),
        (0x20e, "WB_RGGBLevelsShade"),
        (0x20f, "WB_RGGBLevelsCloudy"),
        (0x210, "WB_RGGBLevelsTungsten"),
        (0x211, "WB_RGGBLevelsFluorescentD"),
        (0x212, "WB_RGGBLevelsFluorescentN"),
        (0x213, "WB_RGGBLevelsFluorescentW"),
        (0x214, "WB_RGGBLevelsFlash"),
        (0x215, "CameraInfo"),
        (0x216, "BatteryInfo"),
        (0x21b, "SaturationInfo"),
        (0x21c, "ColorMatrixA2"),
        (0x21d, "ColorMatrixB2"),
        (0x21f, "AFInfo"),
        (0x220, "HuffmanTable"),
        (0x221, "KelvinWB"),
        (0x222, "ColorInfo"),
        (0x224, "EVStepInfo"),
        (0x226, "ShotInfo"),
        (0x227, "FacePos"),
        (0x228, "FaceSize"),
        (0x229, "SerialNumber"),
        (0x22a, "FilterInfo"),
        (0x22b, "LevelInfo"),
        (0x22d, "WBLevels"),
        (0x22e, "Artist"),
        (0x22f, "Copyright"),
        (0x230, "FirmwareVersion"),
        (0x231, "ContrastDetectAFArea"),
        (0x235, "CrossProcessParams"),
        (0x239, "LensInfoQ"),
        (0x23f, "Model"),
        (0x243, "PixelShiftInfo"),
        (0x245, "AFPointInfo"),
        (0x3fe, "DataDump"),
        (0x3ff, "TempInfo"),
        (0x402, "ToneCurve"),
        (0x403, "ToneCurves"),
        (0x405, "UnknownBlock"),
        (0xe00, "PrintIM"),
    ]);

    static ref PENTAX_MODEL_ID_MAP: HashMap<u32, TypeId> = HashMap::from([
        (0x12994, TypeId(vendor::PENTAX, pentax::IST_D_PEF)),
        (0x12aa2, TypeId(vendor::PENTAX, pentax::IST_DS_PEF)),
        (0x12b1a, TypeId(vendor::PENTAX, pentax::IST_DL_PEF)),
        // *ist DS2
        (0x12b7e, TypeId(vendor::PENTAX, pentax::IST_DL2_PEF)),
        (0x12b9c, TypeId(vendor::PENTAX, pentax::K100D_PEF)),
        (0x12b9d, TypeId(vendor::PENTAX, pentax::K110D_PEF)),
        (0x12ba2, TypeId(vendor::PENTAX, pentax::K100D_SUPER_PEF)),
        (0x12c1e, TypeId(vendor::PENTAX, pentax::K10D_PEF)),
        (0x12cd2, TypeId(vendor::PENTAX, pentax::K20D_PEF)),
        (0x12cfa, TypeId(vendor::PENTAX, pentax::K200D_PEF)),
        // K2000
        // K-m
        (0x12db8, TypeId(vendor::PENTAX, pentax::K7_PEF)),
        (0x12dfe, TypeId(vendor::PENTAX, pentax::KX_PEF)),
        (0x12e08, TypeId(vendor::PENTAX, pentax::PENTAX_645D_PEF)),
        (0x12e6c, TypeId(vendor::PENTAX, pentax::KR_PEF)),
        (0x12e76, TypeId(vendor::PENTAX, pentax::K5_PEF)),
        // Q
        // K-01
        // K-30
        // Q10
        (0x12f70, TypeId(vendor::PENTAX, pentax::K5_II_PEF)),
        (0x12f71, TypeId(vendor::PENTAX, pentax::K5_IIS_PEF)),
        // Q7
        // K-50
        (0x12fc0, TypeId(vendor::PENTAX, pentax::K3_PEF)),
        // K-500
        (0x13010, TypeId(vendor::RICOH, ricoh::PENTAX_645Z_PEF)),
        (0x1301a, TypeId(vendor::PENTAX, pentax::KS1_PEF)),
        (0x13024, TypeId(vendor::PENTAX, pentax::KS2_PEF)),
        // Q-S1
        (0x13092, TypeId(vendor::PENTAX, pentax::K1_PEF)),
        (0x1309c, TypeId(vendor::PENTAX, pentax::K3_II_PEF)),
        // GR III
        (0x13222, TypeId(vendor::PENTAX, pentax::K70_PEF)),
        (0x1322c, TypeId(vendor::PENTAX, pentax::KP_PEF)),
        (0x13240, TypeId(vendor::PENTAX, pentax::K1_MKII_PEF)),
        (0x13254, TypeId(vendor::PENTAX, pentax::K3_MKIII_PEF)),
    ]);

    static ref MAKE_TO_ID_MAP: tiff::MakeToIdMap = HashMap::from([
        ("PENTAX *ist D      ", TypeId(vendor::PENTAX, pentax::IST_D_PEF)),
        ("PENTAX *ist DL     ", TypeId(vendor::PENTAX, pentax::IST_DL_PEF)),
        ("PENTAX *ist DL2    ", TypeId(vendor::PENTAX, pentax::IST_DL2_PEF)),
        ("PENTAX *ist DS     ", TypeId(vendor::PENTAX, pentax::IST_DS_PEF)),
        ("PENTAX K10D        ", TypeId(vendor::PENTAX, pentax::K10D_PEF)),
        ("PENTAX K100D       ", TypeId(vendor::PENTAX, pentax::K100D_PEF)),
        ("PENTAX K100D Super ", TypeId(vendor::PENTAX, pentax::K100D_SUPER_PEF)),
        ("PENTAX K110D       ", TypeId(vendor::PENTAX, pentax::K110D_PEF)),
        ("PENTAX K20D        ", TypeId(vendor::PENTAX, pentax::K20D_PEF)),
        ("PENTAX K200D       ", TypeId(vendor::PENTAX, pentax::K200D_PEF)),
        ("PENTAX K-1         ", TypeId(vendor::PENTAX, pentax::K1_PEF)),
        ("PENTAX K-1 Mark II ", TypeId(vendor::PENTAX, pentax::K1_MKII_PEF)),
        ("PENTAX K-r         ", TypeId(vendor::PENTAX, pentax::KR_PEF)),
        ("PENTAX K-3         ", TypeId(vendor::PENTAX, pentax::K3_PEF)),
        ("PENTAX K-3 II      ", TypeId(vendor::PENTAX, pentax::K3_II_PEF)),
        ("PENTAX K-3 Mark III             ", TypeId(vendor::PENTAX, pentax::K3_MKIII_PEF)),
        ("PENTAX K-5         ", TypeId(vendor::PENTAX, pentax::K5_PEF)),
        ("PENTAX K-5 II      ", TypeId(vendor::PENTAX, pentax::K5_II_PEF)),
        ("PENTAX K-5 II s    ", TypeId(vendor::PENTAX, pentax::K5_IIS_PEF)),
        ("PENTAX K-7         ", TypeId(vendor::PENTAX, pentax::K7_PEF)),
        ("PENTAX K-70        ", TypeId(vendor::PENTAX, pentax::K70_PEF)),
        ("PENTAX K-S1        ", TypeId(vendor::PENTAX, pentax::KS1_PEF)),
        ("PENTAX K-S2        ", TypeId(vendor::PENTAX, pentax::KS2_PEF)),
        ("PENTAX K-x         ", TypeId(vendor::PENTAX, pentax::KX_PEF)),
        ("PENTAX KP          ", TypeId(vendor::PENTAX, pentax::KP_PEF)),
        ("PENTAX 645D        ", TypeId(vendor::PENTAX, pentax::PENTAX_645D_PEF)),
        ("PENTAX 645Z        ", TypeId(vendor::RICOH, ricoh::PENTAX_645Z_PEF)),
    ]);

    pub(super) static ref MATRICES: [BuiltinMatrix; 27] = [
        BuiltinMatrix::new(
            TypeId(vendor::PENTAX, pentax::IST_D_PEF),
            0,
            0,
            [9651, -2059, -1189, -8881, 16512, 2487, -1460, 1345, 10687],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::PENTAX, pentax::IST_DL_PEF),
            0,
            0,
            [10829, -2838, -1115, -8339, 15817, 2696, -837, 680, 11939],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::PENTAX, pentax::IST_DL2_PEF),
            0,
            0,
            [10504, -2439, -1189, -8603, 16208, 2531, -1022, 863, 12242],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::PENTAX, pentax::IST_DS_PEF),
            0,
            0,
            [10371, -2333, -1206, -8688, 16231, 2602, -1230, 1116, 11282],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::PENTAX, pentax::K10D_PEF),
            0,
            0,
            [9566, -2863, -803, -7170, 15172, 2112, -818, 803, 9705],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::PENTAX, pentax::K1_PEF),
            0,
            0,
            [8566, -2746, -1201, -3612, 12204, 1550, -893, 1680, 6264],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::PENTAX, pentax::K1_MKII_PEF),
            0,
            0,
            [8596, -2981, -639, -4202, 12046, 2431, -685, 1424, 6122],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::PENTAX, pentax::K100D_PEF),
            0,
            0,
            [11095, -3157, -1324, -8377, 15834, 2720, -1108, 947, 11688],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::PENTAX, pentax::K100D_SUPER_PEF),
            0,
            0,
            [11095, -3157, -1324, -8377, 15834, 2720, -1108, 947, 11688],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::PENTAX, pentax::K110D_PEF),
            0,
            0,
            [11095, -3157, -1324, -8377, 15834, 2720, -1108, 947, 11688],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::PENTAX, pentax::K20D_PEF),
            0,
            0,
            [9427, -2714, -868, -7493, 16092, 1373, -2199, 3264, 7180],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::PENTAX, pentax::K200D_PEF),
            0,
            0,
            [9186, -2678, -907, -8693, 16517, 2260, -1129, 1094, 8524],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::PENTAX, pentax::KR_PEF),
            0,
            0,
            [9895, -3077, -850, -5304, 13035, 2521, -883, 1768, 6936],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::PENTAX, pentax::K3_PEF),
            0,
            0,
            [8542, -2581, -1144, -3995, 12301, 1881, -863, 1514, 5755],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::PENTAX, pentax::K3_II_PEF),
            0,
            0,
            [9251, -3817, -1069, -4627, 12667, 2175, -798, 1660, 5633],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::PENTAX, pentax::K3_MKIII_PEF),
            0,
            0,
            [8571, -2590, -1148, -3995, 12301, 1881, -1052, 1844, 7013],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::PENTAX, pentax::K5_PEF),
            0,
            0,
            [8713, -2833, -743, -4342, 11900, 2772, -722, 1543, 6247],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::PENTAX, pentax::K5_II_PEF),
            0,
            0,
            [8435, -2549, -1130, -3995, 12301, 1881, -989, 1734, 6591],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::PENTAX, pentax::K5_IIS_PEF),
            0,
            0,
            [8170, -2725, -639, -4440, 12017, 2744, -771, 1465, 6599],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::PENTAX, pentax::K7_PEF),
            0,
            0,
            [9142, -2947, -678, -8648, 16967, 1663, -2224, 2898, 8615],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::PENTAX, pentax::K70_PEF),
            0,
            0,
            [8766, -3149, -747, -3976, 11943, 2292, -517, 1259, 5552],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::PENTAX, pentax::KX_PEF),
            0,
            0,
            [8843, -2837, -625, -5025, 12644, 2668, -411, 1234, 7410],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::PENTAX, pentax::KS1_PEF),
            0,
            0,
            [7989, -2511, -1137, -3882, 12350, 1689, -862, 1524, 6444],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::PENTAX, pentax::KS2_PEF),
            0,
            0,
            [8662, -3280, -798, -3928, 11771, 2444, -586, 1232, 6054],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::PENTAX, pentax::KP_PEF),
            0,
            0,
            [8617, -3228, -1034, -4674, 12821, 2044, -803, 1577, 5728],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::PENTAX, pentax::PENTAX_645D_PEF),
            0,
            0x3e00,
            [10646, -3593, -1158, -3329, 11699, 1831, -667, 2874, 6287],
        ),
        BuiltinMatrix::new(
            TypeId(vendor::RICOH, ricoh::PENTAX_645Z_PEF),
            0,
            0x3fff,
            [9519, -3591, -664, -4074, 11725, 2671, -624, 1501, 6653],
        ),
    ];
}

pub(crate) struct PefFile {
    reader: Rc<Viewer>,
    type_id: OnceCell<TypeId>,
    container: OnceCell<tiff::Container>,
    thumbnails: OnceCell<Vec<(u32, thumbnail::ThumbDesc)>>,
}

impl PefFile {
    pub fn factory(reader: Box<dyn ReadAndSeek>) -> Box<dyn RawFile> {
        let viewer = Viewer::new(reader, 0);
        Box::new(PefFile {
            reader: viewer,
            type_id: OnceCell::new(),
            container: OnceCell::new(),
            thumbnails: OnceCell::new(),
        })
    }
}

impl RawFileImpl for PefFile {
    fn identify_id(&self) -> TypeId {
        *self.type_id.get_or_init(|| {
            if let Some(maker_note) = self.maker_note_ifd() {
                if let Some(id) = maker_note.uint_value(exif::MNOTE_PENTAX_MODEL_ID) {
                    log::debug!("Pentax model ID: {:x} ({})", id, id);
                    return PENTAX_MODEL_ID_MAP
                        .get(&id)
                        .copied()
                        .unwrap_or(TypeId(vendor::PENTAX, 0));
                } else {
                    log::error!("Pentax model ID tag not found");
                }
            }
            let container = self.container.get().unwrap();
            tiff::identify_with_exif(container, &MAKE_TO_ID_MAP)
                .unwrap_or(TypeId(vendor::PENTAX, 0))
        })
    }

    fn container(&self) -> &dyn RawContainer {
        self.container.get_or_init(|| {
            // XXX we should be faillible here.
            let view = Viewer::create_view(&self.reader, 0).expect("Created view");
            let mut container = tiff::Container::new(
                view,
                vec![
                    tiff::IfdType::Main,
                    tiff::IfdType::Other,
                    tiff::IfdType::Other,
                ],
                self.type_(),
            );
            container.load(None).expect("PEF container error");
            container
        })
    }

    fn thumbnails(&self) -> &Vec<(u32, thumbnail::ThumbDesc)> {
        self.thumbnails.get_or_init(|| {
            self.container();
            let container = self.container.get().unwrap();
            let mut thumbnails = tiff::tiff_thumbnails(container);

            self.ifd(tiff::IfdType::MakerNote).and_then(|mnote| {
                // The MakerNote has a MNOTE_PENTAX_PREVIEW_IMAGE_SIZE
                // That contain w in [0] and h in [1]
                let start =
                    mnote.uint_value(exif::MNOTE_PENTAX_PREVIEW_IMAGE_START)? + mnote.mnote_offset;
                let len = mnote.uint_value(exif::MNOTE_PENTAX_PREVIEW_IMAGE_LENGTH)?;
                container
                    .add_thumbnail_from_stream(start, len, &mut thumbnails)
                    .ok()
            });

            thumbnails
        })
    }

    fn ifd(&self, ifd_type: tiff::IfdType) -> Option<Rc<Dir>> {
        self.container();
        let container = self.container.get().unwrap();
        match ifd_type {
            tiff::IfdType::Main | tiff::IfdType::Raw => container.directory(0),
            tiff::IfdType::Exif => self
                .ifd(tiff::IfdType::Main)
                .and_then(|dir| dir.get_exif_ifd(container)),
            tiff::IfdType::MakerNote => self
                .ifd(tiff::IfdType::Exif)
                .and_then(|dir| dir.get_mnote_ifd(container)),
            _ => None,
        }
    }

    fn load_rawdata(&self, _skip_decompress: bool) -> Result<RawData> {
        self.container();
        let container = self.container.get().unwrap();
        self.ifd(tiff::IfdType::Raw)
            .ok_or(Error::NotFound)
            .and_then(|dir| tiff::tiff_get_rawdata(container, &dir, self.type_()))
            .map(|mut rawdata| {
                if let Some(mnote) = self.ifd(tiff::IfdType::MakerNote) {
                    mnote
                        .entry(exif::MNOTE_PENTAX_IMAGEAREAOFFSET)
                        .and_then(|e| {
                            let pt = e.value_array::<u16>(container.endian()).and_then(|a| {
                                if a.len() < 2 {
                                    return None;
                                }
                                Some(Point {
                                    x: a[0] as u32,
                                    y: a[1] as u32,
                                })
                            })?;
                            let e = mnote.entry(exif::MNOTE_PENTAX_RAWIMAGESIZE)?;
                            if let Some(sz) =
                                e.value_array::<u16>(container.endian()).and_then(|a| {
                                    if a.len() < 2 {
                                        return None;
                                    }
                                    Some(Size {
                                        width: a[0] as u32,
                                        height: a[1] as u32,
                                    })
                                })
                            {
                                rawdata.set_active_area(Some(Rect::new(pt, sz)));
                            }

                            Some(())
                        });

                    if let Some(white) = mnote.uint_value(exif::MNOTE_PENTAX_WHITELEVEL) {
                        rawdata.set_white(white as u16);
                    }
                }
                // XXX decompress

                rawdata
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

impl RawFile for PefFile {
    fn type_(&self) -> Type {
        Type::Pef
    }
}

impl Dump for PefFile {
    #[cfg(feature = "dump")]
    fn write_dump<W: std::io::Write + ?Sized>(&self, out: &mut W, indent: u32) {
        dump_writeln!(out, indent, "<Pentax PEF File>");
        {
            let indent = indent + 1;
            self.container();
            self.container.get().unwrap().write_dump(out, indent);
        }
        dump_writeln!(out, indent, "</Pentax PEF File>");
    }
}

dumpfile_impl!(PefFile);
