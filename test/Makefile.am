
TESTS = fileio ljpegtest

OPENRAW_LIB = $(top_builddir)/lib/libopenraw.la

INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/lib

check_PROGRAMS = fileio ciffcontainertest ljpegtest


fileio_LDFLAGS = -L$(top_builddir)/lib/io
fileio_LDADD = -lio
fileio_SOURCES = fileio.t.c

ciffcontainertest_SOURCES = ciffcontainertest.cpp
ciffcontainertest_LDADD = $(OPENRAW_LIB)

ljpegtest_SOURCES = ljpegtest.cpp
ljpegtest_LDADD = $(OPENRAW_LIB) $(BOOST_UNIT_TEST_FRAMEWORK_LIB)