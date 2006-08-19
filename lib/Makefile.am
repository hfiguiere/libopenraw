
INCLUDES = -I$(top_srcdir)/include

SUBDIRS = io

noinst_HEADERS = \
	iofile.h \
	rawcontainer.h \
	rawfile.h \
	cr2file.h \
	ifdfilecontainer.h

lib_LTLIBRARIES = libopenraw.la

libopenraw_la_LDFLAGS = -L$(top_builddir)/dcraw \
	-L$(top_builddir)/lib/io 
libopenraw_la_LIBADD = -lio \
	-ldcparse

libopenraw_la_SOURCES = \
	iofile.cpp \
	thumbnails.c \
	or_debug.c \
	ifdfilecontainer.cpp \
	rawcontainer.cpp
