// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - canon.rs
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

//! Canon specific code.

mod cr2;
mod cr3;
mod crw;
mod matrices;

use std::collections::HashMap;

use lazy_static::lazy_static;

use super::TypeId;
use crate::tiff;
use crate::tiff::{exif, Dir, Ifd};
pub(crate) use cr2::Cr2File;
pub(crate) use cr3::Cr3File;
pub(crate) use crw::CrwFile;

#[macro_export]
macro_rules! canon {
    ($id:expr, $model:ident) => {
        (
            $id,
            TypeId(
                $crate::camera_ids::vendor::CANON,
                $crate::camera_ids::canon::$model,
            ),
        )
    };
    ($model:ident) => {
        TypeId(
            $crate::camera_ids::vendor::CANON,
            $crate::camera_ids::canon::$model,
        )
    };
}

pub use tiff::exif::generated::MNOTE_CANON_TAG_NAMES as MNOTE_TAG_NAMES;

lazy_static! {
    /// Map the Canon IDs to `TypeId`. This is the most reliable way for Canon
    static ref CANON_MODEL_ID_MAP: HashMap<u32, TypeId> = HashMap::from([
        canon!(0x80000001, EOS_1D),
        canon!(0x80000167, EOS_1DS),
        canon!(0x80000174, EOS_1DMKII),
        canon!(0x80000175, EOS_20D),
        canon!(0x80000188, EOS_1DSMKII),
        canon!(0x80000189, EOS_350D),
        canon!(0x80000213, EOS_5D),
        canon!(0x80000232, EOS_1DMKIIN),
        canon!(0x80000234, EOS_30D),
        canon!(0x80000236, EOS_400D),
        canon!(0x80000169, EOS_1DMKIII),
        canon!(0x80000190, EOS_40D),
        canon!(0x80000215, EOS_1DSMKIII),
        canon!(0x02230000, G9),
        canon!(0x80000176, EOS_450D),
        canon!(0x80000254, EOS_1000D),
        canon!(0x80000261, EOS_50D),
        canon!(0x02490000, G10),
        canon!(0x80000218, EOS_5DMKII),
        canon!(0x02460000, SX1_IS),
        canon!(0x80000252, EOS_500D),
        canon!(0x02700000, G11),
        canon!(0x02720000, S90),
        canon!(0x80000250, EOS_7D),
        canon!(0x80000281, EOS_1DMKIV),
        canon!(0x80000270, EOS_550D),
        canon!(0x02950000, S95),
        canon!(0x80000287, EOS_60D),
        canon!(0x02920000, G12),
        canon!(0x80000286, EOS_600D),
        canon!(0x80000288, EOS_1100D),
        canon!(0x03110000, S100),
        canon!(0x80000269, EOS_1DX),
        canon!(0x03080000, G1X),
        canon!(0x80000285, EOS_5DMKIII),
        canon!(0x80000301, EOS_650D),
        canon!(0x80000331, EOS_M),
        canon!(0x03320000, S100V),
        canon!(0x03360000, S110),
        canon!(0x03330000, G15),
        canon!(0x03340000, SX50_HS),
        canon!(0x80000302, EOS_6D),
        canon!(0x80000326, EOS_700D),
        canon!(0x80000346, EOS_100D),
        canon!(0x80000325, EOS_70D),
        canon!(0x03540000, G16),
        canon!(0x03550000, S120),
        canon!(0x80000355, EOS_M2),
        canon!(0x80000327, EOS_1200D),
        canon!(0x03640000, G1XMKII),
        canon!(0x80000289, EOS_7DMKII),
        canon!(0x03780000, G7X),
        canon!(0x03750000, SX60_HS),
        canon!(0x80000382, EOS_5DS),
        canon!(0x80000401, EOS_5DS_R),
        canon!(0x80000393, EOS_750D),
        canon!(0x80000347, EOS_760D),
        canon!(0x03740000, EOS_M3),
        canon!(0x03850000, G3X),
        canon!(0x03950000, G5X),
        canon!(0x03930000, G9X),
        canon!(0x03840000, EOS_M10),
        canon!(0x80000328, EOS_1DXMKII),
        canon!(0x80000350, EOS_80D),
        canon!(0x03970000, G7XMKII),
        canon!(0x80000404, EOS_1300D),
        canon!(0x80000349, EOS_5DMKIV),
        canon!(0x03940000, EOS_M5),
        canon!(0x04100000, G9XMKII),
        canon!(0x80000405, EOS_800D),
        canon!(0x80000408, EOS_77D),
        canon!(0x04070000, EOS_M6),
        canon!(0x80000417, EOS_200D),
        canon!(0x80000406, EOS_6DMKII),
        canon!(0x03980000, EOS_M100),
        canon!(0x04180000, G1XMKIII),
        canon!(0x80000432, EOS_2000D),
        canon!(0x80000422, EOS_3000D),
        canon!(0x00000412, EOS_M50),
        canon!(0x80000424, EOS_R),
        canon!(0x80000433, EOS_RP),
        canon!(0x80000421, EOS_R5),
        canon!(0x80000453, EOS_R6),
        canon!(0x80000436, EOS_250D),
        canon!(0x00000804, G5XMKII),
        canon!(0x00000805, SX70_HS),
        canon!(0x00000808, G7XMKIII),
        canon!(0x80000437, EOS_90D),
        canon!(0x00000811, EOS_M6MKII),
        canon!(0x00000812, EOS_M200),
        canon!(0x80000428, EOS_1DXMKIII),
        canon!(0x80000435, EOS_850D),
        canon!(0x80000468, EOS_M50MKII),
        canon!(0x80000450, EOS_R3),
        canon!(0x80000464, EOS_R7),
        canon!(0x80000465, EOS_R10),
        canon!(0x80000480, EOS_R50),
        canon!(0x80000481, EOS_R6MKII),
        canon!(0x80000487, EOS_R8),
    ]);
}

/// Get the TypeId for the model ID.
fn get_typeid_for_modelid(model_id: u32) -> TypeId {
    CANON_MODEL_ID_MAP
        .get(&model_id)
        .copied()
        .unwrap_or(canon!(UNKNOWN))
}

pub(crate) fn identify_from_maker_note(maker_note: &tiff::Dir) -> TypeId {
    if let Some(id) = maker_note.value::<u32>(exif::MNOTE_CANON_MODEL_ID) {
        log::debug!("Canon model ID: {:x}", id);
        return get_typeid_for_modelid(id);
    } else {
        log::error!("Canon model ID tag not found");
    }
    canon!(UNKNOWN)
}

/// SensorInfo currently only contain the active area (x, y, w, h)
pub(crate) struct SensorInfo([u32; 4]);

impl SensorInfo {
    /// Load the `SensorInfo` from the MakerNote
    pub fn new(maker_note: &Dir) -> Option<SensorInfo> {
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
