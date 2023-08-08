// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - bin/ordiag.rs
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

use getopts::Options;
use log::{info, LevelFilter};
use simple_logger::SimpleLogger;

use libopenraw::metadata::Value;
use libopenraw::Bitmap;
use libopenraw::{
    rawfile_from_file, DataType, Error, Ifd, Image, RawFile, RawImage, Result, Thumbnail,
};

pub fn main() {
    let args: Vec<String> = std::env::args().collect();

    let mut opts = Options::new();
    opts.optflag("d", "", "Debug");
    opts.optflag("t", "", "Extract thumbnails");
    opts.optflag("R", "", "Extract Raw data");
    opts.optflag("n", "", "No decompression");

    let matches = match opts.parse(&args[1..]) {
        Ok(m) => m,
        Err(f) => panic!("{}", f.to_string()),
    };

    let loglevel = if matches.opt_present("d") {
        LevelFilter::Debug
    } else {
        LevelFilter::Error
    };
    SimpleLogger::new()
        .with_module_level("mp4parse", LevelFilter::Error)
        .with_module_level("libopenraw", loglevel)
        .init()
        .unwrap();

    let extract_thumbnails = matches.opt_present("t");
    let extract_raw = matches.opt_present("R");
    let skip_decompress = matches.opt_present("n");

    for name in matches.free.iter() {
        process_file(name, extract_thumbnails, extract_raw, skip_decompress);
    }
}

fn make_thumbnail_name(p: &str, thumb: &Thumbnail) -> Option<std::path::PathBuf> {
    std::path::PathBuf::from(p)
        .file_stem()
        .and_then(|s| s.to_str())
        .map(|stem| {
            std::path::PathBuf::from(format!("{}_{}x{}.jpg", stem, thumb.width(), thumb.height()))
        })
}

fn save_thumbnail(p: &str, thumb: &Thumbnail) {
    use std::io::Write;

    match thumb.data_type() {
        DataType::Jpeg => {
            if let Some(fname) = make_thumbnail_name(p, thumb) {
                if let Some(d) = thumb.data8() {
                    let mut f = std::fs::File::create(&fname).expect("Couldn't open file");
                    let amount = f.write(d).expect("Couldn't write thumbnail");
                    println!("Written {fname:?}: {amount} bytes");
                }
            }
        }
        _ => {
            println!("Unsupported format {:?}", thumb.data_type());
        }
    }
}

fn save_raw(p: &str, rawdata: &RawImage) -> Result<usize> {
    if let Some(stem) = std::path::PathBuf::from(p)
        .file_stem()
        .and_then(|s| s.to_str())
    {
        use std::io::Write;

        use byteorder::BigEndian;
        use byteorder::WriteBytesExt;

        let mut amount = 0;
        let raw = std::path::PathBuf::from(format!("{stem}_RAW.pgm"));
        if let Some(d) = rawdata.data16() {
            let mut f = std::fs::File::create(&raw)?;
            amount += f.write(b"P5\n")?;
            amount += f.write(format!("{} {}\n", rawdata.width(), rawdata.height()).as_bytes())?;
            amount += f.write(format!("{}\n", (1 << rawdata.bpc()) - 1).as_bytes())?;
            for b in d {
                f.write_u16::<BigEndian>(*b)?;
                amount += 2;
            }
            println!("Written Raw {raw:?}: {amount} bytes");
        }

        Ok(amount)
    } else {
        Err(Error::Unknown)
    }
}

