
TESTS = fileio ljpegtest testunpack
TESTS_ENVIRONMENT =

OPENRAW_LIB = $(top_builddir)/lib/libopenraw.la

INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/lib

check_PROGRAMS = fileio ciffcontainertest ljpegtest testunpack

EXTRA_DIST = ljpegtest1.jpg

fileio_LDFLAGS = -L$(top_builddir)/lib/io
fileio_LDADD = -lio
fileio_SOURCES = fileio.t.c

ciffcontainertest_SOURCES = ciffcontainertest.cpp
ciffcontainertest_LDADD = $(OPENRAW_LIB)

ljpegtest_SOURCES = ljpegtest.cpp
ljpegtest_LDFLAGS = -static @BOOST_UNIT_TEST_FRAMEWORK_LDFLAGS@
ljpegtest_LDADD = $(OPENRAW_LIB) @BOOST_UNIT_TEST_FRAMEWORK_LIBS@

testunpack_SOURCES = testunpack.cpp
testunpack_LDFLAGS = -static  @BOOST_UNIT_TEST_FRAMEWORK_LDFLAGS@
testunpack_LDADD = $(OPENRAW_LIB) @BOOST_UNIT_TEST_FRAMEWORK_LIBS@