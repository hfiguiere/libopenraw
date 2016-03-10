
AM_CPPFLAGS = -I$(top_srcdir)/include @BOOST_CPPFLAGS@

EXTRA_DIST = libopenraw.sym

pkgconfig_DATA = libopenraw-0.1.pc

TESTS = teststream

check_PROGRAMS = teststream

teststream_SOURCES = io/teststream.cpp
teststream_LDADD = libopenraw.la

noinst_HEADERS = \
	trace.h	\
	rawcontainer.h \
	ifdfile.h tiffepfile.h \
	rawfilefactory.h \
	cr2file.h \
	neffile.h \
	orffile.h \
	arwfile.h \
	dngfile.h \
	peffile.h \
	crwfile.h \
	erffile.h \
	mrwfile.h \
	rw2file.h \
	raffile.h \
	rawfile.hpp \
	bitmapdata.hpp \
	thumbnail.hpp \
	rawdata.hpp \
	cfapattern.hpp \
	metavalue.hpp \
	ifd.h \
	ifdfilecontainer.h \
	ifddir.h ifdentry.h \
	orfcontainer.h \
	rw2container.h \
	mrwcontainer.h \
	ciffcontainer.h \
	jfifcontainer.h \
	rafcontainer.h \
	rafmetacontainer.h \
	decompressor.h \
	ljpegdecompressor.h \
	ljpegdecompressor_priv.h \
	crwdecompressor.h \
	olympusdecompressor.h \
	exception.h \
	endianutils.h \
	unpack.h

lib_LTLIBRARIES = libopenraw.la

libopenraw_la_LDFLAGS = \
	-version-info @LIBOPENRAW_VERSION_INFO@

#	-export-symbols $(srcdir)/libopenraw.sym 

libopenraw_la_LIBADD = -ljpeg

libopenraw_la_SOURCES = \
	io/io.c io/posix_io.h \
	io/posix_io.c io/posix_io.h \
	io/stream.cpp io/stream.h \
	io/streamclone.cpp io/streamclone.h \
	io/memstream.cpp io/memstream.h \
	io/file.cpp io/file.h \
	capi/capi.cpp \
	capi/debug.cpp \
	capi/rawdata.cpp \
	capi/rawfile.cpp \
	capi/bitmapdata.cpp \
	trace.cpp \
	bititerator.h \
	bititerator.cpp \
	cfapattern.cpp \
	xtranspattern.hpp \
	xtranspattern.cpp \
	huffman.h \
	huffman.cpp \
	nefdiffiterator.h \
	nefdiffiterator.cpp \
	nefcfaiterator.h \
	nefcfaiterator.cpp \
	rawfile_private.h \
	rawfile.cpp \
	ifdfile.cpp \
	tiffepfile.cpp \
	rawfilefactory.cpp \
	cr2file.cpp \
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
	makernotedir.h makernotedir.cpp \
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
	metavalue.cpp \
	unpack.cpp \
	render/bimedian_demosaic.cpp render/bimedian_demosaic.h \
	render/grayscale.h render/grayscale.cpp
	$(NULL)
