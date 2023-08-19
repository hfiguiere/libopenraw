
TESTS = extensions

OPENRAW_LIB = $(top_builddir)/lib/libopenraw.la

AM_CPPFLAGS = \
	@BOOST_CPPFLAGS@ \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/lib

check_PROGRAMS = extensions

EXTRA_DIST = ljpegtest1.jpg iterator_test.tif

extensions_SOURCES = extensions.cpp
extensions_LDADD = $(OPENRAW_LIB)
