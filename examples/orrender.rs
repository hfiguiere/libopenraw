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

use libopenraw::{rawfile_from_file, Bitmap, Image, RenderingOptions};

pub fn main() {
    let args: Vec<String> = std::env::args().collect();

    let mut opts = Options::new();
    opts.optflag("d", "", "Debug");
    opts.optopt("o", "", "set output file name", "OUTPUT");

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
        process_file(name, output.as_ref());
    }
}

fn process_file(p: &str, output: Option<&String>) -> libopenraw::Result<()> {
    if let Ok(rawfile) = rawfile_from_file(p, None) {
        log::info!("Rendering raw file {}", p);

        let options = RenderingOptions::default();
        let rendered_image = rawfile.rendered_image(options)?;
        if let Some(ref output) = output {
            println!(
                "output {} x {}",
                rendered_image.width(),
                rendered_image.height()
            );
            if let Some(data8) = rendered_image
                .data16()
                .map(|data16| data16.iter().map(|v| (*v >> 4) as u8).collect::<Vec<u8>>())
            {
                image::save_buffer_with_format(
                    output,
                    &data8,
                    rendered_image.width(),
                    rendered_image.height(),
                    image::ColorType::Rgb8,
                    image::ImageFormat::Png,
                )
                .expect("Failed to save to PNG");
            }
        }
    }

    Ok(())
}
