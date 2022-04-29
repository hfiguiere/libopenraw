/*
 * libopenraw - dng.rs
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

//! Adobe DNG support.

use std::collections::HashMap;
use std::rc::Rc;

use once_cell::unsync::OnceCell;

use crate::bitmap::{Bitmap, Rect};
use crate::camera_ids::{
    adobe, apple, blackmagic, dji, gopro, hasselblad, leica, nokia, pentax, ricoh, samsung, sigma,
    vendor, xiaoyi, zeiss,
};
use crate::container::RawContainer;
use crate::decompress;
use crate::io::Viewer;
use crate::rawfile::ReadAndSeek;
use crate::thumbnail;
use crate::tiff;
use crate::tiff::{exif, Ifd};
use crate::{DataType, Dump, Error, RawData, RawFile, RawFileImpl, Result, Type, TypeId};

lazy_static::lazy_static! {
    /// Make to TypeId map for DNG files.
    static ref MAKE_TO_ID_MAP: tiff::MakeToIdMap = HashMap::from([
        ( "PENTAX 645Z        ", TypeId(vendor::PENTAX, ricoh::PENTAX_645Z_DNG) ),
        ( "PENTAX 645D        ", TypeId(vendor::PENTAX, pentax::PENTAX_645D_DNG) ),
        ( "PENTAX K10D        ", TypeId(vendor::PENTAX, pentax::K10D_DNG) ),
        ( "PENTAX K20D        ", TypeId(vendor::PENTAX, pentax::K20D_DNG) ),
        ( "PENTAX Q           ", TypeId(vendor::PENTAX, pentax::Q_DNG) ),
        ( "PENTAX K200D       ", TypeId(vendor::PENTAX, pentax::K200D_DNG) ),
        ( "PENTAX K2000       ", TypeId(vendor::PENTAX, pentax::K2000_DNG) ),
        ( "PENTAX Q10         ", TypeId(vendor::PENTAX, pentax::Q10_DNG) ),
        ( "PENTAX Q7          ", TypeId(vendor::PENTAX, pentax::Q7_DNG) ),
        ( "PENTAX Q-S1        ", TypeId(vendor::PENTAX, pentax::QS1_DNG) ),
        ( "PENTAX K-x         ", TypeId(vendor::PENTAX, pentax::KX_DNG) ),
        ( "PENTAX K-r         ", TypeId(vendor::PENTAX, pentax::KR_DNG) ),
        ( "PENTAX K-01        ", TypeId(vendor::PENTAX, pentax::K01_DNG) ),
        ( "PENTAX K-1         ", TypeId(vendor::PENTAX, pentax::K1_DNG) ),
        ( "PENTAX K-1 Mark II ", TypeId(vendor::PENTAX, pentax::K1_MKII_DNG) ),
        ( "PENTAX K10D        ", TypeId(vendor::PENTAX, pentax::K10D_DNG) ),
        ( "PENTAX K-30        ", TypeId(vendor::PENTAX, pentax::K30_DNG) ),
        ( "PENTAX K-5         ", TypeId(vendor::PENTAX, pentax::K5_DNG) ),
        ( "PENTAX K-5 II      ", TypeId(vendor::PENTAX, pentax::K5_II_DNG) ),
        ( "PENTAX K-5 II s    ", TypeId(vendor::PENTAX, pentax::K5_IIS_DNG) ),
        ( "PENTAX K-50        ", TypeId(vendor::PENTAX, pentax::K50_DNG) ),
        ( "PENTAX K-500       ", TypeId(vendor::PENTAX, pentax::K500_DNG) ),
        ( "PENTAX K-3         ", TypeId(vendor::PENTAX, pentax::K3_DNG) ),
        ( "PENTAX K-3 II      ", TypeId(vendor::PENTAX, pentax::K3_II_DNG) ),
        ( "PENTAX K-3 Mark III             ", TypeId(vendor::PENTAX,
                                                     pentax::K3_II_DNG) ),
        ( "PENTAX K-7         ", TypeId(vendor::PENTAX, pentax::K7_DNG) ),
        ( "PENTAX K-70        ", TypeId(vendor::PENTAX, pentax::K70_DNG) ),
        ( "PENTAX K-S1        ", TypeId(vendor::PENTAX, pentax::KS1_DNG) ),
        ( "PENTAX K-S2        ", TypeId(vendor::PENTAX, pentax::KS2_DNG) ),
        ( "PENTAX KP          ", TypeId(vendor::PENTAX, pentax::KP_DNG) ),
        ( "PENTAX MX-1            ", TypeId(vendor::PENTAX,
                                            pentax::MX1_DNG) ),
        ( "R9 - Digital Back DMR", TypeId(vendor::LEICA, leica::DMR) ),
        ( "M8 Digital Camera", TypeId(vendor::LEICA, leica::M8) ),
        ( "M9 Digital Camera", TypeId(vendor::LEICA, leica::M9) ),
        ( "M Monochrom", TypeId(vendor::LEICA, leica::M_MONOCHROM) ),
        ( "LEICA M (Typ 240)", TypeId(vendor::LEICA, leica::M_TYP240) ),
        ( "LEICA M MONOCHROM (Typ 246)", TypeId(vendor::LEICA, leica::M_MONOCHROM_TYP246) ),
        ( "LEICA M10", TypeId(vendor::LEICA, leica::M10) ),
        ( "LEICA M10-P", TypeId(vendor::LEICA, leica::M10P) ),
        ( "LEICA M10-D", TypeId(vendor::LEICA, leica::M10D) ),
        ( "LEICA M10-R", TypeId(vendor::LEICA, leica::M10R) ),
        ( "LEICA M10 MONOCHROM", TypeId(vendor::LEICA, leica::M10_MONOCHROM) ),
        ( "LEICA M11", TypeId(vendor::LEICA, leica::M11) ),
        ( "LEICA X1               ", TypeId(vendor::LEICA, leica::X1) ),
        ( "LEICA X2", TypeId(vendor::LEICA, leica::X2) ),
        ( "Leica S2", TypeId(vendor::LEICA, leica::S2) ),
        ( "LEICA X VARIO (Typ 107)", TypeId(vendor::LEICA, leica::X_VARIO) ),
        ( "LEICA X (Typ 113)", TypeId(vendor::LEICA, leica::X_TYP113) ),
        ( "LEICA SL (Typ 601)", TypeId(vendor::LEICA, leica::SL_TYP601) ),
        ( "LEICA SL2", TypeId(vendor::LEICA, leica::SL2) ),
        ( "LEICA T (Typ 701)", TypeId(vendor::LEICA, leica::T_TYP701) ),
        ( "LEICA TL2", TypeId(vendor::LEICA, leica::TL2) ),
        ( "LEICA Q (Typ 116)", TypeId(vendor::LEICA, leica::Q_TYP116) ),
        ( "LEICA Q2", TypeId(vendor::LEICA, leica::Q2) ),
        ( "LEICA CL", TypeId(vendor::LEICA, leica::CL) ),
        ( "LEICA SL2-S", TypeId(vendor::LEICA, leica::SL2S) ),
        ( "LEICA Q2 MONO", TypeId(vendor::LEICA, leica::Q2_MONOCHROM) ),
        ( "GR DIGITAL 2   ", TypeId(vendor::RICOH, ricoh::GR2) ),
        ( "GR                                                             ",
           TypeId(vendor::RICOH, ricoh::GR) ),
        ( "GR II                                                          ",
           TypeId(vendor::RICOH, ricoh::GRII) ),
        ( "RICOH GR III       ", TypeId(vendor::RICOH, ricoh::GRIII) ),
        ( "RICOH GR IIIx      ", TypeId(vendor::RICOH, ricoh::GRIIIX) ),
        ( "GXR            ", TypeId(vendor::RICOH, ricoh::GXR) ),
        ( "GXR A16                                                        ",
           TypeId(vendor::RICOH, ricoh::GXR_A16) ),
        ( "RICOH GX200    ",
           TypeId(vendor::RICOH, ricoh::GX200) ),
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
        ( "iPhone SE", TypeId(vendor::APPLE, apple::IPHONE_SE) ),
        ( "iPhone XS", TypeId(vendor::APPLE, apple::IPHONE_XS) ),
        ( "Blackmagic Pocket Cinema Camera", TypeId(vendor::BLACKMAGIC,
                                                    blackmagic::POCKET_CINEMA) ),
        ( "SIGMA fp", TypeId(vendor::SIGMA, sigma::FP) ),
        ( "SIGMA fp L", TypeId(vendor::SIGMA, sigma::FP_L) ),
        ( "L1D-20c", TypeId(vendor::HASSELBLAD, hasselblad::L1D_20C) ),
        ( "HERO5 Black", TypeId(vendor::GOPRO, gopro::HERO5_BLACK) ),
        ( "HERO6 Black", TypeId(vendor::GOPRO, gopro::HERO6_BLACK) ),
        ( "HERO7 Black", TypeId(vendor::GOPRO, gopro::HERO7_BLACK) ),
        ( "HERO8 Black", TypeId(vendor::GOPRO, gopro::HERO8_BLACK) ),
        ( "HERO9 Black", TypeId(vendor::GOPRO, gopro::HERO9_BLACK) ),
        ( "HERO10 Black", TypeId(vendor::GOPRO, gopro::HERO10_BLACK) ),
        ( "ZX1", TypeId(vendor::ZEISS, zeiss::ZX1) ),
        ( "FC220", TypeId(vendor::DJI, dji::FC220) ),
        ( "FC350", TypeId(vendor::DJI, dji::FC350) ),
        ( "FC6310", TypeId(vendor::DJI, dji::FC6310) ),
        ( "FC7303", TypeId(vendor::DJI, dji::FC7303) ),
        ( "DJI Osmo Action", TypeId(vendor::DJI, dji::OSMO_ACTION) ),
        ( "Lumia 1020", TypeId(vendor::NOKIA, nokia::LUMIA_1020) ),
//        ( 0, TypeId(vendor::ADOBE, adobe::DNG_GENERIC) ),
    ]);
}

pub(crate) struct DngFile {
    reader: Rc<Viewer>,
    type_id: OnceCell<TypeId>,
    container: OnceCell<tiff::Container>,
    thumbnails: OnceCell<Vec<(u32, thumbnail::ThumbDesc)>>,
}

impl DngFile {
    pub fn factory(reader: Box<dyn ReadAndSeek>) -> Box<dyn RawFile> {
        let viewer = Viewer::new(reader, 0);
        Box::new(DngFile {
            reader: viewer,
            type_id: OnceCell::new(),
            container: OnceCell::new(),
            thumbnails: OnceCell::new(),
        })
    }

    fn decompress(rawdata: RawData) -> Result<RawData> {
        match rawdata.data_type() {
            DataType::Raw => Ok(rawdata),
            DataType::CompressedRaw => match rawdata.compression() {
                tiff::Compression::LJpeg => {
                    if let Some(data) = rawdata.data8() {
                        let mut decompressor = decompress::LJpeg::new();
                        let mut io = std::io::Cursor::new(data);
                        decompressor.decompress(&mut io)
                            .map(|mut rawdata2| {
                                rawdata2.set_active_area(rawdata.active_area().cloned());

                                rawdata2
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
            let mut container = tiff::Container::new(view, vec![tiff::IfdType::Main], self.type_());
            container.load(None).expect("IFD container error");
            container
        })
    }

    fn thumbnails(&self) -> &Vec<(u32, thumbnail::ThumbDesc)> {
        self.thumbnails.get_or_init(|| {
            self.container();
            let container = self.container.get().unwrap();
            let mut thumbnails = tiff::tiff_thumbnails(container);
            self.maker_note_ifd().and_then(|mnote| {
                if mnote.id() == "Leica6" {
                    // File with Leica6 MakerNote (Leica M Typ-240) have a
                    // larger preview in the MakerNote
                    mnote.entry(exif::MNOTE_LEICA_PREVIEW_IMAGE).map(|e| {
                        container.add_thumbnail_from_entry(e, mnote.mnote_offset, &mut thumbnails)
                    })
                } else {
                    None
                }
            });

            thumbnails
        })
    }

    fn ifd(&self, ifd_type: tiff::IfdType) -> Option<Rc<tiff::Dir>> {
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
                            .and_then(|subifds| {
                                subifds.iter().find(|subdir| subdir.is_primary()).cloned()
                            })
                    }
                })
            }
            tiff::IfdType::Exif => self.container.get().unwrap().exif_dir(),
            tiff::IfdType::MakerNote => self.container.get().unwrap().mnote_dir(),
            _ => None,
        }
    }

    fn load_rawdata(&self) -> Result<RawData> {
        self.ifd(tiff::IfdType::Raw)
            .ok_or_else(|| {
                log::error!("DNG: couldn't find CFA ifd");
                Error::NotFound
            })
            .and_then(|dir| {
                self.container();
                let container = self.container.get().unwrap();
                tiff::tiff_get_rawdata(container, &dir)
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

                        rawdata
                    })
                    .map_err(|err| {
                        log::error!("Couldn't find DNG raw data {}", err);
                        err
                    })
            })
            .and_then(Self::decompress)
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
    fn print_dump(&self, indent: u32) {
        dump_println!(indent, "<DNG File>");
        {
            let indent = indent + 1;
            self.container().print_dump(indent);
        }
        dump_println!(indent, "</DNG File>");
    }
}
