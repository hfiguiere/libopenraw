ACLOCAL_AMFLAGS = -I m4

SUBDIRS = . lib gnome doc test testsuite demo tools include

dox:
	make -C doc dox

webdist: dox
	$(top_srcdir)/scripts/webdist.sh ${top_builddir} ${VERSION}

EXTRA_DIST = RELEASE_NOTES build-aux/cargo-vendor.sh

CLEANFILES = $(CARGO_TARGET_DIR)/CACHEDIR.TAG $(BUILT_SOURCES)

RUST_FILES = Cargo.lock \
	Cargo.toml \
	src/apple.rs \
	src/bitmap.rs \
	src/camera_ids.rs \
	src/canon.rs \
	src/canon/cr3.rs \
	src/canon/cr2.rs \
	src/canon/crw.rs \
	src/canon/crw/decompress.rs \
	src/canon/crw/ciff/container.rs \
	src/canon/crw/ciff.rs \
	src/canon/matrices.rs \
	src/capi.rs \
	src/capi/bitmap.rs \
	src/capi/ifd.rs \
	src/capi/iterator.rs \
	src/capi/metavalue.rs \
	src/capi/mosaic.rs \
	src/capi/rawdata.rs \
	src/capi/rawfile.rs \
	src/capi/thumbnail.rs \
	src/colour.rs \
	src/colour/matrix.rs \
	src/container.rs \
	src/decompress.rs \
	src/decompress/ljpeg.rs \
	src/decompress/sliced_buffer.rs \
	src/decompress/tiled.rs \
	src/dng.rs \
	src/dump.rs \
	src/epson.rs \
	src/factory.rs \
	src/fujifilm.rs \
	src/fujifilm/matrices.rs \
	src/fujifilm/raf.rs \
	src/io.rs \
	src/identify.rs \
	src/jpeg.rs \
	src/jpeg/container.rs \
	src/leica.rs \
	src/lib.rs \
	src/metavalue.rs \
	src/mp4.rs \
	src/mp4/container.rs \
	src/nikon.rs \
	src/nikon/huffman.rs \
	src/nikon/matrices.rs \
	src/nikon/diffiterator.rs \
	src/olympus.rs \
	src/olympus/decompress.rs \
	src/olympus/matrices.rs \
	src/panasonic.rs \
	src/pentax.rs \
	src/rawdata.rs \
	src/rawfile.rs \
	src/ricoh.rs \
	src/sigma.rs \
	src/sony.rs \
	src/thumbnail.rs \
	src/tiff.rs \
	src/tiff/dir.rs \
	src/tiff/exif/tags.rs \
	src/tiff/entry.rs \
	src/tiff/container.rs \
	src/tiff/exif.rs \
	src/utils.rs \
	$(NULL)

BUILT_SOURCES = _or_dummy.c
_or_dummy.c:
	echo "static int __attribute__((unused)) __or_dummy;" > _or_dummy.c

noinst_LTLIBRARIES = liblibopenraw.la

if DEBUG
CARGO_RELEASE_ARGS=
else
CARGO_RELEASE_ARGS=--release
endif

CARGO_TARGET_DIR=@abs_top_builddir@/target
RUST_PATH=$(CARGO_TARGET_DIR)/@CARGO_TARGET_SUBDIR@
RUST_LIB=$(RUST_PATH)/liblibopenraw.a

liblibopenraw_la_SOURCES = _or_dummy.c
liblibopenraw.la: $(liblibopenraw_la_OBJECTS) $(RUST_FILES)
	CARGO_TARGET_DIR=$(CARGO_TARGET_DIR) \
	cargo build --features=capi $(CARGO_VERBOSE) $(CARGO_RELEASE_ARGS) --lib && \
	$(LINK) $< && mv $(RUST_LIB) .libs/liblibopenraw.a

clean-local:
	CARGO_TARGET_DIR=$(CARGO_TARGET_DIR) cargo clean $(CARGO_VERBOSE) $(CARGO_RELEASE_ARGS)
