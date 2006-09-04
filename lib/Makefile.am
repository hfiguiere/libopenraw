
INCLUDES = -I$(top_srcdir)/include

SUBDIRS = io

noinst_HEADERS = \
	iofile.h \
	rawcontainer.h \
	rawfile.h \
	cr2file.h \
	neffile.h \
	orffile.h \
	arwfile.h \
	ifd.h \
	ifdfilecontainer.h \
	ifddir.h ifdentry.h \
	orfcontainer.h \
	ciffcontainer.h

lib_LTLIBRARIES = libopenraw.la

libopenraw_la_LDFLAGS = -L$(top_builddir)/dcraw \
	-L$(top_builddir)/lib/io 
libopenraw_la_LIBADD = -lio \
	-ldcparse

libopenraw_la_SOURCES = \
	iofile.cpp \
	rawfile.cpp \
	cr2file.cpp \
	neffile.cpp \
	orffile.cpp \
	arwfile.cpp \
	thumbnail.cpp \
	or_debug.c \
	ifdfilecontainer.cpp \
	ifddir.cpp ifdentry.cpp \
	rawcontainer.cpp \
	orfcontainer.cpp \
	ciffcontainer.cpp 
