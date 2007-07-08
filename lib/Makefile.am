
INCLUDES = -I$(top_srcdir)/include

SUBDIRS = io capi

EXTRA_DIST = libopenraw.sym libopenraw-1.0.pc.in

pkgconfig_DATA = libopenraw-1.0.pc

noinst_HEADERS = or_debug.h \
	debug.h	\
	rawcontainer.h \
	ifdfile.h \
	rawfilefactory.h \
	cr2file.h \
	neffile.h \
	orffile.h \
	arwfile.h \
	dngfile.h \
	peffile.h \
	crwfile.h \
	ifd.h \
	ifdfilecontainer.h \
	ifddir.h ifdentry.h \
	orfcontainer.h \
	ciffcontainer.h \
	jfifcontainer.h \
	decompressor.h \
	ljpegdecompressor.h \
	crwdecompressor.h \
	exception.h \
	endianutils.h

lib_LTLIBRARIES = libopenraw.la

libopenraw_la_LDFLAGS = \
	-version-info @LIBOPENRAW_VERSION_INFO@

#	-export-symbols $(srcdir)/libopenraw.sym 

libopenraw_la_LIBADD = io/libio.la capi/libopenrawc.la -ljpeg

libopenraw_la_SOURCES = \
	debug.cpp \
	rawfile.cpp \
	ifdfile.cpp \
	rawfilefactory.cpp \
	cr2file.cpp \
	neffile.cpp \
	orffile.cpp \
	arwfile.cpp \
	dngfile.cpp \
	peffile.cpp \
	crwfile.cpp \
	bitmapdata.cpp \
	thumbnail.cpp \
	rawdata.cpp \
	or_debug.c \
	ifdfilecontainer.cpp \
	ifddir.cpp ifdentry.cpp \
	rawcontainer.cpp \
	orfcontainer.cpp \
	ciffcontainer.cpp \
	jfifcontainer.cpp \
	decompressor.cpp \
	ljpegdecompressor.cpp \
	crwdecompressor.cpp
