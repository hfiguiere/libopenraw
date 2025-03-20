// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - canon.rs
 *
 * Copyright (C) 2022-2024 Hubert Figuière
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

mod colour;
mod cr2;
mod cr3;
mod crw;
mod matrices;

use std::collections::HashMap;

use lazy_static::lazy_static;

use crate::tiff::{self, exif, Dir, Ifd};
use crate::{AspectRatio, Rect, TypeId};
use colour::ColourFormat;
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

lazy_static::lazy_static! {
    static ref MAKE_TO_ID_MAP: tiff::MakeToIdMap = HashMap::from([
        // TIF
        canon!("Canon EOS-1D", EOS_1D),
        canon!("Canon EOS-1DS", EOS_1DS),

        // CRW
        canon!("Canon EOS D30" , EOS_D30),
        canon!("Canon EOS D60" , EOS_D60),
        canon!("Canon EOS 10D" , EOS_10D),
        canon!("Canon EOS DIGITAL REBEL", DIGITAL_REBEL),
        canon!("Canon EOS 300D DIGITAL", EOS_300D),
        canon!("Canon PowerShot G1", G1),
        canon!("Canon PowerShot G2", G2),
        canon!("Canon PowerShot G3", G3),
        canon!("Canon PowerShot G5", G5),
        canon!("Canon PowerShot G6", G6),
        // G7 is CHDK, So remove from the list from now.
        //    canon!("Canon PowerShot G7", G7),
        canon!("Canon PowerShot Pro1", PRO1),
        canon!("Canon PowerShot Pro70", PRO70),
        canon!("Canon PowerShot Pro90 IS", PRO90),
        canon!("Canon PowerShot S30", S30),
        canon!("Canon PowerShot S40", S40),
        canon!("Canon PowerShot S45", S45),
        canon!("Canon PowerShot S50", S50),
        canon!("Canon PowerShot S60", S60),
        canon!("Canon PowerShot S70", S70),

        // CR2
        canon!("Canon EOS-1D Mark II", EOS_1DMKII),
        canon!("Canon EOS-1D Mark II N", EOS_1DMKIIN),
        canon!("Canon EOS-1D Mark III", EOS_1DMKIII),
        canon!("Canon EOS-1D Mark IV", EOS_1DMKIV),
        canon!("Canon EOS-1Ds Mark II", EOS_1DSMKII),
        canon!("Canon EOS-1Ds Mark III", EOS_1DSMKIII),
        canon!("Canon EOS-1D X", EOS_1DX),
        canon!("Canon EOS-1D X Mark II", EOS_1DXMKII),
        canon!("Canon EOS 20D", EOS_20D),
        canon!("Canon EOS 20Da", EOS_20DA),
        canon!("Canon EOS 30D", EOS_30D),
        canon!("Canon EOS 350D DIGITAL", EOS_350D),
        canon!("Canon EOS DIGITAL REBEL XT", REBEL_XT),
        canon!("Canon EOS Kiss Digital N", KISS_DIGITAL_N),
        canon!("Canon EOS 40D", EOS_40D),
        canon!("Canon EOS 400D DIGITAL", EOS_400D),
        canon!("Canon EOS 450D", EOS_450D),
        canon!("Canon EOS DIGITAL REBEL XSi", REBEL_XSI),
        canon!("Canon EOS 50D", EOS_50D),
        canon!("Canon EOS 500D", EOS_500D),
        canon!("Canon EOS 550D", EOS_550D),
        canon!("Canon EOS REBEL T2i", REBEL_T2I),
        canon!("Canon EOS 600D", EOS_600D),
        canon!("Canon EOS REBEL T3i", REBEL_T3I),
        canon!("Canon EOS 60D", EOS_60D),
        canon!("Canon EOS 650D", EOS_650D),
        canon!("Canon EOS REBEL T4i", REBEL_T4I),
        canon!("Canon EOS 70D", EOS_70D),
        canon!("Canon EOS 700D", EOS_700D),
        canon!("Canon EOS 750D", EOS_750D),
        canon!("Canon EOS 760D", EOS_760D),
        canon!("Canon EOS 80D", EOS_80D),
        canon!("Canon EOS 800D", EOS_800D),
        canon!("Canon EOS REBEL T1i", REBEL_T1I),
        canon!("Canon EOS Rebel T5", REBEL_T5),
        canon!("Canon EOS REBEL T5i", REBEL_T5I),
        canon!("Canon EOS Rebel T6i", REBEL_T6I),
        canon!("Canon EOS Rebel T6s", REBEL_T6S),
        canon!("Canon EOS Rebel T6", REBEL_T6),
        canon!("Canon EOS Rebel T7i", REBEL_T7I),
        canon!("Canon EOS Rebel T7", REBEL_T7),
        canon!("Canon EOS 1000D", EOS_1000D),
        canon!("Canon EOS 2000D", EOS_2000D),
        canon!("Canon EOS DIGITAL REBEL XS", REBEL_XS),
        canon!("Canon EOS 1100D", EOS_1100D),
        canon!("Canon EOS 1200D", EOS_1200D),
        canon!("Canon EOS 1300D", EOS_1300D),
        canon!("Canon EOS REBEL T3", REBEL_T3),
        canon!("Canon EOS 100D", EOS_100D),
        canon!("Canon EOS REBEL SL1", REBEL_SL1),
        canon!("Canon EOS 200D", EOS_200D),
        canon!("Canon EOS Rebel SL2", REBEL_SL2),
        canon!("Canon EOS 4000D", EOS_4000D),
        canon!("Canon EOS 5D", EOS_5D),
        canon!("Canon EOS 5D Mark II", EOS_5DMKII),
        canon!("Canon EOS 5D Mark III", EOS_5DMKIII),
        canon!("Canon EOS 5D Mark IV", EOS_5DMKIV),
        canon!("Canon EOS 5DS", EOS_5DS),
        canon!("Canon EOS 5DS R", EOS_5DS_R),
        canon!("Canon EOS 6D", EOS_6D),
        canon!("Canon EOS 6D Mark II", EOS_6DMKII),
        canon!("Canon EOS 7D", EOS_7D),
        canon!("Canon EOS 7D Mark II", EOS_7DMKII),
        canon!("Canon EOS 77D", EOS_77D),
        canon!("Canon EOS Kiss X3", KISS_X3),
        canon!("Canon EOS M", EOS_M),
        canon!("Canon EOS M10", EOS_M10),
        canon!("Canon EOS M100", EOS_M100),
        canon!("Canon EOS M2", EOS_M2),
        canon!("Canon EOS M3", EOS_M3),
        canon!("Canon EOS M5", EOS_M5),
        canon!("Canon EOS M6", EOS_M6),
        canon!("Canon PowerShot G9", G9),
        canon!("Canon PowerShot G10", G10),
        canon!("Canon PowerShot G11", G11),
        canon!("Canon PowerShot G12", G12),
        canon!("Canon PowerShot G15", G15),
        canon!("Canon PowerShot G16", G16),
        canon!("Canon PowerShot G1 X", G1X),
        canon!("Canon PowerShot G1 X Mark II", G1XMKII),
        canon!("Canon PowerShot G1 X Mark III", G1XMKIII),
        canon!("Canon PowerShot G3 X", G3X),
        canon!("Canon PowerShot G5 X", G5X),
        canon!("Canon PowerShot G7 X", G7X),
        canon!("Canon PowerShot G7 X Mark II", G7XMKII),
        canon!("Canon PowerShot G9 X", G9X),
        canon!("Canon PowerShot G9 X Mark II", G9XMKII),
        canon!("Canon PowerShot S90", S90),
        canon!("Canon PowerShot S95", S95),
        canon!("Canon PowerShot S100", S100),
        canon!("Canon PowerShot S100V", S100V),
        canon!("Canon PowerShot S110", S110),
        canon!("Canon PowerShot S120", S120),
        canon!("Canon PowerShot SX1 IS", SX1_IS),
        canon!("Canon PowerShot SX50 HS", SX50_HS),
        canon!("Canon PowerShot SX60 HS", SX60_HS),

        // CR3
        canon!("Canon EOS M50", EOS_M50),
        canon!("Canon EOS M50 Mark II", EOS_M50MKII),
        canon!("Canon EOS M200", EOS_M200),
        canon!("Canon EOS R", EOS_R),
        canon!("Canon EOS RP", EOS_RP),
        canon!("Canon EOS R3", EOS_R3),
        canon!("Canon EOS R5", EOS_R5),
        canon!("Canon EOS R6", EOS_R6),
        canon!("Canon EOS R6 m2", EOS_R6MKII),
        canon!("Canon EOS R7", EOS_R7),
        canon!("Canon EOS R8", EOS_R8),
        canon!("Canon EOS R10", EOS_R10),
        canon!("Canon EOS R100", EOS_R100),
        canon!("Canon EOS R50", EOS_R50),
        canon!("Canon EOS 250D", EOS_250D),
        canon!("Canon EOS Rebel SL3", EOS_250D),
        canon!("Canon EOS 850D", EOS_850D),
        canon!("Canon EOS Rebel T8i", EOS_850D),
        canon!("Canon PowerShot SX70 HS", SX70_HS),
        canon!("Canon PowerShot G5 X Mark II", G5XMKII),
        canon!("Canon PowerShot G7 X Mark III", G7XMKIII),
        canon!("Canon EOS-1D X Mark III", EOS_1DXMKIII),
        canon!("Canon EOS M6 Mark II", EOS_M6MKII),
        canon!("Canon EOS 90D", EOS_90D),
        canon!("Canon EOS R1", EOS_R1),
        canon!("Canon EOS R5m2", EOS_R5MKII),
    ]);
}

