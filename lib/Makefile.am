
AM_CPPFLAGS = -I$(top_srcdir)/include @BOOST_CPPFLAGS@

EXTRA_DIST = libopenraw.sym io/testfile.tmp

pkgconfig_DATA = libopenraw-0.1.pc

TESTS = teststream

check_PROGRAMS = teststream

teststream_SOURCES = io/teststream.cpp
teststream_LDADD = libopenraw.la

noinst_HEADERS = \
	trace.hpp	\
	option.hpp \
	rawcontainer.hpp \
	ifdfile.hpp \
	tiffepfile.hpp \
	rawfilefactory.hpp \
	cr2file.hpp \
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
	cfapattern.hpp \
	metavalue.hpp \
	ifd.hpp \
	ifdfilecontainer.hpp \
	ifddir.hpp ifdentry.hpp \
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
	exception.hpp \
	endianutils.hpp \
	unpack.hpp \
	$(NULL)

lib_LTLIBRARIES = libopenraw.la

libopenraw_la_LDFLAGS = \
	-version-info @LIBOPENRAW_VERSION_INFO@

#	-export-symbols $(srcdir)/libopenraw.sym 

libopenraw_la_LIBADD = -ljpeg

libopenraw_la_SOURCES = \
	io/io.c io/posix_io.h \
	io/posix_io.c io/posix_io.h \
	io/stream.cpp io/stream.hpp \
	io/streamclone.cpp io/streamclone.hpp \
	io/memstream.cpp io/memstream.hpp \
	io/file.cpp io/file.hpp \
	io/io_private.h \
	capi/capi.cpp \
	capi/debug.cpp \
	capi/rawdata.cpp \
	capi/rawfile.cpp \
	capi/bitmapdata.cpp \
	capi/cfapattern.cpp \
	capi/metadata.cpp \
	trace.cpp \
	bititerator.hpp \
	bititerator.cpp \
	cfapattern.cpp \
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
	metavalue.cpp \
	unpack.cpp \
	render/bimedian_demosaic.cpp render/bimedian_demosaic.hpp \
	render/grayscale.hpp render/grayscale.cpp
	$(NULL)
