
AM_CPPFLAGS = -I$(top_srcdir)/include @BOOST_CPPFLAGS@

RUST_FILES = Cargo.lock \
	Cargo.toml \
	lib.rs \
	$(NULL)

DISTCLEANFILES = mp4/.cargo/config.toml

dist-hook:
	mkdir -p $(distdir)/mp4/
	(cd mp4 && $(shell pwd)/$(top_srcdir)/build-aux/cargo-vendor.sh $(shell pwd)/$(top_srcdir)/lib/mp4/Cargo.toml)
	cp -av mp4/vendor $(distdir)/mp4/

distclean-local:
	rm -fr mp4/vendor/*

EXTRA_DIST = libopenraw.sym io/testfile.tmp \
	exif/exif-tags.pl \
	$(RUST_FILES)

update_tags:
	$(srcdir)/exif/exif-tags.pl > $(srcdir)/exif/exif_tags.cpp

pkgconfig_DATA = libopenraw-0.3.pc

TESTS = teststream

check_PROGRAMS = teststream

teststream_SOURCES = io/teststream.cpp
teststream_LDADD = libopenraw_internals.la

noinst_HEADERS = \
	mp4/mp4parse.h \
	mp4/mp4parse_ffi.h \
	trace.hpp	\
	option.hpp \
	rawcontainer.hpp \
	ifdfile.hpp \
	tiffepfile.hpp \
	rawfilefactory.hpp \
	cr2file.hpp \
	cr3file.hpp \
	neffile.hpp \
	orffile.hpp \
	arwfile.hpp \
	dngfile.hpp \
	peffile.hpp \
	crwfile.hpp \
	erffile.hpp \
	mrwfile.hpp \
	rw2file.hpp \
	raffile.hpp \
	rawfile.hpp \
	bitmapdata.hpp \
	thumbnail.hpp \
	rawdata.hpp \
	mosaicinfo.hpp \
	metavalue.hpp \
	metadata.hpp \
	ifd.hpp \
	ifdfilecontainer.hpp \
	ifddir.hpp ifdentry.hpp \
	isomediacontainer.hpp \
	orfcontainer.hpp \
	rw2container.hpp \
	mrwcontainer.hpp \
	ciffcontainer.hpp \
	jfifcontainer.hpp \
	rafcontainer.hpp \
	rafmetacontainer.hpp \
	decompressor.hpp \
	ljpegdecompressor.hpp \
	ljpegdecompressor_priv.hpp \
	crwdecompressor.hpp \
	olympusdecompressor.hpp \
	canon.hpp \
	exception.hpp \
	endianutils.hpp \
	unpack.hpp \
	$(NULL)

lib_LTLIBRARIES = libopenraw.la
noinst_LTLIBRARIES = libopenraw_internals.la

libopenraw_la_LDFLAGS = \
	$(BSYMBOLIC_LDFLAG) \
	-version-info @LIBOPENRAW_VERSION_INFO@ \
	-export-dynamic \
	-no-undefined \
	-export-symbols-regex "^or_.*" \
	$(AM_LDFLAGS)
libopenraw_la_LIBADD = ../liblibopenraw.la
libopenraw_la_SOURCES =
#	capi/capi.h \
#	capi/capi.cpp \
#	capi/debug.cpp \
#	capi/rawdata.cpp \
#	capi/rawfile.cpp \
#	capi/bitmapdata.cpp \
#	capi/mosaicinfo.cpp \
#	capi/metadata.cpp \
#	capi/ifd.cpp \
#	$(NULL)

libopenraw_internals_la_DEPENDENCIES =
libopenraw_internals_la_SOURCES = \
	io/io.c io/posix_io.h \
	io/posix_io.c io/posix_io.h \
	io/stream.cpp io/stream.hpp \
	io/streamclone.cpp io/streamclone.hpp \
	io/memstream.cpp io/memstream.hpp \
	io/file.cpp io/file.hpp \
	io/io_private.h \
	trace.cpp \
	bititerator.hpp \
	bititerator.cpp \
	mosaicinfo.cpp \
	xtranspattern.hpp \
	xtranspattern.cpp \
	huffman.hpp \
	huffman.cpp \
	nefdiffiterator.hpp \
	nefdiffiterator.cpp \
	nefcfaiterator.hpp \
	nefcfaiterator.cpp \
	rawfile_private.hpp \
	rawfile.cpp \
	ifdfile.cpp \
	tiffepfile.cpp \
	rawfilefactory.cpp \
	cr2file.cpp \
	cr3file.cpp \
	neffile.cpp \
	orffile.cpp \
	arwfile.cpp \
	dngfile.cpp \
	peffile.cpp \
	crwfile.cpp \
	erffile.cpp \
	mrwfile.cpp \
	rw2file.cpp \
	raffile.cpp \
	bitmapdata.cpp \
	thumbnail.cpp \
	rawdata.cpp \
	ifdfilecontainer.cpp \
	ifddir.cpp ifdentry.cpp \
	isomediacontainer.cpp \
	makernotedir.hpp makernotedir.cpp \
	rawcontainer.cpp \
	orfcontainer.cpp \
	rw2container.cpp \
	rafcontainer.cpp \
	rafmetacontainer.cpp \
	mrwcontainer.cpp \
	ciffcontainer.cpp \
	jfifcontainer.cpp \
	decompressor.cpp \
	ljpegdecompressor.cpp \
	crwdecompressor.cpp \
	olympusdecompressor.cpp \
	canon.cpp \
	metavalue.cpp \
	metadata.cpp \
	unpack.cpp \
	render/bimedian_demosaic.cpp render/bimedian_demosaic.hpp \
	render/grayscale.hpp render/grayscale.cpp \
	ciff/heap.hpp ciff/heap.cpp \
	ciff/recordentry.hpp ciff/recordentry.cpp \
	ciff/ciffifd.hpp ciff/ciffifd.cpp \
	exif/exif_tags.hpp exif/exif_tags.cpp \
	exif/exif_tags-manual.cpp \
	$(NULL)
