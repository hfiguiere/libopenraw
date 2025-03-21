// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - bin/identify.rs
 *
 * Copyright (C) 2022-2025 Hubert Figuière
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

use libopenraw::{rawfile_from_file, Type};

pub fn main() {
    let args: Vec<String> = std::env::args().collect();

    let mut opts = Options::new();
    opts.optflag("d", "", "Debug");
    opts.optopt("f", "", "Raw Format (for debugging purpose)", "FORMAT");

    let matches = match opts.parse(&args[1..]) {
        Ok(m) => m,
        Err(f) => panic!("{}", f.to_string()),
    };

    let loglevel = if matches.opt_present("d") {
        LevelFilter::Debug
    } else {
        LevelFilter::Error
    };

    let format = matches
        .opt_str("f")
        .map(|format| Type::from(format.as_str()));

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
        let rawfile = rawfile_from_file(&p, format);
        match rawfile {
            Ok(ref rawfile) => match rawfile.type_id() {
                Ok(type_id) => println!("{} {}", name, type_id),
                Err(err) => println!("Error getting type id {name}: {err}"),
            },
            Err(err) => {
                println!("Error opening file {name}: {err}");
            }
        }
    }
}
