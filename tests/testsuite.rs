// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - test/testsuite.rs
 *
 * Copyright (C) 2022-2023 Hubert Figuière
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

use std::path::Path;

use serde::Deserialize;
use serde_xml_rs::{de::Deserializer, from_reader};

use libopenraw::metadata::Value;
use libopenraw::{Bitmap, DataType, Ifd, RawFile, Type, TypeId};
use libopenraw_testing::{raw_checksum, Results, Test};

trait TestRun {
    fn run(&self, rawfile: &dyn RawFile, filename: &str) -> u32;
}

/// Test for RAW
fn raw_test(results: &Results, rawfile: &dyn RawFile) -> u32 {
    let mut count = 0;

    let rawdata = rawfile.raw_data(false);
    assert_eq!(
        rawdata.is_ok(),
        results.raw_data_type.is_some(),
        "Expected Raw data wasn't found"
    );

    // no raw data, bail out
    if rawdata.is_err() {
        return count;
    }

    let rawdata = rawdata.unwrap();

    // RAW data type
    if let Some(ref raw_data_type) = results.raw_data_type {
        count += 1;
        assert_eq!(
            DataType::from(raw_data_type.as_str()),
            rawdata.data_type(),
            "Incorrect type for Raw data"
        );
    }
    // RAW data size
    if let Some(raw_data_size) = results.raw_data_size {
        count += 1;
        assert_eq!(
            raw_data_size as usize,
            rawdata.data_size(),
            "Incorrect Raw data size"
        );
    }

    // RAW dimensions
    if let Some(ref dims) = results.raw_data_dimensions {
        count += 1;
        assert_eq!(dims.len(), 2, "Incorrect number of Raw dimensions");
        assert_eq!(
            dims,
            &[rawdata.width(), rawdata.height()],
            "Incorrect dimensions"
        );
    }

    // RAW active area
    if let Some(ref raw_data_active_area) = results.raw_data_active_area {
        count += 1;
        assert_eq!(raw_data_active_area.len(), 4, "Incorrect active area");
        let active_area = rawdata.active_area();
        assert!(active_area.is_some(), "No active area found");
        let active_area = active_area.unwrap();
        assert_eq!(
            raw_data_active_area,
            &active_area.to_vec(),
            "Incorrect active area"
        );
    }

    // CFA pattern
    if let Some(ref raw_cfa_pattern) = results.raw_cfa_pattern {
        count += 1;
        assert_eq!(
            &rawdata.mosaic_pattern().to_string(),
            raw_cfa_pattern,
            "Incorrect CFA pattern"
        );
    }

    // RAW black and white
    if let Some(ref raw_min_value) = results.raw_min_value {
        count += 1;
        assert_eq!(
            &rawdata.blacks().to_vec(),
            raw_min_value,
            "Incorrect black point"
        );
    }
    if let Some(ref raw_max_value) = results.raw_max_value {
        count += 1;
        assert_eq!(
            &rawdata.whites().to_vec(),
            raw_max_value,
            "Incorrect white point"
        );
    }

    // RAW data checksum. It's not even a md5.
    if let Some(raw_md5) = results.raw_md5 {
        count += 1;
        let buf = if rawdata.data_type() == DataType::CompressedRaw {
            let buf = rawdata.data8();
            assert!(buf.is_some(), "Compressed Raw data not found");
            buf.unwrap()
        } else {
            let buf = rawdata.data16_as_u8();
            assert!(buf.is_some(), "16-bits Raw data not found");
            buf.unwrap()
        };
        let r = raw_checksum(buf);
        assert_eq!(raw_md5, r, "Incorrect Raw data checksum");
    }
    count
}

