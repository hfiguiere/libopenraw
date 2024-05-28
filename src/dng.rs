// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - dng.rs
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

//! Adobe DNG support.

use std::collections::HashMap;
use std::rc::Rc;

use once_cell::unsync::OnceCell;

use crate::bitmap::{Bitmap, Rect};
use crate::camera_ids::{
    adobe, apple, blackmagic, dji, gopro, hasselblad, nokia, samsung, sigma, vendor, xiaoyi, zeiss,
};
use crate::container::RawContainer;
use crate::decompress;
use crate::io::Viewer;
use crate::leica;
use crate::pentax;
use crate::rawfile::{RawFileHandleType, ThumbnailStorage};
use crate::ricoh;
use crate::tiff;
use crate::tiff::{exif, Ifd};
use crate::utils;
use crate::{
    DataType, Dump, Error, RawFile, RawFileHandle, RawFileImpl, RawImage, Result, Type, TypeId,
};

lazy_static::lazy_static! {
    /// Make to TypeId map for DNG files.
    static ref MAKE_TO_ID_MAP: tiff::MakeToIdMap = HashMap::from([
        ricoh!( "PENTAX 645Z        ", PENTAX_645Z_DNG),
        pentax!("PENTAX 645D        ", PENTAX_645D_DNG),
        pentax!("PENTAX K10D        ", K10D_DNG),
        pentax!("PENTAX K20D        ", K20D_DNG),
        pentax!("PENTAX Q           ", Q_DNG),
        pentax!("PENTAX K200D       ", K200D_DNG),
        pentax!("PENTAX K2000       ", K2000_DNG),
        pentax!("PENTAX Q10         ", Q10_DNG),
        pentax!("PENTAX Q7          ", Q7_DNG),
        pentax!("PENTAX Q-S1        ", QS1_DNG),
        pentax!("PENTAX K-x         ", KX_DNG),
        pentax!("PENTAX K-r         ", KR_DNG),
        pentax!("PENTAX K-01        ", K01_DNG),
        pentax!("PENTAX K-1         ", K1_DNG),
        pentax!("PENTAX K-1 Mark II ", K1_MKII_DNG),
        pentax!("PENTAX K10D        ", K10D_DNG),
        pentax!("PENTAX K-30        ", K30_DNG),
        pentax!("PENTAX K-5         ", K5_DNG),
        pentax!("PENTAX K-5 II      ", K5_II_DNG),
        pentax!("PENTAX K-5 II s    ", K5_IIS_DNG),
        pentax!("PENTAX K-50        ", K50_DNG),
        pentax!("PENTAX K-500       ", K500_DNG),
        pentax!("PENTAX K-3         ", K3_DNG),
        pentax!("PENTAX K-3 II      ", K3_II_DNG),
        pentax!("PENTAX K-3 Mark III             ", K3_MKIII_DNG),
        pentax!("PENTAX K-3 Mark III Monochrome                                  ",
                K3_MKIII_MONO_DNG),
        pentax!("PENTAX K-7         ", K7_DNG),
        pentax!("PENTAX K-70        ", K70_DNG),
        pentax!("PENTAX K-S1        ", KS1_DNG),
        pentax!("PENTAX K-S2        ", KS2_DNG),
        pentax!("PENTAX KP          ", KP_DNG),
        pentax!("PENTAX MX-1            ", MX1_DNG),
        leica!("R9 - Digital Back DMR", DMR),
        leica!("M8 Digital Camera", M8),
        leica!("M9 Digital Camera", M9),
        leica!("M Monochrom", M_MONOCHROM),
        leica!("LEICA M (Typ 240)", M_TYP240),
        leica!("LEICA M MONOCHROM (Typ 246)", M_MONOCHROM_TYP246),
        leica!("LEICA M10", M10),
        leica!("LEICA M10-P", M10P),
        leica!("LEICA M10-D", M10D),
        leica!("LEICA M10-R", M10R),
        leica!("LEICA M10 MONOCHROM", M10_MONOCHROM),
        leica!("LEICA M11", M11),
        leica!("LEICA M11 Monochrom", M11_MONOCHROM),
        leica!("LEICA X1               ", X1),
        leica!("LEICA X2", X2),
        leica!("Leica S2", S2),
        leica!("LEICA X VARIO (Typ 107)", X_VARIO),
        leica!("LEICA X (Typ 113)", X_TYP113),
        leica!("LEICA SL (Typ 601)", SL_TYP601),
        leica!("LEICA SL2", SL2),
        leica!("LEICA SL3", SL3),
        leica!("LEICA T (Typ 701)", T_TYP701),
        leica!("LEICA TL2", TL2),
        leica!("LEICA Q (Typ 116)", Q_TYP116),
        leica!("LEICA Q2", Q2),
        leica!("LEICA Q3", Q3),
        leica!("LEICA CL", CL),
        leica!("LEICA SL2-S", SL2S),
        leica!("LEICA Q2 MONO", Q2_MONOCHROM),
        ricoh!( "GR DIGITAL 2   ", GR2),
        ricoh!( "GR                                                             ",
           GR),
        ricoh!( "GR II                                                          ",
           GRII),
        ricoh!( "RICOH GR III       ", GRIII),
        ricoh!( "RICOH GR IIIx      ", GRIIIX),
        ricoh!( "GXR            ", GXR),
        ricoh!( "GXR A16                                                        ",
           GXR_A16),
        ricoh!( "RICOH GX200    ",
           GX200),
        ( "SAMSUNG GX10       ", TypeId(vendor::SAMSUNG, samsung::GX10) ),
        ( "SAMSUNG GX20       ", TypeId(vendor::SAMSUNG, samsung::GX20) ),
        ( "Pro 815    ", TypeId(vendor::SAMSUNG, samsung::PRO815) ),
        ( "M1              ", TypeId(vendor::XIAOYI, xiaoyi::M1) ),
        ( "YDXJ 2", TypeId(vendor::XIAOYI, xiaoyi::YDXJ_2) ),
        ( "YIAC 3", TypeId(vendor::XIAOYI, xiaoyi::YIAC_3) ),
        ( "iPhone 6s Plus", TypeId(vendor::APPLE, apple::IPHONE_6SPLUS) ),
        ( "iPhone 7 Plus", TypeId(vendor::APPLE, apple::IPHONE_7PLUS) ),
        ( "iPhone 8", TypeId(vendor::APPLE, apple::IPHONE_8) ),
        ( "iPhone 12 Pro", TypeId(vendor::APPLE, apple::IPHONE_12_PRO) ),
        ( "iPhone 13 Pro", TypeId(vendor::APPLE, apple::IPHONE_13_PRO) ),
        ( "iPhone 14", TypeId(vendor::APPLE, apple::IPHONE_14) ),
        ( "iPhone 15 Pro", TypeId(vendor::APPLE, apple::IPHONE_15_PRO) ),
        ( "iPhone SE", TypeId(vendor::APPLE, apple::IPHONE_SE) ),
        ( "iPhone XS", TypeId(vendor::APPLE, apple::IPHONE_XS) ),
        ( "Blackmagic Pocket Cinema Camera", TypeId(vendor::BLACKMAGIC,
                                                    blackmagic::POCKET_CINEMA) ),
        ( "SIGMA fp", TypeId(vendor::SIGMA, sigma::FP) ),
        ( "SIGMA fp L", TypeId(vendor::SIGMA, sigma::FP_L) ),
        ( "L1D-20c", TypeId(vendor::HASSELBLAD, hasselblad::L1D_20C) ),
        ( "L2D-20c", TypeId(vendor::HASSELBLAD, hasselblad::L2D_20C) ),
        ( "HERO5 Black", TypeId(vendor::GOPRO, gopro::HERO5_BLACK) ),
        ( "HERO6 Black", TypeId(vendor::GOPRO, gopro::HERO6_BLACK) ),
        ( "HERO7 Black", TypeId(vendor::GOPRO, gopro::HERO7_BLACK) ),
        ( "HERO8 Black", TypeId(vendor::GOPRO, gopro::HERO8_BLACK) ),
        ( "HERO9 Black", TypeId(vendor::GOPRO, gopro::HERO9_BLACK) ),
        ( "HERO10 Black", TypeId(vendor::GOPRO, gopro::HERO10_BLACK) ),
        ( "ZX1", TypeId(vendor::ZEISS, zeiss::ZX1) ),
        ( "FC220", TypeId(vendor::DJI, dji::FC220) ),
        ( "FC350", TypeId(vendor::DJI, dji::FC350) ),
        ( "FC3582", TypeId(vendor::DJI, dji::FC3582) ),
        ( "FC6310", TypeId(vendor::DJI, dji::FC6310) ),
        ( "FC7303", TypeId(vendor::DJI, dji::FC7303) ),
        ( "DJI Osmo Action", TypeId(vendor::DJI, dji::OSMO_ACTION) ),
        ( "Lumia 1020", TypeId(vendor::NOKIA, nokia::LUMIA_1020) ),
//        ( 0, TypeId(vendor::ADOBE, adobe::DNG_GENERIC) ),
    ]);
}