pub use tiff::exif::generated::MNOTE_CANON_TAG_NAMES as MNOTE_TAG_NAMES;

/// This function will output the list of camera ID with names.
/// This is only useful to generate document.
#[cfg(feature = "book")]
pub fn print_models() {
    let id_to_name =
        multimap::MultiMap::<TypeId, &str>::from_iter(MAKE_TO_ID_MAP.iter().map(|v| (*v.1, *v.0)));
    let mut models = CANON_MODEL_ID_MAP.iter().collect::<Vec<_>>();
    models.sort_by(|m1, m2| m1.0.cmp(m2.0));
    for model in models {
        let name = id_to_name.get_vec(model.1);
        println!(
            "| 0x{:08x} | {} |",
            model.0,
            name.map(|v| v.join(", "))
                .unwrap_or_else(|| model.1.to_string())
        );
    }
}

lazy_static! {
    /// Map the Canon IDs to `TypeId`. This is the most reliable way for Canon
    static ref CANON_MODEL_ID_MAP: HashMap<u32, TypeId> = HashMap::from([
        // CRW cameras. Missing is Pro70.
        canon!(0x01100000, G2),
        canon!(0x01110000, S40),
        canon!(0x01120000, S30),
        canon!(0x01140000, EOS_D30),
        canon!(0x01190000, G3),
        canon!(0x01210000, S45),
        canon!(0x01290000, G5),
        canon!(0x01310000, S50),
        canon!(0x01370000, PRO1),
        canon!(0x01380000, S70),
        canon!(0x01390000, S60),
        canon!(0x01400000, G6),
        canon!(0x01668000, EOS_D60),
        canon!(0x03010000, PRO90),
        canon!(0x04040000, G1),
        canon!(0x80000168, EOS_10D),
        canon!(0x80000170, EOS_300D),

        // TIF, CR2, CR3
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
        canon!(0x80000498, EOS_R100),
        canon!(0x80000495, EOS_R1),
        canon!(0x80000496, EOS_R5MKII),
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

pub(crate) struct AspectInfo(Option<AspectRatio>, Rect);

impl AspectInfo {
    /// Load the `AspectInfo` from the MakerNote
    pub fn new(maker_note: &Dir) -> Option<Self> {
        maker_note
            .entry(exif::MNOTE_CANON_ASPECT_INFO)
            .and_then(|e| e.uint_value_array(maker_note.endian()))
            .and_then(Self::parse)
    }

    /// Parse the `AspectInfo` from the array of u16
    fn parse(aspect_info: Vec<u32>) -> Option<AspectInfo> {
        if aspect_info.len() < 5 {
            return None;
        }
        let aspect_ratio = match aspect_info[0] {
            // 256 seen on 7D firmware 2.0.3
            0 | 12 | 13 | 256 => Some(AspectRatio(3, 2)),
            1 => Some(AspectRatio(1, 1)),
            2 | 258 => Some(AspectRatio(4, 3)),
            7 => Some(AspectRatio(16, 9)),
            8 => Some(AspectRatio(4, 5)),
            _ => {
                log::error!("Unknown aspect ratio {}", aspect_info[0]);
                None
            }
        };
        Some(AspectInfo(
            aspect_ratio,
            Rect {
                x: aspect_info[3],
                y: aspect_info[4],
                width: aspect_info[1],
                height: aspect_info[2],
            },
        ))
    }
}

/// SensorInfo currently only contain the active area (x, y, w, h)
pub(crate) struct SensorInfo(Rect);

/*
1 	SensorWidth
2 	SensorHeight
5 	SensorLeftBorder
6 	SensorTopBorder
7 	SensorRightBorder
8 	SensorBottomBorder
9 	BlackMaskLeftBorder
10 	BlackMaskTopBorder
11 	BlackMaskRightBorder
12 	BlackMaskBottomBorder
*/

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
            let mut result = Rect {
                x: sensor_info[5] as u32,
                y: sensor_info[6] as u32,
                ..Default::default()
            };
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
            result.width = w;
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
            result.height = h;
            Some(SensorInfo(result))
        }
    }
}