fn thumbnail_test(results: &Results, rawfile: &dyn RawFile) -> u32 {
    let mut count = 0;
    // Check the number of thumbnails
    if let Some(thumb_num) = results.thumb_num {
        count += 1;
        let thumbnail_sizes = rawfile.thumbnail_sizes();
        assert_eq!(
            thumb_num as usize,
            thumbnail_sizes.len(),
            "Different number of thumbnails"
        );
    }

    if let Some(ref sizes) = results.thumb_sizes {
        count += 1;
        let thumbnail_sizes = rawfile.thumbnail_sizes();
        assert_eq!(
            thumbnail_sizes.len(),
            sizes.len(),
            "Mismatch number of thumbnails"
        );
        for (index, size) in sizes.iter().enumerate() {
            assert_eq!(
                size, &thumbnail_sizes[index],
                "Incorrect size for thumbnail {index}"
            );
        }
    }

    if let Some(ref thumb_formats) = results.thumb_formats {
        count += 1;
        let thumbnails = rawfile.thumbnails();
        let formats: Vec<DataType> = thumb_formats.split(' ').map(DataType::from).collect();

        assert_eq!(
            thumbnails.sizes.len(),
            formats.len(),
            "Mismatch number of thumbnail format"
        );
        for (index, thumbnail) in thumbnails.thumbnails.iter().enumerate() {
            assert_eq!(
                thumbnail.1.data_type, formats[index],
                "Incorrect data type for thumbnail {index}"
            );
        }
    }

    if let Some(ref data_sizes) = results.thumb_data_sizes {
        count += 1;
        let thumbnails = rawfile.thumbnails();
        assert_eq!(
            thumbnails.thumbnails.len(),
            data_sizes.len(),
            "Mismatch number of thumbnail data sizes"
        );
        for (index, thumbnail) in thumbnails.thumbnails.iter().enumerate() {
            assert_eq!(
                thumbnail.1.data_size(),
                data_sizes[index] as u64,
                "Incorrect data size for thumbnail {index}"
            );
        }
    }

    if let Some(ref md5s) = results.thumb_md5 {
        count += 1;
        let thumbnails = rawfile.thumbnails();
        assert_eq!(
            thumbnails.thumbnails.len(),
            md5s.len(),
            "Mismatch number of thumbnail checksum"
        );
        for (index, thumbnail_desc) in thumbnails.thumbnails.iter().enumerate() {
            let thumbnail = rawfile
                .thumbnail(thumbnail_desc.0)
                .expect("Thumbnail not found");
            let buf = thumbnail.data8().unwrap();
            let r = raw_checksum(buf);
            assert_eq!(r, md5s[index], "Incorrect checksum for thumbnail {index}");
        }
    }

    // XXX todo

    count
}

impl TestRun for Results {
    fn run(&self, rawfile: &dyn RawFile, filename: &str) -> u32 {
        let mut count = 0;
        // Check RAW type
        if let Some(ref raw_type) = self.raw_type {
            count += 1;
            assert_eq!(
                Type::from(raw_type.as_str()),
                rawfile.type_(),
                "Incorrect Raw file type"
            );

            // Make sure idenfication from content works.
            let file = Box::new(std::io::BufReader::new(
                std::fs::File::open(filename).expect("Couldn't open the file"),
            ));
            let rawfile = libopenraw::rawfile_from_io(file, None)
                .unwrap_or_else(|_| panic!("Couldn't load raw file {}", filename));
            assert_eq!(
                Type::from(raw_type.as_str()),
                rawfile.type_(),
                "Incorrect Raw file type identified"
            );
        }
        // Check RAW file ID
        if let Some(raw_type_id) = self.raw_type_id {
            count += 1;
            assert_eq!(
                TypeId::from(raw_type_id),
                rawfile.type_id(),
                "Incorrect Raw file TypeID"
            );
        }

        if let Some(exif_make) = &self.exif_make {
            let make = rawfile.metadata_value(&"Exif.Image.Make".to_string());
            assert!(make.is_some());
            let make = make.as_ref().and_then(Value::string);
            assert!(make.is_some());
            let make = make.unwrap();
            assert_eq!(&make, exif_make);
        }

        if let Some(exif_model) = &self.exif_model {
            let model = rawfile.metadata_value(&"Exif.Image.Model".to_string());
            assert!(model.is_some());
            let model = model.as_ref().and_then(Value::string);
            assert!(model.is_some());
            let model = model.unwrap();
            assert_eq!(&model, exif_model);
        }

        if let Some(meta_orientation) = self.meta_orientation {
            let orientation = rawfile.orientation();
            assert_eq!(meta_orientation, orientation);
        }

        // Check the MakerNote count
        if let Some(maker_note_count) = self.maker_note_count {
            count += 1;
            let maker_note = rawfile.maker_note_ifd();
            match maker_note_count {
                -1 => assert!(maker_note.is_none(), "Expected to have no MakerNote"),
                c if c < -1 => unreachable!(),
                _ => {
                    let maker_note = maker_note.unwrap();
                    assert_eq!(
                        maker_note.num_entries(),
                        maker_note_count as usize,
                        "Incorrect MakerNote count"
                    );
                }
            }
        }
        // Check MakerNote ID
        if let Some(ref maker_note_id) = self.maker_note_id {
            count += 1;
            let maker_note_id = maker_note_id.as_bytes();
            let maker_note = rawfile.maker_note_ifd();
            assert!(maker_note.is_some(), "Expected MakerNote");
            let maker_note = maker_note.unwrap();
            assert_eq!(
                &maker_note.id()[0..maker_note_id.len()],
                maker_note_id,
                "Incorrect MakerNote ID"
            );
        }

        count += thumbnail_test(self, rawfile);
        count += raw_test(self, rawfile);
        count
    }
}

