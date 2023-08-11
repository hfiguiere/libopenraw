// SPDX-License-Identifier: LGPL-3.0-or-later
#[macro_use]
extern crate afl;

fn main() {
    fuzz!(|data: &[u8]| {
        let io = Box::new(std::io::Cursor::new(data.to_vec()));
        let _ = libopenraw::rawfile_from_io(io, None).and_then(|rawfile| {
            let _ = rawfile.type_id();
            let sizes = rawfile.thumbnail_sizes();
            for size in sizes {
                let _ = rawfile.thumbnail(*size)?;
            }

            let _ = rawfile.raw_data(false)?;

            Ok(())
        });
    });
}
