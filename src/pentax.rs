// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - pentax.rs
 *
 * Copyright (C) 2022-2025 Hubert Figuière
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

use crate::bitmap::Bitmap;
use crate::colour::BuiltinMatrix;
use crate::container::RawContainer;
use crate::io::Viewer;
use crate::rawfile::{RawFileHandleType, ThumbnailStorage};
use crate::tiff;
use crate::tiff::{exif, Dir, Ifd};
use crate::utils;
use crate::{
    AspectRatio, DataType, Dump, Error, Point, RawFile, RawFileHandle, RawFileImpl, RawImage, Rect,
    Result, Size, Type, TypeId,
};

mod decompress;

#[macro_export]
macro_rules! pentax {
    ($id:expr, $model:ident) => {
        (
            $id,
            TypeId(
                $crate::camera_ids::vendor::PENTAX,
                $crate::camera_ids::pentax::$model,
            ),
        )
    };
    ($model:ident) => {
        TypeId(
            $crate::camera_ids::vendor::PENTAX,
            $crate::camera_ids::pentax::$model,
        )
    };
}

#[macro_export]
macro_rules! ricoh {
    ($id:expr, $model:ident) => {
        (
            $id,
            TypeId(
                $crate::camera_ids::vendor::RICOH,
                $crate::camera_ids::ricoh::$model,
            ),
        )
    };
    ($model:ident) => {
        TypeId(
            $crate::camera_ids::vendor::RICOH,
            $crate::camera_ids::ricoh::$model,
        )
    };
}

pub use crate::tiff::exif::generated::MNOTE_PENTAX_TAG_NAMES as MNOTE_TAG_NAMES;

