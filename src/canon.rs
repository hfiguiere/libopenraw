/*
 * libopenraw - canon.rs
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

//! Canon specific code.

mod cr2;
mod cr3;

use std::collections::HashMap;
use std::rc::Rc;

use lazy_static::lazy_static;

use super::TypeId;
use crate::camera_ids::{canon, vendor};
use crate::tiff;
use crate::tiff::{exif, Dir, Ifd};
pub use cr2::Cr2File;
pub use cr3::Cr3File;

lazy_static! {
    /// Map the Canon IDs to `TypeId`. This is the most reliable way for Canon
    static ref CANON_MODEL_ID_MAP: HashMap<u32, TypeId> = HashMap::from([
        (0x80000001, TypeId(vendor::CANON, canon::EOS_1D)),
        (0x80000167, TypeId(vendor::CANON, canon::EOS_1DS)),
        (0x80000174, TypeId(vendor::CANON, canon::EOS_1DMKII)),
        (0x80000175, TypeId(vendor::CANON, canon::EOS_20D)),
        (0x80000188, TypeId(vendor::CANON, canon::EOS_1DSMKII)),
        (0x80000189, TypeId(vendor::CANON, canon::EOS_350D)),
        (0x80000213, TypeId(vendor::CANON, canon::EOS_5D)),
        (0x80000232, TypeId(vendor::CANON, canon::EOS_1DMKIIN)),
        (0x80000234, TypeId(vendor::CANON, canon::EOS_30D)),
        (0x80000236, TypeId(vendor::CANON, canon::EOS_400D)),
        (0x80000169, TypeId(vendor::CANON, canon::EOS_1DMKIII)),
        (0x80000190, TypeId(vendor::CANON, canon::EOS_40D)),
        (0x80000215, TypeId(vendor::CANON, canon::EOS_1DSMKIII)),
        (0x02230000, TypeId(vendor::CANON, canon::G9)),
        (0x80000176, TypeId(vendor::CANON, canon::EOS_450D)),
        (0x80000254, TypeId(vendor::CANON, canon::EOS_1000D)),
        (0x80000261, TypeId(vendor::CANON, canon::EOS_50D)),
        (0x02490000, TypeId(vendor::CANON, canon::G10)),
        (0x80000218, TypeId(vendor::CANON, canon::EOS_5DMKII)),
        (0x02460000, TypeId(vendor::CANON, canon::SX1_IS)),
        (0x80000252, TypeId(vendor::CANON, canon::EOS_500D)),
        (0x02700000, TypeId(vendor::CANON, canon::G11)),
        (0x02720000, TypeId(vendor::CANON, canon::S90)),
        (0x80000250, TypeId(vendor::CANON, canon::EOS_7D)),
        (0x80000281, TypeId(vendor::CANON, canon::EOS_1DMKIV)),
        (0x80000270, TypeId(vendor::CANON, canon::EOS_550D)),
        (0x02950000, TypeId(vendor::CANON, canon::S95)),
        (0x80000287, TypeId(vendor::CANON, canon::EOS_60D)),
        (0x02920000, TypeId(vendor::CANON, canon::G12)),
        (0x80000286, TypeId(vendor::CANON, canon::EOS_600D)),
        (0x80000288, TypeId(vendor::CANON, canon::EOS_1100D)),
        (0x03110000, TypeId(vendor::CANON, canon::S100)),
        (0x80000269, TypeId(vendor::CANON, canon::EOS_1DX)),
        (0x03080000, TypeId(vendor::CANON, canon::G1X)),
        (0x80000285, TypeId(vendor::CANON, canon::EOS_5DMKIII)),
        (0x80000301, TypeId(vendor::CANON, canon::EOS_650D)),
        (0x80000331, TypeId(vendor::CANON, canon::EOS_M)),
        (0x03320000, TypeId(vendor::CANON, canon::S100V)),
        (0x03360000, TypeId(vendor::CANON, canon::S110)),
        (0x03330000, TypeId(vendor::CANON, canon::G15)),
        (0x03340000, TypeId(vendor::CANON, canon::SX50_HS)),
        (0x80000302, TypeId(vendor::CANON, canon::EOS_6D)),
        (0x80000326, TypeId(vendor::CANON, canon::EOS_700D)),
        (0x80000346, TypeId(vendor::CANON, canon::EOS_100D)),
        (0x80000325, TypeId(vendor::CANON, canon::EOS_70D)),
        (0x03540000, TypeId(vendor::CANON, canon::G16)),
        (0x03550000, TypeId(vendor::CANON, canon::S120)),
        (0x80000355, TypeId(vendor::CANON, canon::EOS_M2)),
        (0x80000327, TypeId(vendor::CANON, canon::EOS_1200D)),
        (0x03640000, TypeId(vendor::CANON, canon::G1XMKII)),
        (0x80000289, TypeId(vendor::CANON, canon::EOS_7DMKII)),
        (0x03780000, TypeId(vendor::CANON, canon::G7X)),
        (0x03750000, TypeId(vendor::CANON, canon::SX60_HS)),
        (0x80000382, TypeId(vendor::CANON, canon::EOS_5DS)),
        (0x80000401, TypeId(vendor::CANON, canon::EOS_5DS_R)),
        (0x80000393, TypeId(vendor::CANON, canon::EOS_750D)),
        (0x80000347, TypeId(vendor::CANON, canon::EOS_760D)),
        (0x03740000, TypeId(vendor::CANON, canon::EOS_M3)),
        (0x03850000, TypeId(vendor::CANON, canon::G3X)),
        (0x03950000, TypeId(vendor::CANON, canon::G5X)),
        (0x03930000, TypeId(vendor::CANON, canon::G9X)),
        (0x03840000, TypeId(vendor::CANON, canon::EOS_M10)),
        (0x80000328, TypeId(vendor::CANON, canon::EOS_1DXMKII)),
        (0x80000350, TypeId(vendor::CANON, canon::EOS_80D)),
        (0x03970000, TypeId(vendor::CANON, canon::G7XMKII)),
        (0x80000404, TypeId(vendor::CANON, canon::EOS_1300D)),
        (0x80000349, TypeId(vendor::CANON, canon::EOS_5DMKIV)),
        (0x03940000, TypeId(vendor::CANON, canon::EOS_M5)),
        (0x04100000, TypeId(vendor::CANON, canon::G9XMKII)),
        (0x80000405, TypeId(vendor::CANON, canon::EOS_800D)),
        (0x80000408, TypeId(vendor::CANON, canon::EOS_77D)),
        (0x04070000, TypeId(vendor::CANON, canon::EOS_M6)),
        (0x80000417, TypeId(vendor::CANON, canon::EOS_200D)),
        (0x80000406, TypeId(vendor::CANON, canon::EOS_6DMKII)),
        (0x03980000, TypeId(vendor::CANON, canon::EOS_M100)),
        (0x04180000, TypeId(vendor::CANON, canon::G1XMKIII)),
        (0x80000432, TypeId(vendor::CANON, canon::EOS_2000D)),
        (0x80000422, TypeId(vendor::CANON, canon::EOS_3000D)),
        (0x00000412, TypeId(vendor::CANON, canon::EOS_M50)),
        (0x80000424, TypeId(vendor::CANON, canon::EOS_R)),
        (0x80000433, TypeId(vendor::CANON, canon::EOS_RP)),
        (0x80000421, TypeId(vendor::CANON, canon::EOS_R5)),
        (0x80000453, TypeId(vendor::CANON, canon::EOS_R6)),
        (0x80000436, TypeId(vendor::CANON, canon::EOS_250D)),
        (0x00000804, TypeId(vendor::CANON, canon::G5XMKII)),
        (0x00000805, TypeId(vendor::CANON, canon::SX70_HS)),
        (0x00000808, TypeId(vendor::CANON, canon::G7XMKIII)),
        (0x80000437, TypeId(vendor::CANON, canon::EOS_90D)),
        (0x00000811, TypeId(vendor::CANON, canon::EOS_M6MKII)),
        (0x00000812, TypeId(vendor::CANON, canon::EOS_M200)),
        (0x80000428, TypeId(vendor::CANON, canon::EOS_1DXMKIII)),
        (0x80000435, TypeId(vendor::CANON, canon::EOS_850D)),
        (0x80000468, TypeId(vendor::CANON, canon::EOS_M50MKII)),
        (0x80000450, TypeId(vendor::CANON, canon::EOS_R3)),
    ]);

    /// Canon MakerNote tag names
    pub static ref MNOTE_TAG_NAMES: HashMap<u16, &'static str> = HashMap::from([
        (0x1, "CanonCameraSettings"),
        (0x2, "CanonFocalLength"),
        (0x3, "CanonFlashInfo"),
        (0x4, "CanonShotInfo"),
        (0x5, "CanonPanorama"),
        (0x6, "CanonImageType"),
        (0x7, "CanonFirmwareVersion"),
        (0x8, "FileNumber"),
        (0x9, "OwnerName"),
        (0xa, "UnknownD30"),
        (0xc, "SerialNumber"),
        (0xd, "CanonCameraInfo1D"),
        (0xe, "CanonFileLength"),
        (0xf, "CustomFunctions1D"),
        (0x10, "CanonModelID"),
        (0x11, "MovieInfo"),
        (0x12, "CanonAFInfo"),
        (0x13, "ThumbnailImageValidArea"),
        (0x15, "SerialNumberFormat"),
        (0x1a, "SuperMacro"),
        (0x1c, "DateStampMode"),
        (0x1d, "MyColors"),
        (0x1e, "FirmwareRevision"),
        (0x23, "Categories"),
        (0x24, "FaceDetect1"),
        (0x25, "FaceDetect2"),
        (0x26, "CanonAFInfo2"),
        (0x27, "ContrastInfo"),
        (0x28, "ImageUniqueID"),
        (0x29, "WBInfo"),
        (0x2f, "FaceDetect3"),
        (0x35, "TimeInfo"),
        (0x38, "BatteryType"),
        (0x3c, "AFInfo3"),
        (0x81, "RawDataOffset"),
        (0x83, "OriginalDecisionDataOffset"),
        (0x90, "CustomFunctions1D"),
        (0x91, "PersonalFunctions"),
        (0x92, "PersonalFunctionValues"),
        (0x93, "CanonFileInfo"),
        (0x94, "AFPointsInFocus1D"),
        (0x95, "LensModel"),
        (0x96, "SerialInfo"),
        (0x97, "DustRemovalData"),
        (0x98, "CropInfo"),
        (0x99, "CustomFunctions2"),
        (0x9a, "AspectInfo"),
        (0xa0, "ProcessingInfo"),
        (0xa1, "ToneCurveTable"),
        (0xa2, "SharpnessTable"),
        (0xa3, "SharpnessFreqTable"),
        (0xa4, "WhiteBalanceTable"),
        (0xa9, "ColorBalance"),
        (0xaa, "MeasuredColor"),
        (0xae, "ColorTemperature"),
        (0xb0, "CanonFlags"),
        (0xb1, "ModifiedInfo"),
        (0xb2, "ToneCurveMatching"),
        (0xb3, "WhiteBalanceMatching"),
        (0xb4, "ColorSpace"),
        (0xb6, "PreviewImageInfo"),
        (0xd0, "VRDOffset"),
        (0xe0, "SensorInfo"),
        (0x4001, "ColorData1"),
        (0x4002, "CRWParam"),
        (0x4003, "ColorInfo"),
        (0x4005, "Flavor"),
        (0x4008, "PictureStyleUserDef"),
        (0x4009, "PictureStylePC"),
        (0x4010, "CustomPictureStyleFileName"),
        (0x4013, "AFMicroAdj"),
        (0x4015, "VignettingCorr"),
        (0x4016, "VignettingCorr2"),
        (0x4018, "LightingOpt"),
        (0x4019, "LensInfo"),
        (0x4020, "AmbienceInfo"),
        (0x4021, "MultiExp"),
        (0x4024, "FilterInfo"),
        (0x4025, "HDRInfo"),
        (0x4028, "AFConfig"),
        (0x403f, "RawBurstModeRoll"),
    ]);
}

/// Get the TypeId for the model ID.
fn get_typeid_for_modelid(model_id: u32) -> TypeId {
    CANON_MODEL_ID_MAP
        .get(&model_id)
        .copied()
        .unwrap_or(TypeId(vendor::CANON, canon::UNKNOWN))
}

pub(crate) fn identify_from_maker_note(maker_note: Rc<tiff::Dir>) -> TypeId {
    if let Some(id) = maker_note.value::<u32>(exif::MNOTE_CANON_MODEL_ID) {
        log::debug!("Canon model ID: {:x}", id);
        return get_typeid_for_modelid(id);
    } else {
        log::error!("Canon model ID tag not found");
    }
    TypeId(0, 0)
}

/// SensorInfo currently only contain the active area (x, y, w, h)
pub(crate) struct SensorInfo([u32; 4]);

impl SensorInfo {
    /// Load the `SensorInfo` from the MakerNote
    pub fn new(maker_note: Rc<Dir>) -> Option<SensorInfo> {
        maker_note
            .entry(exif::MNOTE_CANON_SENSORINFO)
            .and_then(|e| e.value_array(maker_note.endian()))
            .and_then(Self::parse)
    }

    /// Parse the `SensorInfo` from the array of u16
    fn parse(sensor_info: Vec<u16>) -> Option<SensorInfo> {
        if sensor_info.len() <= 8 {
            log::warn!("Data too small for sensor info {}", sensor_info.len());
            None
        } else {
            let mut result = [0u32; 4];
            result[0] = sensor_info[5] as u32;
            result[1] = sensor_info[6] as u32;
            if sensor_info[7] <= sensor_info[5] {
                log::warn!(
                    "sensor_info: bottom {} <= top {}",
                    sensor_info[7],
                    sensor_info[5]
                );
                return None;
            }
            let mut w: u32 = (sensor_info[7] - sensor_info[5]) as u32;
            // it seems that this could lead to an odd number. Make it even.
            if (w % 2) != 0 {
                w += 1;
            }
            result[2] = w;
            if sensor_info[8] <= sensor_info[6] {
                log::warn!(
                    "sensor_info: right {} <= left {}",
                    sensor_info[8],
                    sensor_info[6]
                );
                return None;
            }
            let mut h: u32 = (sensor_info[8] - sensor_info[6]) as u32;
            // same as for width
            if (h % 2) != 0 {
                h += 1;
            }
            result[3] = h;
            Some(SensorInfo(result))
        }
    }
}
