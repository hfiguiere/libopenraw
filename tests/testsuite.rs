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

use std::fmt::Display;
use std::path::Path;
use std::str::FromStr;

use serde::{de::Error, Deserialize};
use serde_xml_rs::{de::Deserializer, from_reader};

use libopenraw::metadata::Value;
use libopenraw::{Bitmap, DataType, Ifd, RawFile, Type, TypeId};

#[derive(Clone, Debug, Deserialize, PartialEq)]
#[serde(rename_all = "camelCase")]
struct Results {
    raw_type: Option<String>,
    raw_type_id: Option<u32>,
    /// MakeNoteCount can be -1 for an error (expected)
    maker_note_count: Option<i32>,
    maker_note_id: Option<String>,
    exif_make: Option<String>,
    exif_model: Option<String>,
    thumb_num: Option<u32>,
    #[serde(deserialize_with = "from_list")]
    #[serde(default)]
    thumb_sizes: Option<Vec<u32>>,
    thumb_formats: Option<String>, // XXX split array
    #[serde(deserialize_with = "from_list")]
    #[serde(default)]
    thumb_data_sizes: Option<Vec<u32>>,
    #[serde(deserialize_with = "from_list")]
    #[serde(default)]
    thumb_md5: Option<Vec<u16>>,
    raw_data_type: Option<String>,
    raw_data_size: Option<u32>,
    #[serde(deserialize_with = "from_list")]
    #[serde(default)]
    raw_data_dimensions: Option<Vec<u32>>,
    #[serde(deserialize_with = "from_list")]
    #[serde(default)]
    raw_data_active_area: Option<Vec<u32>>,
    raw_cfa_pattern: Option<String>,
    #[serde(deserialize_with = "from_list")]
    #[serde(default)]
    raw_min_value: Option<Vec<u16>>,
    #[serde(deserialize_with = "from_list")]
    #[serde(default)]
    raw_max_value: Option<Vec<u16>>,
    raw_md5: Option<u32>,
    meta_orientation: Option<u32>,
}

/// Deserialize a space separated list on numbers to a vector.
fn from_list<'de, D, T>(deserializer: D) -> Result<Option<Vec<T>>, D::Error>
where
    D: serde::Deserializer<'de>,
    T: FromStr,
    <T as FromStr>::Err: Display,
{
    let s: String = Deserialize::deserialize(deserializer)?;
    let v: Vec<&str> = s.split(' ').collect();
    let mut ints = vec![];
    for num in v {
        let n = num.parse::<T>().map_err(D::Error::custom)?;
        ints.push(n);
    }
    Ok(Some(ints))
}

impl Results {
    /// CRC checksum for the RAW data (8 bits only)
    fn raw_checksum(buf: &[u8]) -> u16 {
        // This is the same algorithm as used in the C++ implementation
        let crc = crc::Crc::<u16>::new(&crc::CRC_16_IBM_3740);
        let mut digest = crc.digest();
        digest.update(buf);

        digest.finalize()
    }

    /// Test for RAW
    fn raw_test(&self, rawfile: &dyn RawFile) -> u32 {
        let mut count = 0;

        let rawdata = rawfile.raw_data(false);
        assert_eq!(
            rawdata.is_ok(),
            self.raw_data_type.is_some(),
            "Expected Raw data wasn't found"
        );

        // no raw data, bail out
        if rawdata.is_err() {
            return count;
        }

        let rawdata = rawdata.unwrap();

        // RAW data type
        if let Some(ref raw_data_type) = self.raw_data_type {
            count += 1;
            assert_eq!(
                DataType::from(raw_data_type.as_str()),
                rawdata.data_type(),
                "Incorrect type for Raw data"
            );
        }
        // RAW data size
        if let Some(raw_data_size) = self.raw_data_size {
            count += 1;
            assert_eq!(
                raw_data_size as usize,
                rawdata.data_size(),
                "Incorrect Raw data size"
            );
        }

        // RAW dimensions
        if let Some(ref dims) = self.raw_data_dimensions {
            count += 1;
            assert_eq!(dims.len(), 2, "Incorrect number of Raw dimensions");
            assert_eq!(
                dims,
                &[rawdata.width(), rawdata.height()],
                "Incorrect dimensions"
            );
        }

        // RAW active area
        if let Some(ref raw_data_active_area) = self.raw_data_active_area {
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
        if let Some(ref raw_cfa_pattern) = self.raw_cfa_pattern {
            count += 1;
            assert_eq!(
                &rawdata.mosaic_pattern().to_string(),
                raw_cfa_pattern,
                "Incorrect CFA pattern"
            );
        }

        // RAW black and white
        if let Some(ref raw_min_value) = self.raw_min_value {
            count += 1;
            assert_eq!(
                &rawdata.blacks().to_vec(),
                raw_min_value,
                "Incorrect black point"
            );
        }
        if let Some(ref raw_max_value) = self.raw_max_value {
            count += 1;
            assert_eq!(
                &rawdata.whites().to_vec(),
                raw_max_value,
                "Incorrect white point"
            );
        }

        // RAW data checksum. It's not even a md5.
        if let Some(raw_md5) = self.raw_md5 {
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
            let r = Self::raw_checksum(buf);
            assert_eq!(raw_md5, r as u32, "Incorrect Raw data checksum");
        }
        count
    }

    fn thumbnail_test(&self, rawfile: &dyn RawFile) -> u32 {
        let mut count = 0;
        // Check the number of thumbnails
        if let Some(thumb_num) = self.thumb_num {
            count += 1;
            let thumbnail_sizes = rawfile.thumbnail_sizes();
            assert_eq!(
                thumb_num as usize,
                thumbnail_sizes.len(),
                "Different number of thumbnails"
            );
        }

        if let Some(ref sizes) = self.thumb_sizes {
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

        if let Some(ref thumb_formats) = self.thumb_formats {
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

        if let Some(ref data_sizes) = self.thumb_data_sizes {
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

        if let Some(ref md5s) = self.thumb_md5 {
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
                let r = Self::raw_checksum(buf);
                assert_eq!(r, md5s[index], "Incorrect checksum for thumbnail {index}");
            }
        }

        // XXX todo

        count
    }

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

        count += self.thumbnail_test(rawfile);
        count += self.raw_test(rawfile);
        count
    }
}

#[derive(Clone, Debug, Deserialize, PartialEq)]
#[serde(rename = "test")]
struct Test {
    name: String,
    file: String,
    source: Option<String>,
    results: Results,
}

impl Test {
    fn run(&self) {
        let rawfile = libopenraw::rawfile_from_file(self.file.clone(), None);
        match rawfile {
            Ok(rawfile) => {
                print!("Test '{}'", &self.name);
                let count = self.results.run(rawfile.as_ref(), &self.file);
                println!(" produced {count} results");
            }
            Err(err) => println!("Test '{}' skipped ({}): {}", &self.name, self.file, err),
        }
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
    fn run(&mut self) {
        for test in &self.tests {
            test.run();
        }
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
