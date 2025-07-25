// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * libopenraw - the_benchmark.rs
 *
 * Copyright (C) 2023-2025 Hubert Figuière
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

use std::io::Read;

const XTRANS_FILE: &str = "Fujifilm/X-Pro1/DSCF2131.RAF";
const CANON_20D_FILE: &str = "Canon/EOS 20D/IMG_3893.CR2";
const LEICA_MONO_FILE: &str = "Leica/M Monochrom/L1000622.DNG";
const OLYMPUS_EP1_FILE: &str = "Olympus/E-P1/P1080385.ORF";
const FILES: [&str; 13] = [
    "Apple/iPhone XS/IMG_1105.dng",
    "Canon/EOS 10D/CRW_7673.CRW",
    CANON_20D_FILE,
    "Canon/EOS R5/Canon_EOS_R5_CRAW_ISO_100_nocrop_nodual.CR3",
    "Epson/R-D1/_EPS0672.ERF",
    XTRANS_FILE,
    "Leica/M8/L1030132.DNG",
    LEICA_MONO_FILE,
    // Nikon unpack
    "Nikon/D100/DSC_2376.NEF",
    // Nikon Quantized
    "Nikon/D60/DSC_8294.NEF",
    OLYMPUS_EP1_FILE,
    "Pentax/K100D/IMGP1754.PEF",
    "Sony/ILCE-7RM4/DSC00395.ARW",
];

use criterion::{criterion_group, criterion_main, Criterion};
use libopenraw::{rawfile_from_file, LJpeg, RenderingOptions, RenderingStage};

pub fn ordiag_benchmark(c: &mut Criterion) {
    let dataset = std::env::var("RAWFILES_ROOT").expect("RAWFILES_ROOT not set");
    let dataset = std::path::PathBuf::from(dataset);
    for file in FILES {
        let bench_name = format!("ordiag-{file}");
        let file = dataset.join(file);
        c.bench_function(&bench_name, |b| {
            b.iter(|| {
                let rawfile = rawfile_from_file(&file, None).expect("RAW didn't decode");
                if let Some(sizes) = rawfile.thumbnail_sizes() {
                    for size in sizes {
                        let _ = rawfile.thumbnail(*size);
                    }
                }
                let _ = rawfile.raw_data(false);
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
            let mut decompressor = LJpeg::new(false);
            let mut io = std::fs::File::open("test/ljpegtest1.jpg").expect("Couldn't open");
            let mut buffer = Vec::<u8>::new();
            io.read_to_end(&mut buffer).expect("Couldn't read");
            let _ = decompressor.discard_decompress(&buffer);
        });
    });
}

fn colour_correct_benchmark(c: &mut Criterion) {
    let dataset = std::env::var("RAWFILES_ROOT").expect("RAWFILES_ROOT not set");
    let dataset = std::path::PathBuf::from(dataset);
    let files = [CANON_20D_FILE, OLYMPUS_EP1_FILE];
    for file in files {
        let bench_name = format!("colour_correct-{file}");
        let file = dataset.join(file);
        let rawfile = rawfile_from_file(&file, None).expect("RAW didn't decode");
        let options = RenderingOptions::default().with_stage(RenderingStage::Interpolation);
        assert_eq!(
            options.stage,
            RenderingStage::Interpolation,
            "we only want to benchmark interpolation"
        );
        let rawdata = rawfile.raw_data(false).expect("Couldn't get raw data");
        let uncorrected_raw = rawdata.rendered_buffer(&options).expect("Rendering failed");

        c.bench_function(&bench_name, |b| {
            b.iter(|| {
                let raw = uncorrected_raw.clone();
                let _ = rawdata
                    .colour_correct(raw, options.target, true)
                    .expect("Couldn't colour correct");
            });
        });
    }
}

fn bimedian_benchmark(c: &mut Criterion) {
    let dataset = std::env::var("RAWFILES_ROOT").expect("RAWFILES_ROOT not set");
    let dataset = std::path::PathBuf::from(dataset);
    let files = [CANON_20D_FILE, OLYMPUS_EP1_FILE];
    for file in files {
        let bench_name = format!("bimedian-{file}");
        let file = dataset.join(file);
        let rawfile = rawfile_from_file(&file, None).expect("RAW didn't decode");
        let options = RenderingOptions::default().with_stage(RenderingStage::Interpolation);
        assert_eq!(
            options.stage,
            RenderingStage::Interpolation,
            "we only want to benchmark interpolation"
        );
        let rawdata = rawfile.raw_data(false).expect("Couldn't get raw data");
        c.bench_function(&bench_name, |b| {
            b.iter(|| {
                let _ = rawdata.rendered_image(&options).expect("Rendering failed");
            });
        });
    }
}

/// Benchmark the grayscale Linear rendering.
fn grayscale_benchmark(c: &mut Criterion) {
    let dataset = std::env::var("RAWFILES_ROOT").expect("RAWFILES_ROOT not set");
    let dataset = std::path::PathBuf::from(dataset);
    let file = LEICA_MONO_FILE;
    let bench_name = format!("grayscale-{file}");
    let file = dataset.join(file);
    let rawfile = rawfile_from_file(&file, None).expect("RAW didn't decode");
    let options = RenderingOptions::default().with_stage(RenderingStage::Interpolation);
    assert_eq!(
        options.stage,
        RenderingStage::Interpolation,
        "we only want to benchmark interpolation"
    );
    let rawdata = rawfile.raw_data(false).expect("Couldn't get raw data");
    c.bench_function(&bench_name, |b| {
        b.iter(|| {
            let _ = rawdata.rendered_image(&options).expect("Rendering failed");
        });
    });
}

/// Benchmmark X-Trans rendering.
/// This assume the input file is apropriate as we don't choose the
/// algorithm yet.
fn xtrans_benchmark(c: &mut Criterion) {
    let dataset = std::env::var("RAWFILES_ROOT").expect("RAWFILES_ROOT not set");
    let dataset = std::path::PathBuf::from(dataset);
    let file = XTRANS_FILE;
    let bench_name = format!("xtrans-{file}");
    let file = dataset.join(file);
    let rawfile = rawfile_from_file(&file, None).expect("RAW didn't decode");
    let options = RenderingOptions::default().with_stage(RenderingStage::Interpolation);
    assert_eq!(
        options.stage,
        RenderingStage::Interpolation,
        "we only want to benchmark interpolation"
    );
    let rawdata = rawfile.raw_data(false).expect("Couldn't get raw data");
    c.bench_function(&bench_name, |b| {
        b.iter(|| {
            let _ = rawdata.rendered_image(&options).expect("Rendering failed");
        });
    });
}

criterion_group!(benches, ordiag_benchmark, dump_benchmark, ljpeg_benchmark);
criterion_group!(
    name = render_benches;
    config = Criterion::default().significance_level(0.1).sample_size(10);
    targets = xtrans_benchmark, bimedian_benchmark, grayscale_benchmark, colour_correct_benchmark
);
criterion_main!(benches, render_benches);
