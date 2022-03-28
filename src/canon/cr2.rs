/*
 * libopenraw - canon/cr2.rs
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

//! Canon CR2 format, the 2nd generation of Canon RAW format, based on
//! TIFF.

use std::collections::HashMap;
use std::rc::Rc;

use once_cell::unsync::OnceCell;

use crate::bitmap;
use crate::camera_ids::{canon, vendor};
use crate::colour::BuiltinMatrix;
use crate::container::GenericContainer;
use crate::decompress;
use crate::ifd;
use crate::ifd::{exif, Dir, Ifd};
use crate::io::Viewer;
use crate::rawfile::ReadAndSeek;
use crate::thumbnail;
use crate::thumbnail::ThumbDesc;
use crate::{Error, RawData, RawFile, RawFileImpl, Result, Type, TypeId};

lazy_static::lazy_static! {
    static ref MATRICES: [BuiltinMatrix; 78] = [
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_1DMKII),
      0,
      0xe80,
      [ 6264, -582, -724, -8312, 15948, 2504, -1744, 1919, 8664 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_1DMKIIN),
      0,
      0xe80,
      [ 6240, -466, -822, -8180, 15825, 2500, -1801, 1938, 8042 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_1DMKIII),
      0,
      0xe80,
      [ 6291, -540, -976, -8350, 16145, 2311, -1714, 1858, 7326 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_1DMKIV),
      0,
      0x3bb0,
      [ 6014, -220, -795, -4109, 12014, 2361, -561, 1824, 5787 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_1DSMKII),
      0,
      0xe80,
      [ 6517, -602, -867, -8180, 15926, 2378, -1618, 1771, 7633 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_1DSMKIII),
      0,
      0x3bb0,
      [ 5859, -211, -930, -8255, 16017, 2353, -1732, 1887, 7448 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_1DX),
      0,
      0x3c4e,
      [ 6847, -614, -1014, -4669, 12737, 2139, -1197, 2488, 6846 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_1DXMKII),
      0,
      0x3c4e,
      [ 7596, -978, -967, -4808, 12571, 2503, -1398, 2567, 5752 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_20D),
      0,
      0xfff,
      [ 6599, -537, -891, -8071, 15783, 2424, -1983, 2234, 7462 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_20DA),
      0,
      0,
      [ 14155, -5065, -1382, -6550, 14633, 2039, -1623, 1824, 6561 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_30D),
      0,
      0,
      [ 6257, -303, -1000, -7880, 15621, 2396, -1714, 1904, 7046 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_350D),
      0,
      0xfff,
      [ 6018, -617, -965, -8645, 15881, 2975, -1530, 1719, 7642 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_40D),
      0,
      0x3f60,
      [ 6071, -747, -856, -7653, 15365, 2441, -2025, 2553, 7315 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_400D),
      0,
      0xe8e,
      [ 7054, -1501, -990, -8156, 15544, 2812, -1278, 1414, 7796 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_450D),
      0,
      0x390d,
      [ 5784, -262, -821, -7539, 15064, 2672, -1982, 2681, 7427 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_50D),
      0,
      0x3d93,
      [ 4920, 616, -593, -6493, 13964, 2784, -1774, 3178, 7005 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_500D),
      0,
      0x3479,
      [ 4763, 712, -646, -6821, 14399, 2640, -1921, 3276, 6561 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_550D),
      0,
      0x3dd7,
      [ 6941, -1164, -857, -3825, 11597, 2534, -416, 1540, 6039 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_600D),
      0,
      0x3510,
      [ 6461, -907, -882, -4300, 12184, 2378, -819, 1944, 5931 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_60D),
      0,
      0x2ff7,
      [ 6719, -994, -925, -4408, 12426, 2211, -887, 2129, 6051 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_650D),
      0,
      0x354d,
      [ 6602, -841, -939, -4472, 12458, 2247, -975, 2039, 6148 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_700D),
      0,
      0x3c00,
      [ 6602, -841, -939, -4472, 12458, 2247, -975, 2039, 6148 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_80D),
      0,
      0,
      [ 7457,-671,-937,-4849,12495,2643,-1213,2354,5492 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_800D),
      0,
      0,
      [ 6970, -512, -968, -4425, 12161, 2553, -739, 1982, 5601 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_100D),
      0,
      0x350f,
      [ 6602, -841, -939, -4472, 12458, 2247, -975, 2039, 6148 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_200D),
      0,
      0x350f,
      [ 7377, -742, -998, -4235, 11981, 2549, -673, 1918, 5538 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_1000D),
      0,
      0xe43,
      [ 6771, -1139, -977, -7818, 15123, 2928, -1244, 1437, 7533 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_1100D),
      0,
      0x3510,
      [ 6444, -904, -893, -4563, 12308, 2535, -903, 2016, 6728 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_100D),
      0,
      0x3806,
      [ 6602, -841, -939, -4472, 12458, 2247, -975, 2039, 6148 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_1200D), // Rebel T5
      0,
      0x37c2,
      [ 6461, -907, -882, -4300, 12184, 2378, -819, 1944, 5931 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_1300D), // Rebel T6
      0,
      0x3510,
      [ 6939, -1016, -866, -4428, 12473, 2177, -1175, 2178, 6162 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_2000D), // Rebel T7
      0,
      0,
      [ 8532, -701, -1167, -4095, 11879, 2508, -797, 2424, 7010 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_3000D), // 4000D, Rebel T100
      0,
      0,
      [ 6939, -1016, -866, -4428, 12473, 2177, -1175, 2178, 6162 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_5D),
      0,
      0xe6c,
      [ 6347, -479, -972, -8297, 15954, 2480, -1968, 2131, 7649 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_5DMKII),
      0,
      0x3cf0,
      [ 4716, 603, -830, -7798, 15474, 2480, -1496, 1937, 6651 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_5DMKIII),
      0,
      0,
      [ 6722, -635, -963, -4287, 12460, 2028, -908, 2162, 5668 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_5DMKIV),
      0,
      0x3bb0,
      [  6014, -220, -795, -4109, 12014, 2361, -561, 1824, 5787 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_5DS),
      0,
      0xe6c,
      [ 6250, -711, -808, -5153, 12794, 2636, -1249, 2198, 5610 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_5DS_R),
      0,
      0xe6c,
      [ 6250, -711, -808, -5153, 12794, 2636, -1249, 2198, 5610 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_6D),
      0,
      0x3c82,
      [ 7034, -804, -1014, -4420, 12564, 2058, -851, 1994, 5758 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_6DMKII),
      0,
      0,
      [ 6875, -970, -932, -4691, 12459, 2501, -874, 1953, 5809 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_7D),
      0,
      0x3510,
      [ 6844, -996, -856, -3876, 11761, 2396, -593, 1772, 6198 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_7DMKII),
      0,
      0x3510,
      [ 7268, -1082, -969, -4186, 11839, 2663, -825, 2029, 5839 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_70D),
      0,
      0x3bc7,
      [ 7034, -804, -1014, -4420, 12564, 2058, -851, 1994, 5758 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_750D),
      0,
      0x368e,
      [ 6362, -823, -847, -4426, 12109, 2616, -743, 1857, 5635 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_760D),
      0,
      0x350f,
      [ 6362, -823, -847, -4426, 12109, 2616, -743, 1857, 5635 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_77D),
      0,
      0,
      [ 7377, -742, -998, -4235, 11981, 2549, -673, 1918, 5538 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_M),
      0,
      0,
      [ 6602, -841, -939, -4472, 12458, 2247, -975, 2039, 6148 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_M2),
      0,
      0,
      [ 6400, -480, -888, -5294, 13416, 2047, -1296, 2203, 6137 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_M3),
      0,
      0,
      [ 6362, -823, -847, -4426, 12109, 2616, -743, 1857, 5635 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_M5),
      0,
      0,
      [ 8532, -701, -1167, -4095, 11879, 2508, -797, 2424, 7010 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_M6),
      0,
      0,
      [ 8532, -701, -1167, -4095, 11879, 2508, -797, 2424, 7010 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_M10),
      0,
      0,
      [ 6400, -480, -888, -5294, 13416, 2047, -1296, 2203, 6137 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::EOS_M100),
      0,
      0,
      [ 8532, -701, -1167, -4095, 11879, 2508, -797, 2424, 7010 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::G9),
      0,
      0,
      [ 7368, -2141, -598, -5621, 13254, 2625, -1418, 1696, 5743 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::G10),
      0,
      0,
      [ 11093, -3906, -1028, -5047, 12492, 2879, -1003, 1750, 5561 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::G11),
      0,
      0,
      [ 12177, -4817, -1069, -1612, 9864, 2049, -98, 850, 4471 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::G12),
      0,
      0,
      [ 13244, -5501, -1248, -1508, 9858, 1935, -270, 1083, 4366 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::G15),
      0,
      0,
      [ 7474, -2301, -567, -4056, 11456, 2975, -222, 716, 4181 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::G16),
      0,
      0,
      [ 8020, -2687, -682, -3704, 11879, 2052, -965, 1921, 5556 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::G1X),
      0,
      0,
      [ 7378, -1255, -1043, -4088, 12251, 2048, -876, 1946, 5805 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::G1XMKII),
      0,
      0,
      [ 7378, -1255, -1043, -4088, 12251, 2048, -876, 1946, 5805 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::G1XMKIII),
      0,
      0,
      [ 8532, -701, -1167, -4095, 11879, 2508, -797, 2424, 7010 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::G3X),
      0,
      0,
      [ 9701, -3857, -921, -3149, 11537, 1817, -786, 1817, 5147 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::G5X),
      0,
      0,
      [ 9602, -3823, -937, -2984, 11495, 1675, -407, 1415, 5049 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::G7X),
      0,
      0,
      [ 9602, -3823, -937, -2984, 11495, 1675, -407, 1415, 5049 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::G7XMKII),
      0,
      0,
      [ 9602, -3823, -937, -2984, 11495, 1675, -407, 1415, 5049 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::G9X),
      0,
      0,
      [ 9602, -3823, -937, -2984, 11495, 1675, -407, 1415, 5049 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::G9XMKII),
      0,
      0,
      [ 10056, -4131, -944, -2576, 11143, 1625, -238, 1294, 5179] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::S90),
      0,
      0,
      [ 12374, -5016, -1049, -1677, 9902, 2078, -83, 852, 4683 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::S95),
      0,
      0,
      [ 13440, -5896, -1279, -1236, 9598, 1931, -180, 1001, 4651 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::S100),
      0,
      0,
      [ 7968, -2565, -636, -2873, 10697, 2513, 180, 667, 4211 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::S100V),
      0,
      0,
      [ 7968, -2565, -636, -2873, 10697, 2513, 180, 667, 4211 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::S110),
      0,
      0,
      [ 8039, -2643, -654, -3783, 11230, 2930, -206, 690, 4194 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::S120),
      0,
      0,
      [ 6961, -1685, -695, -4625, 12945, 1836, -1114, 2152, 5518 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::SX50_HS),
      0,
      0,
      [ 12432, -4753, -1247, -2110, 10691, 1629, -412, 1623, 4926 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::SX60_HS),
      0,
      0,
      [ 13161, -5451, -1344, -1989, 10654, 1531, -47, 1271, 4955 ] ),
    BuiltinMatrix::new( TypeId(vendor::CANON, canon::SX1_IS),
      0,
      0,
      [ 6578, -259, -502, -5974, 13030, 3309, -308, 1058, 4970 ] ),
    ];
}

/// Canon CR2 File
pub struct Cr2File {
    reader: Rc<Viewer>,
    container: OnceCell<ifd::Container>,
    thumbnails: OnceCell<HashMap<u32, thumbnail::ThumbDesc>>,
}

impl Cr2File {
    pub fn factory(reader: Box<dyn ReadAndSeek>) -> Box<dyn RawFile> {
        let viewer = Viewer::new(reader);
        Box::new(Cr2File {
            reader: viewer,
            container: OnceCell::new(),
            thumbnails: OnceCell::new(),
        })
    }

    /// Return a lazily loaded `mp4::Container`
    fn container(&self) -> &ifd::Container {
        self.container.get_or_init(|| {
            // XXX we should be faillible here.
            let view = Viewer::create_view(&self.reader, 0).expect("Created view");
            let mut container = ifd::Container::new(view);
            container.load().expect("TIFF container error");
            container
        })
    }

    fn is_cr2(&self) -> bool {
        // XXX todo
        true
    }

    /// Get the raw bytes.
    fn get_raw_bytes(&self, offset: u64, byte_len: u64, slices: &[u16]) -> Result<RawData> {
        let data = self.container().load_buffer8(offset, byte_len);
        if (data.len() as u64) != byte_len {
            log::warn!("Size mismatch for data. Moving on");
        }

        // XXX handle the don't decompress option
        let mut decompressor = decompress::LJpeg::new();
        // in fact on Canon CR2 files slices either do not exists
        // or is 3.
        if slices.len() > 1 {
            decompressor.set_slices(slices);
        }

        let mut io = std::io::Cursor::new(data);
        decompressor.decompress(&mut io)
    }

    /// Load the `RawData` for actual CR2 files.
    fn load_cr2_rawdata(&self) -> Result<RawData> {
        self.container();
        let container = self.container.get().unwrap();

        let cfa_ifd = self.ifd(ifd::Type::Cfa).ok_or_else(|| {
            log::debug!("CFA IFD not found");
            Error::NotFound
        })?;
        let offset = cfa_ifd
            .value::<u32>(exif::EXIF_TAG_STRIP_OFFSETS)
            .ok_or_else(|| {
                log::debug!("offset not found");
                Error::NotFound
            })?;
        let byte_len = cfa_ifd
            .value::<u32>(exif::EXIF_TAG_STRIP_BYTE_COUNTS)
            .ok_or_else(|| {
                log::debug!("byte len not found");
                Error::NotFound
            })?;
        let slices = cfa_ifd
            .entry(exif::CR2_TAG_SLICE)
            .or_else(|| {
                log::debug!("CR2 slice not found");
                None
            })
            .and_then(|entry| entry.value_array::<u16>(container.endian()))
            .or_else(|| {
                log::debug!("CR2 slice value not found");
                None
            })
            .unwrap_or_default();

        // The tags exif::EXIF_TAG_PIXEL_X_DIMENSION
        // and exif::EXIF_TAG_PIXEL_Y_DIMENSION from the Exif IFD
        // contain X & Y but we don't need them right now.
        // We'll use the active area and the JPEG stream.

        let mut rawdata = self.get_raw_bytes(offset as u64, byte_len as u64, &slices)?;

        let sensor_info = container
            .mnote_dir(Type::Cr2)
            .and_then(super::SensorInfo::new)
            .map(|sensor_info| bitmap::Rect {
                x: sensor_info.0[0],
                y: sensor_info.0[1],
                width: sensor_info.0[2],
                height: sensor_info.0[3],
            });
        rawdata.set_active_area(sensor_info);

        Ok(rawdata)
    }
}

impl RawFileImpl for Cr2File {
    fn identify_id(&self) -> TypeId {
        if let Some(maker_note) = self.maker_note_ifd() {
            super::identify_from_maker_note(maker_note)
        } else {
            log::error!("MakerNote not found");
            TypeId(0, 0)
        }
    }

    /// Return a lazily loaded `ifd::Container`
    fn container(&self) -> &dyn GenericContainer {
        self.container.get_or_init(|| {
            // XXX we should be faillible here.
            let view = Viewer::create_view(&self.reader, 0).expect("Created view");
            let mut container = ifd::Container::new(view);
            container.load().expect("TIFF container error");
            container
        })
    }

    fn thumbnails(&self) -> &std::collections::HashMap<u32, ThumbDesc> {
        self.thumbnails.get_or_init(|| {
            if self.is_cr2() {
                self.container();
                let container = self.container.get().unwrap();
                ifd::tiff_thumbnails(container)
            } else {
                // XXX todo non CR2 files
                HashMap::new()
            }
        })
    }

    fn ifd(&self, ifd_type: ifd::Type) -> Option<Rc<Dir>> {
        self.container();
        let container = self.container.get().unwrap();
        match ifd_type {
            ifd::Type::Cfa => {
                if !self.is_cr2() {
                    self.ifd(ifd::Type::MakerNote)
                } else {
                    // XXX todo set the IFD to type Cfa
                    container.directory(3)
                }
            }
            ifd::Type::Main =>
            // XXX todo set the IFD to type Main
            {
                container.directory(0)
            }
            ifd::Type::Exif => container.exif_dir(),
            ifd::Type::MakerNote => container.mnote_dir(Type::Cr2),
            _ => None,
        }
    }

    fn load_rawdata(&self) -> Result<RawData> {
        if self.is_cr2() {
            return self.load_cr2_rawdata();
        }
        Err(Error::NotSupported)
    }

    fn get_builtin_colour_matrix(&self) -> Result<Vec<f64>> {
        MATRICES
            .iter()
            .find(|m| m.camera == self.type_id())
            .map(|m| Vec::from(m.matrix))
            .ok_or(Error::NotFound)
    }
}

impl RawFile for Cr2File {
    fn type_(&self) -> Type {
        Type::Cr2
    }
}
