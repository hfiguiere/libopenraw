
INCLUDES = -I$(top_srcdir)/include

SUBDIRS = io capi

EXTRA_DIST = libopenraw.sym libopenraw-1.0.pc.in

pkgconfig_DATA = libopenraw-1.0.pc

noinst_HEADERS = or_debug.h \
	iofile.h \
	debug.h	\
	rawcontainer.h \
	rawfile.h \
	cr2file.h \
	neffile.h \
	orffile.h \
	arwfile.h \
	dngfile.h \
	peffile.h \
	ifd.h \
	ifdfilecontainer.h \
	ifddir.h ifdentry.h \
	thumbnail.h \
	orfcontainer.h

lib_LTLIBRARIES = libopenraw.la

libopenraw_la_LDFLAGS = \
	-export-symbols $(srcdir)/libopenraw.sym \
	-version-info @LIBOPENRAW_VERSION_INFO@

libopenraw_la_LIBADD = io/libio.la capi/libopenrawc.la

libopenraw_la_SOURCES = \
	iofile.cpp \
	debug.cpp \
	rawfile.cpp \
	cr2file.cpp \
	neffile.cpp \
	orffile.cpp \
	arwfile.cpp \
	dngfile.cpp \
	peffile.cpp \
	thumbnail.cpp \
	or_debug.c \
	ifdfilecontainer.cpp \
	ifddir.cpp ifdentry.cpp \
	rawcontainer.cpp \
	orfcontainer.cpp