#[derive(Debug)]
pub(crate) struct DngFile {
    reader: Rc<Viewer>,
    type_id: OnceCell<TypeId>,
    container: OnceCell<tiff::Container>,
    thumbnails: OnceCell<ThumbnailStorage>,
}

impl DngFile {
    pub fn factory(reader: Rc<Viewer>) -> RawFileHandle {
        RawFileHandleType::new(DngFile {
            reader,
            type_id: OnceCell::new(),
            container: OnceCell::new(),
            thumbnails: OnceCell::new(),
        })
    }

    fn decompress(mut rawdata: RawImage) -> Result<RawImage> {
        match rawdata.data_type() {
            DataType::Raw => Ok(rawdata),
            DataType::CompressedRaw => match rawdata.compression() {
                tiff::Compression::LJpeg => {
                    if let Some(data) = rawdata.data8() {
                        let mut decompressor = decompress::LJpeg::new();
                        let mut io = std::io::Cursor::new(data);
                        decompressor.decompress(&mut io).map(|buffer| {
                            rawdata.set_with_buffer(buffer);
                            rawdata.set_data_type(DataType::Raw);
                            rawdata
                        })
                    } else if rawdata.tile_data().is_some() {
                        let decompressor = decompress::TiledLJpeg::new();
                        decompressor.decompress(rawdata)
                    } else {
                        log::error!("No data to decompress LJPEG");
                        Ok(rawdata)
                    }
                }
                _ => {
                    log::error!(
                        "Unsupported compression for DNG: {:?}",
                        rawdata.compression()
                    );
                    Ok(rawdata)
                }
            },
            _ => {
                log::warn!("Unexpected data type for DNG: {:?}", rawdata.data_type());
                Ok(rawdata)
            }
        }
    }
}

