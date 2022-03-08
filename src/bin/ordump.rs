/*
 * libopenraw - bin/ordump.rs
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

use getopts::Options;
use log::{info, LevelFilter};
use simple_logger::SimpleLogger;

use libopenraw::ifd;
use libopenraw::ifd::Ifd;
use libopenraw::{raw_file_from_file, DataType, Thumbnail};

pub fn main() {
    let args: Vec<String> = std::env::args().collect();

    let mut opts = Options::new();
    opts.optflag("d", "", "Debug");
    opts.optflag("t", "", "Extract thumbnails");

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

    for name in matches.free.iter() {
        process_file(name, extract_thumbnails);
    }
}

fn save_thumbnail(p: &str, thumb: &Thumbnail) {
    use std::io::Write;

    match thumb.data_type {
        DataType::Jpeg => {
            if let Some(stem) = std::path::PathBuf::from(p)
                .file_stem()
                .and_then(|s| s.to_str())
            {
                let filename = format!("{}_{}x{}", stem, thumb.width, thumb.height);
                let thumbnail = std::path::PathBuf::from(filename).with_extension("jpg");
                let mut f = std::fs::File::create(&thumbnail).expect("Couldn't open file");
                let amount = f.write(&thumb.data).expect("Couldn't write thumbnail");
                println!("Written {:?}: {} bytes", thumbnail, amount);
            }
        }
        _ => {
            println!("Unsupported format {:?}", thumb.data_type);
        }
    }
}

fn process_file(p: &str, extract_thumbnails: bool) {
    let rawfile = raw_file_from_file(p, None);

    info!("Dumping {}", p);

    match rawfile {
        Ok(ref rawfile) => {
            println!("Raw type: {:?}", rawfile.type_());
            println!("Vendor id: {}", rawfile.vendor_id());
            println!("Type id: {:?}", rawfile.type_id());

            let sizes = rawfile.thumbnail_sizes();
            println!("Thumbnail sizes: {:?}", &sizes);
            for size in sizes {
                let thumb = rawfile.thumbnail(size);
                match thumb {
                    Ok(ref thumb) => {
                        println!("\tThumbnail size: {} x {}", thumb.width, thumb.height);
                        println!("\tFormat: {:?}", thumb.data_type);
                        println!("\tSize: {} bytes", thumb.data.len());

                        if extract_thumbnails {
                            save_thumbnail(p, thumb);
                        }
                    }
                    Err(err) => {
                        println!("Failed to fetch preview for {}: {}", size, err);
                    }
                }
            }

            let exif_ifd = rawfile.ifd(ifd::Type::Exif);
            println!("Has Exif: {}", exif_ifd.is_some());
            if let Some(exif_ifd) = exif_ifd {
                println!("Number of Exif entries {}", exif_ifd.num_entries());
            }
            let maker_note = rawfile.ifd(ifd::Type::MakerNote);
            if let Some(maker_note) = maker_note {
                println!("Number of MakerNote entries {}", maker_note.num_entries());
            }
        }
        Err(err) => {
            println!("Failed to open raw file: {}", err);
        }
    }
}