lazy_static::lazy_static! {
    static ref PENTAX_MODEL_ID_MAP: HashMap<u32, TypeId> = HashMap::from([
        pentax!(0x12994, IST_D_PEF),
        pentax!(0x12aa2, IST_DS_PEF),
        pentax!(0x12b1a, IST_DL_PEF),
        // *ist DS2
        pentax!(0x12b7e, IST_DL2_PEF),
        pentax!(0x12b9c, K100D_PEF),
        pentax!(0x12b9d, K110D_PEF),
        pentax!(0x12ba2, K100D_SUPER_PEF),
        pentax!(0x12c1e, K10D_PEF),
        pentax!(0x12cd2, K20D_PEF),
        pentax!(0x12cfa, K200D_PEF),
        pentax!(0x12d72, K2000_PEF),
        pentax!(0x12d73, KM_PEF),
        pentax!(0x12db8, K7_PEF),
        pentax!(0x12dfe, KX_PEF),
        pentax!(0x12e08, PENTAX_645D_PEF),
        pentax!(0x12e6c, KR_PEF),
        pentax!(0x12e76, K5_PEF),
        // Q
        // K-01
        // K-30
        // Q10
        pentax!(0x12f70, K5_II_PEF),
        pentax!(0x12f71, K5_IIS_PEF),
        // Q7
        // K-50
        pentax!(0x12fc0, K3_PEF),
        // K-500
        ricoh!(0x13010, PENTAX_645Z_PEF),
        pentax!(0x1301a, KS1_PEF),
        pentax!(0x13024, KS2_PEF),
        // Q-S1
        pentax!(0x13092, K1_PEF),
        pentax!(0x1309c, K3_II_PEF),
        // GR III
        pentax!(0x13222, K70_PEF),
        pentax!(0x1322c, KP_PEF),
        pentax!(0x13240, K1_MKII_PEF),
        pentax!(0x13254, K3_MKIII_PEF),
    ]);

    static ref MAKE_TO_ID_MAP: tiff::MakeToIdMap = HashMap::from([
        pentax!("PENTAX *ist D      ", IST_D_PEF),
        pentax!("PENTAX *ist DL     ", IST_DL_PEF),
        pentax!("PENTAX *ist DL2    ", IST_DL2_PEF),
        pentax!("PENTAX *ist DS     ", IST_DS_PEF),
        pentax!("PENTAX K10D        ", K10D_PEF),
        pentax!("PENTAX K100D       ", K100D_PEF),
        pentax!("PENTAX K100D Super ", K100D_SUPER_PEF),
        pentax!("PENTAX K110D       ", K110D_PEF),
        pentax!("PENTAX K20D        ", K20D_PEF),
        pentax!("PENTAX K200D       ", K200D_PEF),
        pentax!("PENTAX K2000       ", K2000_PEF),
        pentax!("PENTAX K-1         ", K1_PEF),
        pentax!("PENTAX K-1 Mark II ", K1_MKII_PEF),
        pentax!("PENTAX K-r         ", KR_PEF),
        pentax!("PENTAX K-3         ", K3_PEF),
        pentax!("PENTAX K-3 II      ", K3_II_PEF),
        pentax!("PENTAX K-3 Mark III             ", K3_MKIII_PEF),
        pentax!("PENTAX K-5         ", K5_PEF),
        pentax!("PENTAX K-5 II      ", K5_II_PEF),
        pentax!("PENTAX K-5 II s    ", K5_IIS_PEF),
        pentax!("PENTAX K-7         ", K7_PEF),
        pentax!("PENTAX K-70        ", K70_PEF),
        pentax!("PENTAX K-S1        ", KS1_PEF),
        pentax!("PENTAX K-S2        ", KS2_PEF),
        pentax!("PENTAX K-m         ", KM_PEF),
        pentax!("PENTAX K-x         ", KX_PEF),
        pentax!("PENTAX KP          ", KP_PEF),
        pentax!("PENTAX 645D        ", PENTAX_645D_PEF),
        ricoh!("PENTAX 645Z        ", PENTAX_645Z_PEF),
    ]);

    pub(super) static ref MATRICES: [BuiltinMatrix; 29] = [
        BuiltinMatrix::new(
            pentax!(IST_D_PEF),
            0,
            0,
            [9651, -2059, -1189, -8881, 16512, 2487, -1460, 1345, 10687],
        ),
        BuiltinMatrix::new(
            pentax!(IST_DL_PEF),
            0,
            0,
            [10829, -2838, -1115, -8339, 15817, 2696, -837, 680, 11939],
        ),
        BuiltinMatrix::new(
            pentax!(IST_DL2_PEF),
            0,
            0,
            [10504, -2439, -1189, -8603, 16208, 2531, -1022, 863, 12242],
        ),
        BuiltinMatrix::new(
            pentax!(IST_DS_PEF),
            0,
            0,
            [10371, -2333, -1206, -8688, 16231, 2602, -1230, 1116, 11282],
        ),
        BuiltinMatrix::new(
            pentax!(K10D_PEF),
            0,
            0,
            [9566, -2863, -803, -7170, 15172, 2112, -818, 803, 9705],
        ),
        BuiltinMatrix::new(
            pentax!(K1_PEF),
            0,
            0,
            [8566, -2746, -1201, -3612, 12204, 1550, -893, 1680, 6264],
        ),
        BuiltinMatrix::new(
            pentax!(K1_MKII_PEF),
            0,
            0,
            [8596, -2981, -639, -4202, 12046, 2431, -685, 1424, 6122],
        ),
        BuiltinMatrix::new(
            pentax!(K100D_PEF),
            127,
            3950,
            [11095, -3157, -1324, -8377, 15834, 2720, -1108, 947, 11688],
        ),
        BuiltinMatrix::new(
            pentax!(K100D_SUPER_PEF),
            0,
            0,
            [11095, -3157, -1324, -8377, 15834, 2720, -1108, 947, 11688],
        ),
        BuiltinMatrix::new(
            pentax!(K110D_PEF),
            0,
            0,
            [11095, -3157, -1324, -8377, 15834, 2720, -1108, 947, 11688],
        ),
        BuiltinMatrix::new(
            pentax!(K20D_PEF),
            0,
            0,
            [9427, -2714, -868, -7493, 16092, 1373, -2199, 3264, 7180],
        ),
        BuiltinMatrix::new(
            pentax!(K200D_PEF),
            0,
            0,
            [9186, -2678, -907, -8693, 16517, 2260, -1129, 1094, 8524],
        ),
        BuiltinMatrix::new(
            pentax!(K2000_PEF),
            0,
            0,
            [9730, -2989, -970, -8527, 16258, 2381, -1060, 970, 8362],
        ),
        BuiltinMatrix::new(
            pentax!(KR_PEF),
            0,
            0,
            [9895, -3077, -850, -5304, 13035, 2521, -883, 1768, 6936],
        ),
        BuiltinMatrix::new(
            pentax!(K3_PEF),
            0,
            0,
            [8542, -2581, -1144, -3995, 12301, 1881, -863, 1514, 5755],
        ),
        BuiltinMatrix::new(
            pentax!(K3_II_PEF),
            0,
            0,
            [9251, -3817, -1069, -4627, 12667, 2175, -798, 1660, 5633],
        ),
        BuiltinMatrix::new(
            pentax!(K3_MKIII_PEF),
            0,
            0,
            [8571, -2590, -1148, -3995, 12301, 1881, -1052, 1844, 7013],
        ),
        BuiltinMatrix::new(
            pentax!(K5_PEF),
            0,
            0,
            [8713, -2833, -743, -4342, 11900, 2772, -722, 1543, 6247],
        ),
        BuiltinMatrix::new(
            pentax!(K5_II_PEF),
            0,
            0,
            [8435, -2549, -1130, -3995, 12301, 1881, -989, 1734, 6591],
        ),
        BuiltinMatrix::new(
            pentax!(K5_IIS_PEF),
            0,
            0,
            [8170, -2725, -639, -4440, 12017, 2744, -771, 1465, 6599],
        ),
        BuiltinMatrix::new(
            pentax!(K7_PEF),
            0,
            0,
            [9142, -2947, -678, -8648, 16967, 1663, -2224, 2898, 8615],
        ),
        BuiltinMatrix::new(
            pentax!(K70_PEF),
            0,
            0,
            [8766, -3149, -747, -3976, 11943, 2292, -517, 1259, 5552],
        ),
        BuiltinMatrix::new(
            pentax!(KM_PEF),
            0,
            0,
            [9730, -2989, -970, -8527, 16258, 2381, -1060, 970, 8362],
        ),
        BuiltinMatrix::new(
            pentax!(KX_PEF),
            0,
            0,
            [8843, -2837, -625, -5025, 12644, 2668, -411, 1234, 7410],
        ),
        BuiltinMatrix::new(
            pentax!(KS1_PEF),
            0,
            0,
            [7989, -2511, -1137, -3882, 12350, 1689, -862, 1524, 6444],
        ),
        BuiltinMatrix::new(
            pentax!(KS2_PEF),
            0,
            0,
            [8662, -3280, -798, -3928, 11771, 2444, -586, 1232, 6054],
        ),
        BuiltinMatrix::new(
            pentax!(KP_PEF),
            0,
            0,
            [8617, -3228, -1034, -4674, 12821, 2044, -803, 1577, 5728],
        ),
        BuiltinMatrix::new(
            pentax!(PENTAX_645D_PEF),
            0,
            0x3e00,
            [10646, -3593, -1158, -3329, 11699, 1831, -667, 2874, 6287],
        ),
        BuiltinMatrix::new(
            ricoh!(PENTAX_645Z_PEF),
            0,
            0x3fff,
            [9519, -3591, -664, -4074, 11725, 2671, -624, 1501, 6653],
        ),
    ];
}

