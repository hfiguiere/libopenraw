
INCLUDES = -I$(top_srcdir)/include 

SUBDIRS = io capi

EXTRA_DIST = libopenraw.sym libopenraw-1.0.pc.in

pkgconfig_DATA = libopenraw-1.0.pc

noinst_HEADERS = or_debug.h \
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
	ifd.h \
	ifdfilecontainer.h \
	ifddir.h ifdentry.h \
	orfcontainer.h \
	mrwcontainer.h \
	ciffcontainer.h \
	jfifcontainer.h \
	decompressor.h \
	ljpegdecompressor.h \
	ljpegdecompressor_priv.h \
	crwdecompressor.h \
	exception.h \
	endianutils.h \
	metavalue.h \
	unpack.h

lib_LTLIBRARIES = libopenraw.la

libopenraw_la_LDFLAGS = \
	-version-info @LIBOPENRAW_VERSION_INFO@

#	-export-symbols $(srcdir)/libopenraw.sym 

libopenraw_la_LIBADD = io/libio.la capi/libopenrawc.la -ljpeg

libopenraw_la_SOURCES = \
	trace.cpp \
	bititerator.h \
	bititerator.cpp \
	huffman.h \
	huffman.cpp \
	nefdiffiterator.h \
	nefdiffiterator.cpp \
	nefcfaiterator.h \
	nefcfaiterator.cpp \
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
	bitmapdata.cpp \
	thumbnail.cpp \
	rawdata.cpp \
	or_debug.c \
	ifdfilecontainer.cpp \
	ifddir.cpp ifdentry.cpp \
	rawcontainer.cpp \
	orfcontainer.cpp \
	mrwcontainer.cpp \
	ciffcontainer.cpp \
	jfifcontainer.cpp \
	decompressor.cpp \
	ljpegdecompressor.cpp \
	crwdecompressor.cpp \
	metavalue.cpp \
	unpack.cpp \
	bimedian_demosaic.cpp demosaic.h \
	$(NULL)