fn extract_rawdata(p: &str, rawfile: &dyn RawFile, extract_raw: bool, skip_decompress: bool) {
    let before = std::time::Instant::now();
    let rawdata = rawfile.raw_data(skip_decompress);
    println!("Elapsed time: {:.2?}", before.elapsed());

    if let Ok(rawdata) = rawdata {
        println!("Found rawdata:");
        println!("\tFormat: {:?}", rawdata.data_type());
        println!("\tSize: {}x{}", rawdata.width(), rawdata.height());
        println!("\tActive area: {:?}", rawdata.active_area());
        println!("\tBayer Type: {:?}", rawdata.mosaic_pattern());
        let bpc = rawdata.bpc();
        println!("\tBpc: {bpc}");
        println!("\tWhites = {:?}", rawdata.whites(),);
        println!("\tBlacks = {:?}", rawdata.blacks());
        if rawdata.data_type() == DataType::CompressedRaw {
            if let Some(d) = rawdata.data8() {
                println!("\tRaw data: {} bytes", d.len());
            } else if let Some(d) = rawdata.tile_data() {
                println!("\tTiled raw data: {} tiles", d.len());
            } else {
                println!("\tMissing compressed raw data.");
            }
        } else if rawdata.data16().is_some() {
            println!("\tRaw data: {} bytes", rawdata.data_size());
        } else {
            println!("\tNo 16bits Raw data found.");
        }
        if rawdata.linearization_table().is_some() {
            println!("\tHas a linearization table.");
        }
        if let Ok((origin, _)) = rawfile.colour_matrix(1) {
            println!("\tColour Matrix Origin: {origin:?}");
        }
        if let Ok((_, matrix)) = rawfile.colour_matrix(1) {
            println!(
                "\tCalibration illuminant 1: {} ({:?})",
                rawfile.calibration_illuminant(1) as u32,
                rawfile.calibration_illuminant(1)
            );
            println!("\tColour matrix 1: {matrix:?}");
        }
        if let Ok((_, matrix)) = rawfile.colour_matrix(2) {
            println!(
                "\tCalibration illuminant 2: {} ({:?})",
                rawfile.calibration_illuminant(2) as u32,
                rawfile.calibration_illuminant(2)
            );
            println!("\tColour matrix 2: {matrix:?}");
        }
        if extract_raw {
            if let Err(err) = save_raw(p, &rawdata) {
                println!("Saving raw failed: {err}");
            }
        }
    } else {
        println!("Raw data not found");
    }
}

fn process_file(p: &str, extract_thumbnails: bool, extract_raw: bool, skip_decompress: bool) {
    let rawfile = rawfile_from_file(p, None);

    info!("Diags {}", p);

    match rawfile {
        Ok(ref rawfile) => {
            println!("Raw type: {:?}", rawfile.type_());
            println!("Vendor id: {}", rawfile.vendor_id());
            println!("Type id: {:?}", rawfile.type_id());
            if let Some(make) = rawfile
                .metadata_value(&"Exif.Image.Make".to_string())
                .as_ref()
                .and_then(Value::string)
            {
                println!("Make: {:?}", make);
            }
            if let Some(model) = rawfile
                .metadata_value(&"Exif.Image.Model".to_string())
                .as_ref()
                .and_then(Value::string)
            {
                println!("Model: {:?}", model);
            }
            if let Some(unique) = rawfile
                .metadata_value(&"Exif.Image.UniqueCameraModel".to_string())
                .as_ref()
                .and_then(Value::string)
            {
                println!("Unique Camera Model: {:?}", unique);
            }

            let sizes = rawfile.thumbnail_sizes();
            println!("Thumbnail sizes: {:?}", &sizes);
            for size in sizes {
                let thumb = rawfile.thumbnail(*size);
                match thumb {
                    Ok(ref thumb) => {
                        println!("\tThumbnail size: {} x {}", thumb.width(), thumb.height());
                        println!("\tFormat: {:?}", thumb.data_type());
                        println!(
                            "\tSize: {} bytes",
                            thumb.data8().map(|d| d.len()).unwrap_or(0)
                        );

                        if extract_thumbnails {
                            save_thumbnail(p, thumb);
                        }
                    }
                    Err(err) => {
                        println!("Failed to fetch preview for {size}: {err}");
                    }
                }
            }

            extract_rawdata(p, rawfile.as_ref(), extract_raw, skip_decompress);

            let exif_ifd = rawfile.exif_ifd();
            println!("Has Exif: {}", exif_ifd.is_some());
            if let Some(exif_ifd) = exif_ifd {
                println!("Number of Exif entries {}", exif_ifd.num_entries());
            }
            let maker_note = rawfile.maker_note_ifd();
            if let Some(maker_note) = maker_note {
                println!("Number of MakerNote entries {}", maker_note.num_entries());
            }
            let orientation = rawfile.orientation();
            println!("Orientation: {orientation}");
        }
        Err(err) => {
            println!("Failed to open raw file: {err}");
        }
    }
}

#[cfg(test)]
mod test {

    use super::make_thumbnail_name;
    use libopenraw::{DataType, Thumbnail};

    #[test]
    fn test_make_thumbnail_name() {
        let filename: &str = "samples/dng/iphone-13-pro_1.57+IMG_0445.DNG";
        let thumbnail = Thumbnail::new(100, 75, DataType::Jpeg, vec![100, 120, 130]);
        let n = make_thumbnail_name(filename, &thumbnail);
        assert_eq!(
            n,
            Some(std::path::PathBuf::from(
                "iphone-13-pro_1.57+IMG_0445_100x75.jpg"
            ))
        );
    }
}