#[derive(Debug)]
pub(crate) struct PefFile {
    reader: Rc<Viewer>,
    type_id: OnceCell<TypeId>,
    container: OnceCell<tiff::Container>,
    thumbnails: OnceCell<ThumbnailStorage>,
    #[cfg(feature = "probe")]
    probe: Option<crate::Probe>,
}

impl PefFile {
    pub fn factory(reader: Rc<Viewer>) -> RawFileHandle {
        RawFileHandleType::new(PefFile {
            reader,
            type_id: OnceCell::new(),
            container: OnceCell::new(),
            thumbnails: OnceCell::new(),
            #[cfg(feature = "probe")]
            probe: None,
        })
    }
}

impl RawFileImpl for PefFile {
    #[cfg(feature = "probe")]
    probe_imp!();

    fn identify_id(&self) -> TypeId {
        *self.type_id.get_or_init(|| {
            if let Some(maker_note) = self.maker_note_ifd() {
                if let Some(id) = maker_note.uint_value(exif::MNOTE_PENTAX_MODEL_ID) {
                    log::debug!("Pentax model ID: {:x} ({})", id, id);
                    return PENTAX_MODEL_ID_MAP
                        .get(&id)
                        .copied()
                        .unwrap_or(pentax!(UNKNOWN));
                } else {
                    log::error!("Pentax model ID tag not found");
                }
            }
            let container = self.container.get().unwrap();
            tiff::identify_with_exif(container, &MAKE_TO_ID_MAP).unwrap_or(pentax!(UNKNOWN))
        })
    }

    fn container(&self) -> &dyn RawContainer {
        self.container.get_or_init(|| {
            // XXX we should be faillible here.
            let view = Viewer::create_view(&self.reader, 0).expect("Created view");
            let mut container = tiff::Container::new(
                view,
                vec![
                    (tiff::IfdType::Main, None),
                    (tiff::IfdType::Other, None),
                    (tiff::IfdType::Other, None),
                ],
                self.type_(),
            );
            container.load(None).expect("PEF container error");
            probe!(
                self.probe,
                "raw.container.endian",
                &format!("{:?}", container.endian())
            );
            container
        })
    }