impl RawFileImpl for DngFile {
    fn identify_id(&self) -> TypeId {
        *self.type_id.get_or_init(|| {
            self.container();
            let container = self.container.get().unwrap();
            tiff::identify_with_exif(container, &MAKE_TO_ID_MAP)
                .unwrap_or(TypeId(vendor::ADOBE, adobe::DNG_GENERIC))
        })
    }

    /// Return a lazily loaded `tiff::Container`
    fn container(&self) -> &dyn RawContainer {
        self.container.get_or_init(|| {
            // XXX we should be faillible here.
            let view = Viewer::create_view(&self.reader, 0).expect("Created view");
            let mut container =
                tiff::Container::new(view, vec![(tiff::IfdType::Main, None)], self.type_());
            container.load(None).expect("IFD container error");
            container
        })
    }

    fn thumbnails(&self) -> &ThumbnailStorage {
        self.thumbnails.get_or_init(|| {
            self.container();
            let container = self.container.get().unwrap();
            let mut thumbnails = tiff::tiff_thumbnails(container);
            self.maker_note_ifd().and_then(|mnote| {
                if mnote.id() == b"Leica6\0" {
                    // File with Leica6 MakerNote (Leica M Typ-240) have a
                    // larger preview in the MakerNote
                    mnote.entry(exif::MNOTE_LEICA_PREVIEW_IMAGE).map(|e| {
                        container.add_thumbnail_from_entry(e, mnote.mnote_offset, &mut thumbnails)
                    })
                } else {
                    None
                }
            });

            ThumbnailStorage::with_thumbnails(thumbnails)
        })
    }

