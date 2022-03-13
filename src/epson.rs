//! Epson ERF support.

use std::collections::HashMap;
use std::rc::Rc;

use once_cell::unsync::OnceCell;

use crate::bitmap;
use crate::camera_ids;
use crate::camera_ids::vendor;
use crate::container::GenericContainer;
use crate::ifd;
use crate::ifd::{exif, Ifd};
use crate::io::Viewer;
use crate::rawfile::ReadAndSeek;
use crate::thumbnail;
use crate::{DataType, Error, RawData, RawFile, RawFileImpl, Result, Type, TypeId};

/// The MakerNote tag names. It's actually the same as Olympus.
pub(crate) use crate::olympus::MNOTE_TAG_NAMES;

use crate::colour::BuiltinMatrix;

lazy_static::lazy_static! {
    /// EPSON built-in colour matrices
    static ref MATRICES: [BuiltinMatrix; 2] = [
        BuiltinMatrix::new(
            TypeId(vendor::EPSON, camera_ids::epson::RD1), 0, 0,
            [ 6827, -1878, -732, -8429, 16012, 2564, -704, 592, 7145 ]
        ),
        BuiltinMatrix::new(
            TypeId(vendor::EPSON, camera_ids::epson::RD1S), 0, 0,
            [ 6827, -1878, -732, -8429, 16012, 2564, -704, 592, 7145 ]
        ),
    ];
}

lazy_static::lazy_static! {
    /// Make to TypeId map for ERF files.
    static ref MAKE_TO_ID_MAP: ifd::MakeToIdMap = HashMap::from([
        ( "R-D1", TypeId(vendor::EPSON, camera_ids::epson::RD1) ),
        ( "R-D1s", TypeId(vendor::EPSON, camera_ids::epson::RD1S) ),
    ]);
}

/// ERF RAW file support
pub(crate) struct ErfFile {
    reader: Rc<Viewer>,
    container: OnceCell<ifd::Container>,
    thumbnails: OnceCell<HashMap<u32, thumbnail::ThumbDesc>>,
    cfa: OnceCell<Option<Rc<ifd::Dir>>>,
}

impl ErfFile {
    pub fn factory(reader: Box<dyn ReadAndSeek>) -> Box<dyn RawFile> {
        let viewer = Viewer::new(reader);
        Box::new(ErfFile {
            reader: viewer,
            container: OnceCell::new(),
            thumbnails: OnceCell::new(),
            cfa: OnceCell::new(),
        })
    }

    /// Return a lazily loaded `ifd::Container`
    fn container(&self) -> &ifd::Container {
        self.container.get_or_init(|| {
            // XXX we should be faillible here.
            let view = Viewer::create_view(&self.reader, 0).expect("Created view");
            let mut container = ifd::Container::new(view);
            container.load().expect("IFD container error");
            container
        })
    }

    /// Return the CFA dir
    fn cfa_dir(&self) -> Option<&Rc<ifd::Dir>> {
        self.cfa
            .get_or_init(|| ifd::tiff_locate_cfa_ifd(self.container()))
            .as_ref()
    }
}

impl RawFileImpl for ErfFile {
    fn identify_id(&self) -> TypeId {
        if let Some(id) = ifd::identify_with_exif(self.container(), &MAKE_TO_ID_MAP) {
            id
        } else {
            TypeId(0, 0)
        }
    }

    fn thumbnails(&self) -> &std::collections::HashMap<u32, thumbnail::ThumbDesc> {
        self.thumbnails.get_or_init(|| {
            let mut thumbnails = ifd::tiff_thumbnails(self.container());
            self.maker_note_ifd().and_then(|mnote| {
                mnote.entry(exif::ERF_TAG_PREVIEW_IMAGE).map(|e| {
                    let mut data = Vec::from(e.data());
                    // The data start by 0xee instead of 0xff for a JPEG. Not sure why.
                    data[0] = 0xff;
                    let desc = thumbnail::ThumbDesc {
                        // It is 640x424 (3:2 aspect ratio)
                        width: 640,
                        height: 424,
                        data_type: DataType::Jpeg,
                        data: thumbnail::Data::Bytes(data),
                    };
                    thumbnails.insert(640, desc);
                })
            });

            thumbnails
        })
    }

    fn thumbnail_for_size(&self, size: u32) -> Result<thumbnail::Thumbnail> {
        let thumbnails = self.thumbnails();
        if let Some(desc) = thumbnails.get(&size) {
            self.container().make_thumbnail(desc)
        } else {
            log::warn!("Thumbnail size {} not found", size);
            Err(Error::NotFound)
        }
    }

    fn ifd(&self, ifd_type: ifd::Type) -> Option<Rc<ifd::Dir>> {
        // XXX todo
        match ifd_type {
            ifd::Type::Cfa => self.cfa_dir().cloned(),
            ifd::Type::Exif => self.container().exif_dir(),
            ifd::Type::MakerNote => self.container().mnote_dir(),
            _ => None,
        }
    }

    fn load_rawdata(&self) -> Result<RawData> {
        self.ifd(ifd::Type::Cfa)
            .ok_or_else(|| {
                log::error!("CFA not found");
                Error::NotFound
            })
            .and_then(|ref ifd| {
                ifd::tiff_get_rawdata(self.container(), ifd).map(|mut rawdata| {
                    self.maker_note_ifd().and_then(|mnote| {
                        mnote
                            .entry_cloned(
                                exif::MNOTE_EPSON_SENSORAREA,
                                &mut *self.container().borrow_view_mut(),
                            )
                            // the data type is `Undefined`
                            .and_then(|e| e.value_array::<u16>(mnote.endian()))
                            .or_else(|| {
                                log::error!("Failed to read sensor area");
                                None
                            })
                            .and_then(|a| {
                                if a.len() >= 4 {
                                    rawdata.set_active_area(Some(bitmap::Rect {
                                        x: a[0] as u32,
                                        y: a[1] as u32,
                                        width: a[2] as u32,
                                        height: a[3] as u32,
                                    }));
                                    Some(())
                                } else {
                                    None
                                }
                            })
                    });
                    rawdata
                })
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

impl RawFile for ErfFile {
    fn type_(&self) -> Type {
        Type::Erf
    }
}

#[cfg(test)]
mod test {}
