Fuzzin libopenraw Rust
======================

You need to install Rust afl. See https://rust-fuzz.github.io/book/afl.html

`cargo install afl`

Build the fuzzing targets (this include the test binaries):

`cargo afl build --features=fuzzing`

Then you need to run the fuzzer:

`cargo afl fuzz -i in -o out -t 1000 target/debug/fuzz-ljpeg`

This will fuzz the Ljpeg decompressor.

Run `target/debug/test-ljpeg` with a filename from the output
to find crashes. Fix the crash, run the test again.

## Targets

Targets have a fuzz and a test element. The fuzz is the fuzzer harness, while
the test element allow just testing the data.

### LJPEG

`fuzz-ljpeg` for fuzzing and `test-ljpeg` for debugging:

Takes as an input the LJPEG stream.