    fn ifd(&self, ifd_type: tiff::IfdType) -> Option<&tiff::Dir> {
        self.container();
        match ifd_type {
            tiff::IfdType::Main => self.container.get().unwrap().directory(0),
            tiff::IfdType::Raw => {
                // This is the TIFF/EP way.
                // XXX eventually refactor for NEF
                self.ifd(tiff::IfdType::Main).and_then(|dir| {
                    // Leica Monochrom has the main IFD being primary.
                    if dir.is_primary() {
                        Some(dir)
                    } else {
                        dir.get_sub_ifds(self.container.get().unwrap())
                            .iter()
                            .find(|subdir| subdir.is_primary())
                    }
                })
            }
            tiff::IfdType::Exif => self.container.get().unwrap().exif_dir(),
            tiff::IfdType::MakerNote => self.container.get().unwrap().mnote_dir(),
            _ => None,
        }
    }

    fn load_rawdata(&self, skip_decompress: bool) -> Result<RawImage> {
        self.ifd(tiff::IfdType::Raw)
            .ok_or_else(|| {
                log::error!("DNG: couldn't find CFA ifd");
                Error::NotFound
            })
            .and_then(|dir| {
                self.container();
                let container = self.container.get().unwrap();
                tiff::tiff_get_rawdata(container, dir, self.type_())
                    .map(|mut rawdata| {
                        let active_area = dir
                            .entry(exif::DNG_TAG_ACTIVE_AREA)
                            .and_then(|e| e.uint_value_array(container.endian()))
                            // check the size of the array. Should be 4
                            .and_then(|a| if a.len() >= 4 { Some(a) } else { None })
                            .map(|a| Rect {
                                x: a[1],
                                y: a[0],
                                height: a[2],
                                width: a[3],
                            })
                            .or_else(|| {
                                Some(Rect {
                                    x: 0,
                                    y: 0,
                                    width: rawdata.width(),
                                    height: rawdata.height(),
                                })
                            });
                        rawdata.set_active_area(active_area);
                        if let Some(blacks) = dir.uint_value_array(exif::DNG_TAG_BLACK_LEVEL) {
                            rawdata.set_blacks(utils::to_quad(&blacks));
                        }
                        if let Some(whites) = dir.uint_value_array(exif::DNG_TAG_WHITE_LEVEL) {
                            rawdata.set_whites(utils::to_quad(&whites));
                        }
                        if let Some(as_shot_wb) = self
                            .main_ifd()
                            .and_then(|dir| dir.float_value_array(exif::DNG_TAG_AS_SHOT_NEUTRAL))
                        {
                            rawdata.set_as_shot_neutral(&as_shot_wb);
                        }
                        rawdata
                    })
                    .map_err(|err| {
                        log::error!("Couldn't find DNG raw data {}", err);
                        err
                    })
            })
            .and_then(|rawdata| {
                if !skip_decompress {
                    Self::decompress(rawdata)
                } else {
                    Ok(rawdata)
                }
            })
    }

    fn get_builtin_colour_matrix(&self) -> Result<Vec<f64>> {
        Err(Error::NotSupported)
    }
}

impl RawFile for DngFile {
    fn type_(&self) -> Type {
        Type::Dng
    }
}

impl Dump for DngFile {
    #[cfg(feature = "dump")]
    fn write_dump<W: std::io::Write + ?Sized>(&self, out: &mut W, indent: u32) {
        dump_writeln!(out, indent, "<DNG File>");
        {
            let indent = indent + 1;
            self.container();
            self.container.get().unwrap().write_dump(out, indent);
        }
        dump_writeln!(out, indent, "</DNG File>");
    }
}

dumpfile_impl!(DngFile);
