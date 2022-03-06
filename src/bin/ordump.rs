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

use log::{info, LevelFilter};
use simple_logger::SimpleLogger;

use libopenraw::ifd;
use libopenraw::raw_file_from_file;

pub fn main() {
    // XXX extract from the arguments the log level
    SimpleLogger::new()
        .with_module_level("mp4parse", LevelFilter::Error)
        .init()
        .unwrap();

    let mut args: Vec<String> = std::env::args().collect();
    args.remove(0);
    for name in args {
        process_file(&name);
    }
}

fn process_file(p: &str) {
    let rawfile = raw_file_from_file(p, None);

    info!("Dumping {}", p);

    match rawfile {
        Ok(ref rawfile) => {
            println!("Raw type: {:?}", rawfile.type_());
            println!("Vendor id: {}", rawfile.vendor_id());
            println!("Type id: {}", rawfile.type_id());

            let sizes = rawfile.thumbnail_sizes();
            println!("Thumbnail sizes: {:?}", &sizes);
            for size in sizes {
                let thumb = rawfile.thumbnail(size);
                match thumb {
                    Ok(ref thumb) => {
                        println!("\tThumbnail size: {} x {}", thumb.width, thumb.height);
                        println!("\tFormat: {:?}", thumb.data_type);
                        println!("\tSize: {} bytes", thumb.data.len());
                    }
                    Err(err) => {
                        println!("Failed to fetch preview for {}: {}", size, err);
                    }
                }
            }

            let exif_ifd = rawfile.ifd(ifd::Type::Exif);
            println!("Has Exif: {}", exif_ifd.is_some());
            if let Some(exif_ifd) = exif_ifd {
                println!("Number of exif entries {}", exif_ifd.num_entries());
            }
        }
        Err(err) => {
            println!("Failed to open raw file: {}", err);
        }
    }
}
