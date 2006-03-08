
INCLUDES = -I$(top_srcdir)/include

SUBDIRS = io


lib_LTLIBRARIES = libopenraw.la

libopenraw_la_LDFLAGS = -L$(top_builddir)/dcraw \
	-L$(top_builddir)/lib/io 
libopenraw_la_LIBADD = -lio \
	-ldcparse -ltiff -lexif

libopenraw_la_SOURCES = thumbnails.c \
	cr2.c \
	nef.c \
	or_debug.c \
	or_exif.c \
	or_tiff.c
