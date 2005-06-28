
INCLUDES = -I$(top_srcdir)/include

SUBDIRS = io


lib_LTLIBRARIES = libopenraw.la

libopenraw_la_LIBADD = io/libio.a \
	$(top_builddir)/dcraw/libdcparse.a

libopenraw_la_SOURCES = thumbnails.c