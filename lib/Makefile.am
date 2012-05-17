
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
	rw2file.h \
	raffile.h \
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
	cfapattern.cpp \
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
	or_debug.c \
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
	bimedian_demosaic.cpp bimedian_demosaic.h \
	$(NULL)