    fn thumbnails(&self) -> &ThumbnailStorage {
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

            ThumbnailStorage::with_thumbnails(thumbnails)
        })
    }

    fn ifd(&self, ifd_type: tiff::IfdType) -> Option<&Dir> {
        self.container();
        let container = self.container.get().unwrap();
        match ifd_type {
            tiff::IfdType::Main | tiff::IfdType::Raw => container.directory(0),
            tiff::IfdType::Exif => container.exif_dir(),
            tiff::IfdType::MakerNote => container.mnote_dir(),
            _ => None,
        }
    }

    fn load_rawdata(&self, _skip_decompress: bool) -> Result<RawImage> {
        self.container();
        let container = self.container.get().unwrap();
        self.ifd(tiff::IfdType::Raw)
            .ok_or(Error::NotFound)
            .and_then(|dir| tiff::tiff_get_rawdata(container, dir, self.type_()))
            .map(|mut rawdata| {
                if let Some(mnote) = self.ifd(tiff::IfdType::MakerNote) {
                    let user_crop = mnote
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
                                probe!(self.probe, "pef.user_crop", true);
                                Some(Rect::new(pt, sz))
                            } else {
                                None
                            }
                        });
                    let aspect_ratio =
                        mnote
                            .value::<u8>(exif::MNOTE_PENTAX_ASPECT_RATIO)
                            .and_then(|v| {
                                probe!(self.probe, "pef.aspect_ratio", v);
                                match v {
                                    0 => Some(AspectRatio(4, 3)),
                                    1 => Some(AspectRatio(3, 2)),
                                    2 => Some(AspectRatio(16, 9)),
                                    3 => Some(AspectRatio(1, 1)),
                                    _ => None,
                                }
                            });
                    rawdata.set_user_crop(user_crop, aspect_ratio);
                    rawdata.set_active_area(Some(Rect {
                        x: 0,
                        y: 0,
                        width: rawdata.width(),
                        height: rawdata.height(),
                    }));
                    if let Some(blacks) = mnote.uint_value_array(exif::MNOTE_PENTAX_BLACK_POINT) {
                        probe!(self.probe, "pef.blacks", "true");
                        rawdata.set_blacks(utils::to_quad(&blacks));
                    }
                    if let Some(white) = mnote.uint_value(exif::MNOTE_PENTAX_WHITELEVEL) {
                        probe!(self.probe, "pef.whites", "true");
                        rawdata.set_whites([white as u16; 4]);
                    } else if let Some((black, white)) = MATRICES
                        .iter()
                        .find(|m| m.camera == self.type_id())
                        .map(|m| (m.black, m.white))
                    {
                        if white != 0 {
                            probe!(self.probe, "pef.whites.static", "true");
                            rawdata.set_whites([white; 4]);
                        }
                        // If black isn't already set.
                        if rawdata.blacks()[0] == 0 {
                            probe!(self.probe, "pef.blacks.static", "true");
                            rawdata.set_blacks([black; 4]);
                        }
                    }
                    if let Some(wb) = mnote.uint_value_array(exif::MNOTE_PENTAX_WHITE_BALANCE) {
                        // ExifTool and the MNOTE_PENTAX_TAG_NAMES have this as WhitePoint
                        let g = wb[1] as f64;
                        let wb = [g / wb[0] as f64, 1.0, g / wb[3] as f64];
                        rawdata.set_as_shot_neutral(&wb);
                        probe!(self.probe, "pef.wb", "true");
                    }
                }
                probe!(
                    self.probe,
                    "pef.compression",
                    &format!("{:?}", rawdata.compression())
                );
                if let Some(data8) = rawdata.data8() {
                    let huffman = self
                        .ifd(tiff::IfdType::MakerNote)
                        .and_then(|mnote| mnote.entry(exif::MNOTE_PENTAX_HUFFMAN_TABLE))
                        .and_then(|e| e.value_array::<u8>(container.endian()));
                    let huffman = huffman
                        .as_ref()
                        .map(|huffman| (huffman.as_slice(), container.endian()));
                    probe!(self.probe, "pef.compression.huffman", huffman.is_some());
                    if let Ok(image) = decompress::decompress(
                        data8,
                        huffman,
                        rawdata.width() as usize,
                        rawdata.height() as usize,
                    ) {
                        rawdata.set_data16(image);
                        rawdata.set_data_type(DataType::Raw);
                        rawdata.set_compression(tiff::Compression::None);
                    }
                }
                rawdata
            })
    }

    fn get_builtin_colour_matrix(&self) -> Result<Vec<f64>> {
        self.builtin_colour_matrix(&*MATRICES)
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
