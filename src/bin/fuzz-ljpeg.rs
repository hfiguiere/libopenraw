#[macro_use]
extern crate afl;

use libopenraw::LJpeg;

/// Fuzz the Ljpeg decompressor. Use test-ljpeg-decompress to debug the crashes.

fn main() {
    fuzz!(|data: &[u8]| {
        let mut io = std::io::Cursor::new(data);
        let mut decompressor = LJpeg::new();
        let _ = decompressor.decompress(&mut io);
    });
}
