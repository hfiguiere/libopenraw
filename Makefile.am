ACLOCAL_AMFLAGS = -I m4

SUBDIRS = . lib doc test testsuite demo tools include

dox:
	make -C doc dox

webdist: dox
	$(top_srcdir)/scripts/webdist.sh ${top_builddir} ${VERSION}

EXTRA_DIST = MPL-2.0 \
	RELEASE_NOTES \
	build-aux/cargo-vendor.sh \
	$(RUST_FILES) \
	$(TESTDATA_FILES)

CLEANFILES = $(CARGO_TARGET_DIR)/CACHEDIR.TAG $(BUILT_SOURCES)

TESTDATA_FILES = \
	testdata/identify/content_cr2 \
	testdata/identify/content_cr3 \
	testdata/identify/content_crw \
	testdata/identify/content_mrw \
	testdata/identify/content_raf

RUST_FILES = Cargo.lock \
	Cargo.toml \
	benches/the_benchmark.rs \
	build.rs \
	src/apple.rs \
	src/bin/fuzz-ljpeg.rs \
	src/bin/fuzz-olymdecompress.rs \
	src/bin/fuzz-rawfile.rs \
	src/bin/identify.rs \
	src/bin/metadata.rs \
	src/bin/ordiag.rs \
	src/bin/ordump.rs \
	src/bin/rawdata.rs \
	src/bin/test-ljpeg.rs \
	src/bin/test-rawfile.rs \
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
	src/metadata.rs \
	src/minolta.rs \
	src/mosaic.rs \
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
	src/rawfile.rs \
	src/rawimage.rs \
	src/render.rs \
	src/render/demosaic.rs \
	src/render/grayscale.rs \
	src/ricoh.rs \
	src/sigma.rs \
	src/sony.rs \
	src/thumbnail.rs \
	src/tiff.rs \
	src/tiff/dir.rs \
	src/tiff/entry.rs \
	src/tiff/container.rs \
	src/tiff/exif.rs \
	src/tiff/exif/generated.rs \
	src/tiff/exif/tags.rs \
	src/tiff/iterator.rs \
	src/utils.rs \
	libopenraw-testing/Cargo.toml \
	libopenraw-testing/src/bin/gentest.rs \
	libopenraw-testing/src/lib.rs \
	$(NULL)

BUILT_SOURCES = _or_dummy.c
_or_dummy.c:
	echo "static int __attribute__((unused)) __or_dummy;" > _or_dummy.c

noinst_LTLIBRARIES = libopenraw.la

if DEBUG
CARGO_RELEASE_ARGS=
else
CARGO_RELEASE_ARGS=--release
endif

CARGO_TARGET_DIR=@abs_top_builddir@/target
RUST_PATH=$(CARGO_TARGET_DIR)/@CARGO_TARGET_SUBDIR@
RUST_LIB=$(RUST_PATH)/liblibopenraw.a

Cargo.lock:
	CARGO_TARGET_DIR=$(CARGO_TARGET_DIR) \
	cargo update

libopenraw_la_SOURCES = _or_dummy.c
libopenraw.la: $(liblibopenraw_la_OBJECTS) $(RUST_FILES)
	CARGO_TARGET_DIR=$(CARGO_TARGET_DIR) \
	cargo build --features=capi $(CARGO_VERBOSE) $(CARGO_RELEASE_ARGS) --lib && \
	$(LINK) $< && mv $(RUST_LIB) .libs/libopenraw.a

check-local: $(RUST_FILES)
	CARGO_TARGET_DIR=$(CARGO_TARGET_DIR) \
	cargo test --features=capi $(CARGO_VERBOSE) $(CARGO_RELEASE_ARGS) --all-targets

clean-local:
	CARGO_TARGET_DIR=$(CARGO_TARGET_DIR) cargo clean $(CARGO_VERBOSE) $(CARGO_RELEASE_ARGS)

DISTCLEANFILES = .cargo/config.toml $(CARGO_TARGET_DIR)/.rustc_info.json

dist-hook:
	($(shell pwd)/$(top_srcdir)/build-aux/cargo-vendor.sh $(shell pwd)/$(top_srcdir)/Cargo.toml)
	cp -av vendor $(distdir)/

distclean-local:
	rm -fr vendor/*
