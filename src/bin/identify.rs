/*
 * libopenraw - bin/identify.rs
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

//! Just identify files. Meant to be used a test tool to see if
//! RAW files can be open.

use getopts::Options;
use log::LevelFilter;
use simple_logger::SimpleLogger;

use libopenraw::raw_file_from_file;

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
        let p = std::path::PathBuf::from(name);
        if !p.is_file() {
            continue;
        }
        let rawfile = raw_file_from_file(&p, None);
        match rawfile {
            Ok(ref rawfile) => {
                println!("{} {}", name, rawfile.type_id());
            }
            Err(err) => {
                println!("Error {}: {}", name, err);
            }
        }
    }
}
