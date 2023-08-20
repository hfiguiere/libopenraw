// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - libopenraw-testing.rs
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
use std::str::FromStr;

use serde::{de::Error, Deserialize};

use libopenraw::{metadata::Value, Bitmap, DataType, Ifd, RawFile, Rect};

/// CRC checksum for the RAW data (8 bits only)
pub fn raw_checksum(buf: &[u8]) -> u16 {
    // This is the same algorithm as used in the C++ implementation
    let crc = crc::Crc::<u16>::new(&crc::CRC_16_IBM_3740);
    let mut digest = crc.digest();
    digest.update(buf);

    digest.finalize()
}

#[derive(Clone, Debug, Deserialize, PartialEq)]
#[serde(rename_all = "camelCase")]
pub struct Results {
    pub raw_type: Option<String>,
    pub raw_type_id: Option<u32>,
    /// MakeNoteCount can be -1 for an error (expected)
    pub maker_note_count: Option<i32>,
    pub maker_note_id: Option<String>,
    pub exif_make: Option<String>,
    pub exif_model: Option<String>,
    pub thumb_num: Option<u32>,
    #[serde(deserialize_with = "from_list")]
    #[serde(default)]
    pub thumb_sizes: Option<Vec<u32>>,
    pub thumb_formats: Option<String>, // XXX split array
    #[serde(deserialize_with = "from_list")]
    #[serde(default)]
    pub thumb_data_sizes: Option<Vec<u32>>,
    #[serde(deserialize_with = "from_list")]
    #[serde(default)]
    pub thumb_md5: Option<Vec<u16>>,
    pub raw_data_type: Option<String>,
    pub raw_data_size: Option<u32>,
    #[serde(deserialize_with = "from_list")]
    #[serde(default)]
    pub raw_data_dimensions: Option<Vec<u32>>,
    #[serde(deserialize_with = "from_list")]
    #[serde(default)]
    pub raw_data_active_area: Option<Vec<u32>>,
    pub raw_cfa_pattern: Option<String>,
    #[serde(deserialize_with = "from_list")]
    #[serde(default)]
    pub raw_min_value: Option<Vec<u16>>,
    #[serde(deserialize_with = "from_list")]
    #[serde(default)]
    pub raw_max_value: Option<Vec<u16>>,
    pub raw_md5: Option<u16>,
    pub meta_orientation: Option<u32>,
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

#[derive(Clone, Debug, Deserialize, PartialEq)]
#[serde(rename = "test")]
pub struct Test {
    pub name: String,
    pub file: String,
    pub source: Option<String>,
    pub results: Results,
}

fn make_results(rawfile: &dyn RawFile) -> Results {
    let raw_type = Some(rawfile.type_().into());
    let raw_type_id = Some(rawfile.type_id().into());
    let exif_make = rawfile
        .metadata_value(&"Exif.Image.Make".to_string())
        .as_ref()
        .and_then(Value::string);
    let exif_model = rawfile
        .metadata_value(&"Exif.Image.Model".to_string())
        .as_ref()
        .and_then(Value::string);
    let meta_orientation = Some(rawfile.orientation());

    let maker_note = rawfile.maker_note_ifd();
    let maker_note_count = maker_note
        .map(|mnote| mnote.num_entries() as i32)
        .or(Some(-1));
    let maker_note_id = maker_note.and_then(|mnote| String::from_utf8(mnote.id().to_vec()).ok());

    let thumbnail_sizes = rawfile.thumbnail_sizes();
    let thumb_num = Some(thumbnail_sizes.len() as u32);
    let thumb_sizes = Some(thumbnail_sizes.to_vec());

    let thumbnails = rawfile.thumbnails();
    let thumb_formats = Some(
        thumbnails
            .thumbnails
            .iter()
            .map(|t| t.1.data_type.into())
            .collect::<Vec<String>>()
            .join(" "),
    );
    let thumb_data_sizes = Some(
        thumbnails
            .thumbnails
            .iter()
            .map(|t| t.1.data_size() as u32)
            .collect(),
    );
    let thumb_md5 = Some(
        thumbnails
            .thumbnails
            .iter()
            .flat_map(|t| {
                rawfile
                    .thumbnail(t.0)
                    .ok()
                    .and_then(|t| t.data8().map(raw_checksum))
            })
            .collect(),
    );

    let rawdata = rawfile.raw_data(false);
    let rawdata = rawdata.as_ref();
    let raw_data_type = rawdata.map(|rawdata| rawdata.data_type().into()).ok();
    let raw_data_size = rawdata.map(|rawdata| rawdata.data_size() as u32).ok();
    let raw_data_dimensions = rawdata
        .map(|rawdata| vec![rawdata.width(), rawdata.height()])
        .ok();
    let raw_data_active_area = rawdata
        .ok()
        .and_then(|rawdata| rawdata.active_area())
        .map(Rect::to_vec);
    let raw_cfa_pattern = rawdata
        .map(|rawdata| rawdata.mosaic_pattern().to_string())
        .ok();
    let raw_min_value = rawdata.map(|rawdata| rawdata.blacks().to_vec()).ok();
    let raw_max_value = rawdata.map(|rawdata| rawdata.whites().to_vec()).ok();
    let raw_md5 = rawdata
        .ok()
        .and_then(|rawdata| {
            if rawdata.data_type() == DataType::CompressedRaw {
                rawdata.data8()
            } else {
                rawdata.data16_as_u8()
            }
        })
        .map(raw_checksum);

    Results {
        raw_type,
        raw_type_id,
        exif_make,
        exif_model,
        meta_orientation,
        maker_note_count,
        maker_note_id,
        thumb_num,
        thumb_sizes,
        thumb_formats,
        thumb_data_sizes,
        thumb_md5,
        raw_data_type,
        raw_data_size,
        raw_data_dimensions,
        raw_data_active_area,
        raw_cfa_pattern,
        raw_min_value,
        raw_max_value,
        raw_md5,
    }
}
