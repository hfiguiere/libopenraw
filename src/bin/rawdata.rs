//! This tool is used to extract the uncompressed rawdata from Rawfiles.
//! Like the `-n` option in ordiag.

use std::io::Write;

use libopenraw::Bitmap;

fn main() {
    let args: Vec<String> = std::env::args().collect();
    assert!(args.len() > 1, "Incorrect number of arguments");

    let _ = libopenraw::rawfile_from_file(&args[1], None).and_then(|rawfile| {
        // always skip decompression
        let rawdata = rawfile.raw_data(true)?;
        let mut f = std::fs::File::create("rawdata.raw")?;
        let _ = f.write(rawdata.data8().unwrap())?;

        Ok(())
    });
}
