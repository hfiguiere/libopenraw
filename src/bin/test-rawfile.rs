fn main() {
    let args: Vec<String> = std::env::args().collect();
    assert!(args.len() > 1, "Incorrect number of arguments");

    let _ = libopenraw::rawfile_from_file(&args[1], None)
        .and_then(|rawfile| {
            let _ = rawfile.type_id();
            let sizes = rawfile.thumbnail_sizes();
            for size in sizes {
                let _ = rawfile.thumbnail(*size)?;
            }

            let _ = rawfile.raw_data(false)?;

            Ok(())
        })
        .map_err(|e| {
            println!("Error decoding raw file {e}");
            e
        });
}
