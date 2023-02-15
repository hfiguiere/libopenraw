#!/bin/sh

#
# Run cargo vendor in the current directory ( $builddir ).
# using the manifest in $1
if [ -z "$1" ] ; then
    echo "Argument needed"
    exit 127
fi
if [ ! -f "$1" ] ; then
    echo "No Cargo.toml found at $1"
    exit 127
fi

cargo vendor --manifest-path "$1"

# remove big binaries for Windows
# Caveat this tarball won't build on Windows.
# See https://github.com/rust-lang/cargo/issues/7058#issuecomment-751856262
rm vendor/winapi*gnu*/lib/*.a

mkdir .cargo
cat > .cargo/config.toml <<EOF
[net]
offline = true

[source.crates-io]
replace-with = "vendored-sources"

[source.vendored-sources]
directory = "vendor"
EOF