#[derive(Debug, Deserialize, PartialEq)]
#[serde(rename = "testsuite")]
struct TestSuite {
    #[serde(rename = "$value")]
    tests: Vec<Test>,
    #[serde(skip)]
    stats: TestStats,
}

#[derive(Debug, Deserialize, PartialEq)]
#[serde(rename = "test")]
struct Override {
    name: String,
    file: Option<String>,
    source: Option<String>,
    results: Option<Results>,
}

#[derive(Debug, Deserialize, PartialEq)]
#[serde(rename = "testsuite")]
struct Overrides {
    #[serde(rename = "$value")]
    tests: Vec<Override>,
}

impl TestSuite {
    fn run_test(test: &Test) {
        let rawfile = libopenraw::rawfile_from_file(test.file.clone(), None);
        match rawfile {
            Ok(rawfile) => {
                print!("Test '{}'", &test.name);
                let count = test.results.run(rawfile.as_ref(), &test.file);
                println!(" produced {count} results");
            }
            Err(err) => println!("Test '{}' skipped ({}): {}", &test.name, test.file, err),
        }
    }

    fn run(&mut self) {
        self.tests.iter().for_each(TestSuite::run_test);
    }

    /// Load the overrides to, notably, change the local path of the files.
    fn load_overrides<P>(&mut self, path: P)
    where
        P: AsRef<Path>,
    {
        if let Ok(file) = std::fs::File::open(path) {
            let overrides: Overrides = from_reader(file).expect("Deserialization of overrides");
            for override_ in overrides.tests {
                let name = &override_.name;
                if let Some(ref file) = override_.file {
                    let index = self.tests.iter().position(|t| &t.name == name);
                    if let Some(index) = index {
                        self.tests[index].file = file.to_string();
                    }
                }
            }
        }
    }
}

/// Test statistics
#[derive(Default, Debug, PartialEq)]
struct TestStats {
    passed: u32,
    failed: u32,
    skipped: u32,
}

fn load_testsuite<P>(path: P) -> Option<TestSuite>
where
    P: AsRef<Path>,
{
    let file = std::fs::File::open(path).expect("Failed to open");
    // By default serde will trim the whitespace at the end of CDATA.
    // We need it not to for the testsuite.
    let config = serde_xml_rs::ParserConfig::new().trim_whitespace(false);
    let event_reader = serde_xml_rs::EventReader::new_with_config(file, config);
    let testsuite =
        TestSuite::deserialize(&mut Deserializer::new(event_reader)).expect("Deserialization");

    Some(testsuite)
}

#[test]
fn test_regression_suite() {
    use log::LevelFilter;
    use simple_logger::SimpleLogger;
    SimpleLogger::new()
        .with_module_level("serde_xml_rs", LevelFilter::Error)
        .with_module_level("mp4parse", LevelFilter::Error)
        .with_module_level("libopenraw", LevelFilter::Error)
        .init()
        .unwrap();

    let testsuite = load_testsuite("testsuite/testsuite.xml");

    assert!(testsuite.is_some(), "Expected testsuite to load");
    let mut testsuite = testsuite.unwrap();

    testsuite.load_overrides("testsuite/testsuite.xml.overrides");

    testsuite.run();

    println!(
        "Test suite, passed: {} failed: {} skipped: {}",
        testsuite.stats.passed, testsuite.stats.failed, testsuite.stats.skipped
    );
}
