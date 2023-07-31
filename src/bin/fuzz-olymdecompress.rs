#[macro_use]
extern crate afl;

use libopenraw::decompress_olympus;

/// Fuzz the Olympus decompressor.

fn main() {
    fuzz!(|data: &[u8]| {
        let _ = decompress_olympus(data, 4100, 3084);
    });
}
