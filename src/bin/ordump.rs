use libopenraw::raw_file_from_file;

pub fn main() {
    let mut args: Vec<String> = std::env::args().collect();
    args.remove(0);
    println!("args: {:?}", args);
    for name in args {
        process_file(&name);
    }
}

fn process_file(p: &str) {
    let rawfile = raw_file_from_file(p, None);

    println!("Dumping {}", p);

    match rawfile {
        Ok(ref rawfile) => {
            println!("Raw type: {:?}", rawfile.type_());
            println!("Vendor id: {}", rawfile.vendor_id());
            println!("Type id: {}", rawfile.type_id());

            let sizes = rawfile.thumbnail_sizes();
            println!("Thumbnail sizes: {:?}", &sizes);
            for size in sizes {
                let thumb = rawfile.thumbnail(size);
                match thumb {
                    Ok(ref thumb) => {
                        println!("\tThumbnail size: {} x {}", thumb.width, thumb.height);
                        println!("\tFormat: {:?}", thumb.data_type);
                        println!("\tSize: {} bytes", thumb.data.len());
                    }
                    Err(err) => {
                        println!("Failed to fetch preview for {}: {}", size, err);
                    }
                }
            }
        }
        Err(err) => {
            println!("Failed to open raw file: {}", err);
        }
    }
}
