// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - libopenraw-testing.rs
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

//! Generate a test case from a raw file.

use std::io::Write;
use std::path::Path;

use getopts::Options;

use libopenraw::{rawfile_from_file, Type};
use libopenraw_testing::{make_results, Test};

pub fn main() {
    let args: Vec<String> = std::env::args().collect();

    let mut opts = Options::new();
    opts.optopt("o", "", "Output", "OUTPUT");

    let matches = match opts.parse(&args[1..]) {
        Ok(m) => m,
        Err(f) => panic!("{}", f.to_string()),
    };

    let output = matches.opt_str("o");

    let file = matches.free.first().expect("Input file required");

    // XXX normalize in case of relative.
    let rawfile = rawfile_from_file(file, None).expect("Raw file opening failed");
    let filename = Path::new(file)
        .file_name()
        .map(|s| s.to_string_lossy())
        .unwrap_or_default();
    let name = format!(
        "{} - {filename}",
        <Type as Into<String>>::into(rawfile.type_())
    );

    let results = make_results(&*rawfile);

    let t = Test {
        name,
        file: file.to_string(),
        source: None,
        results,
    };

    let xml_serialized = t.serialize_to_xml();
    if let Some(output) = output {
        println!("writing to {output}");
        let mut file = std::fs::File::create(output).expect("Couldn't open file");
        file.write_all(&xml_serialized.into_bytes())
            .expect("Writing failed");
    } else {
        println!("{xml_serialized}");
    }
}
