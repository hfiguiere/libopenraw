// SPDX-License-Identifier: LGPL-3.0-or-later
use libopenraw::LJpeg;

/// Test the LJpeg decrompression from the command line
/// This is used for the output of the `fuzz-ljpeg` fuzz target.

fn main() {
    let args: Vec<String> = std::env::args().collect();
    assert!(args.len() > 1, "Incorrect number of arguments");

    let mut decompressor = LJpeg::new();

    let io = std::fs::File::open(&args[1]);
    assert!(io.is_ok());
    let mut io = std::io::BufReader::new(io.unwrap());
    let _ = decompressor.discard_decompress(&mut io).map_err(|e| {
        println!("Error decoding: {}", e);
        e
    });
}
