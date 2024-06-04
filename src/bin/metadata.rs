// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - bin/metadata.rs
 *
 * Copyright (C) 2023-2024 Hubert Figuière
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

use std::convert::TryInto;

use getopts::Options;
use log::LevelFilter;
use simple_logger::SimpleLogger;

use libopenraw::rawfile_from_file;
use libopenraw::tiff::exif::TagType;

pub fn main() {
    let args: Vec<String> = std::env::args().collect();

    let mut opts = Options::new();
    opts.optflag("d", "", "Debug");

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

    for name in matches.free.iter() {
        process_file(name);
    }
}

fn type_to_string(t: i16) -> String {
    let t: TagType = t.try_into().unwrap_or(TagType::Error_);
    <TagType as Into<&'static str>>::into(t).to_string()
}

fn process_file(p: &str) {
    if let Ok(rawfile) = rawfile_from_file(p, None) {
        log::info!("Metadata raw file {}", p);

        for metadata in rawfile.metadata() {
            println!(
                "{} ({}) => {:?}",
                metadata.0,
                type_to_string(metadata.2),
                metadata.1.into_string(false)
            );
        }
    }
}
