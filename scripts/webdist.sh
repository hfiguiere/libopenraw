#!/bin/sh
#
# This script is a helper to copy the documentation to the website.
#
# Argument 1 is the top builddir.
# It is run by the webdist target of the build system.
# It requires setting $WEB_ROOT to the top level dir of the hugo website.
#

set -e

if [ -z "$1" -o -z "$2" ]; then
    echo "Usage: $0 TOP_BUILDDIR VERSION"
    exit 1
fi

TOP_BUILDDIR=$1
VERSION=$2

if [ -z "$WEB_ROOT" ]; then
    echo "WEB_ROOT isn't set"
    exit 2
fi
if [ ! -d "$WEB_ROOT/doxygen/api/libopenraw" ]; then
    echo "Destination directory not found"
    exit 3
fi
if [ ! -f "$WEB_ROOT/config.toml" ]; then
    echo "Not a hugo directory"
    exit 4
fi
if [ ! -d "$TOP_BUILDDIR" ]; then
    echo "TOP_BUILDDIR isn't a directory"
    exit 5
fi
if [ ! -x `which rsync` ]; then
    echo "rsync not found"
    exit 6
fi

# copy the doxygen output to the latest stable directory, and the versioned.
if [ ! -d $WEB_ROOT/doxygen/api/libopenraw/$VERSION ]; then
    mkdir $WEB_ROOT/doxygen/api/libopenraw/$VERSION
fi
rsync -av --delete $TOP_BUILDDIR/doc/doxygen/html/ $WEB_ROOT/doxygen/api/libopenraw/$VERSION/
rsync -av --delete $TOP_BUILDDIR/doc/doxygen/html/ $WEB_ROOT/doxygen/api/libopenraw/stable/
