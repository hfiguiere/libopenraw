
TESTS = fileio ljpegtest
TESTS_ENVIRONMENT =

OPENRAW_LIB = $(top_builddir)/lib/libopenraw.la

INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/lib

check_PROGRAMS = fileio ciffcontainertest ljpegtest

EXTRA_DIST = ljpegtest1.jpg

fileio_LDFLAGS = -L$(top_builddir)/lib/io
fileio_LDADD = -lio
fileio_SOURCES = fileio.t.c

ciffcontainertest_SOURCES = ciffcontainertest.cpp
ciffcontainertest_LDADD = $(OPENRAW_LIB)

ljpegtest_SOURCES = ljpegtest.cpp
ljpegtest_LDFLAGS = -static
ljpegtest_LDADD = $(OPENRAW_LIB) $(BOOST_UNIT_TEST_FRAMEWORK_LIB)
