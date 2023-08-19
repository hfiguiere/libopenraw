// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - the_benchmark.rs
 *
 * Copyright (C) 2023 Hubert Figuière
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

#![doc = include_str!("../doc/benchmarks.md")]

const FILES: [&str; 12] = [
    "Apple/iPhone XS/IMG_1105.dng",
    "Canon/EOS 10D/CRW_7673.CRW",
    "Canon/EOS 20D/IMG_3893.CR2",
    "Canon/Canon EOS R5/Canon_EOS_R5_CRAW_ISO_100_nocrop_nodual.CR3",
    "Epson/R-D1/_EPS0672.ERF",
    "Fujifilm/X-Pro1/DSCF2131.RAF",
    "Leica/M8/L1030132.DNG",
    // Nikon unpack
    "Nikon/D100/DSC_2376.NEF",
    // Nikon Quantized
    "Nikon/D60/DSC_8294.NEF",
    "Olympus/E-P1/P1080385.ORF",
    "Pentax/K100D/IMGP1754.PEF",
    "Sony/ILCE-7RM4/DSC00395.ARW",
];

use criterion::{criterion_group, criterion_main, Criterion};
use libopenraw::{rawfile_from_file, LJpeg};

pub fn ordiag_benchmark(c: &mut Criterion) {
    let dataset = std::env::var("RAWFILES_ROOT").expect("RAWFILES_ROOT not set");
    let dataset = std::path::PathBuf::from(dataset);
    for file in FILES {
        let bench_name = format!("ordiag-{file}");
        let file = dataset.join(file);
        c.bench_function(&bench_name, |b| {
            b.iter(|| {
                let rawfile = rawfile_from_file(&file, None);
                if let Ok(rawfile) = rawfile {
                    let sizes = rawfile.thumbnail_sizes();
                    for size in sizes {
                        let _ = rawfile.thumbnail(*size);
                    }
                    let _ = rawfile.raw_data(false);
                }
            })
        });
    }
}

pub fn dump_benchmark(c: &mut Criterion) {
    let dataset = std::env::var("RAWFILES_ROOT").expect("RAWFILES_ROOT not set");
    let dataset = std::path::PathBuf::from(dataset);
    for file in FILES {
        let bench_name = format!("dump-{file}");
        let file = dataset.join(file);
        c.bench_function(&bench_name, |b| {
            b.iter(|| {
                let rawfile = rawfile_from_file(&file, None).expect("RAW didn't decode");
                rawfile.dump_file(&mut std::io::sink());
            });
        });
    }
}

fn ljpeg_benchmark(c: &mut Criterion) {
    c.bench_function("ljpeg", |b| {
        b.iter(|| {
            let mut decompressor = LJpeg::new();
            let io = std::fs::File::open("test/ljpegtest1.jpg").expect("Couldn't open");
            let mut buffered = std::io::BufReader::new(io);
            let _ = decompressor.discard_decompress(&mut buffered);
        });
    });
}

criterion_group!(benches, ordiag_benchmark, dump_benchmark, ljpeg_benchmark);
criterion_main!(benches);
