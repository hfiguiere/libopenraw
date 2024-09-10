// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - bin/orrender.rs
 *
 * Copyright (C) 2023 Hubert Figuière
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
use log::LevelFilter;
use simple_logger::SimpleLogger;

use libopenraw::{rawfile_from_file, Bitmap, RenderingOptions};

pub fn main() {
    let args: Vec<String> = std::env::args().collect();

    let mut opts = Options::new();
    opts.optflag("d", "", "Debug");
    opts.optopt("o", "", "Set output file name", "OUTPUT");

    let matches = match opts.parse(&args[1..]) {
        Ok(m) => m,
        Err(f) => panic!("{}", f.to_string()),
    };

    let output = matches.opt_str("o");

    println!("output {output:?}");
    if matches.free.is_empty() {
        println!("orrender [-o OUTPUT] file");
        return;
    }

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

    for name in matches.free.iter() {
        process_file(name, output.as_ref()).expect("Error processing");
    }
}

fn process_file(p: &str, output: Option<&String>) -> libopenraw::Result<()> {
    if let Ok(rawfile) = rawfile_from_file(p, None) {
        log::info!("Rendering raw file {}", p);

        // Default options are Colour stage + SRgb.
        let options = RenderingOptions::default();
        let rawdata = rawfile.raw_data(false)?;
        let rendered_image = rawdata.rendered_image(options)?;
        if let Some(output) = output {
            let width = rendered_image.width();
            let height = rendered_image.height();
            if let Some(data16) = rendered_image.data16() {
                let scale = u8::MAX as f64 / u16::MAX as f64;
                let data8 = data16
                    .iter()
                    .map(|v| (*v as f64 * scale) as u8)
                    .collect::<Vec<u8>>();
                log::debug!(
                    "orrender rgb(u8) at 1000. 1000: [{}, {}, {}]",
                    data8[1000 * 1000 * 3],
                    data8[1000 * 1000 * 3 + 1],
                    data8[1000 * 1000 * 3 + 2]
                );
                image::save_buffer_with_format(
                    output,
                    &data8,
                    width,
                    height,
                    image::ColorType::Rgb8,
                    image::ImageFormat::Png,
                )
                .expect("Failed to save to PNG");
            }
        }
    }

    Ok(())
}
