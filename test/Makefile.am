
TESTS = fileio testoption ljpegtest testunpack extensions
TESTS_ENVIRONMENT =

OPENRAW_LIB = $(top_builddir)/lib/libopenraw.la

AM_CPPFLAGS = \
	@BOOST_CPPFLAGS@ \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/lib

check_PROGRAMS = fileio testoption ciffcontainertest ljpegtest testunpack\
	extensions

EXTRA_DIST = ljpegtest1.jpg

fileio_SOURCES = fileio.t.c
fileio_LDFLAGS = -static $(OPENRAW_LIB)

testoption_SOURCES = option.t.cpp
testoption_LDFLAGS = -static @BOOST_UNIT_TEST_FRAMEWORK_LDFLAGS@

extensions_SOURCES = extensions.cpp
extensions_LDFLAGS = -static $(OPENRAW_LIB)

ciffcontainertest_SOURCES = ciffcontainertest.cpp
ciffcontainertest_LDADD = $(OPENRAW_LIB)

ljpegtest_SOURCES = ljpegtest.cpp
ljpegtest_LDFLAGS = -static @BOOST_UNIT_TEST_FRAMEWORK_LDFLAGS@
ljpegtest_LDADD = $(OPENRAW_LIB) @BOOST_UNIT_TEST_FRAMEWORK_LIBS@

testunpack_SOURCES = testunpack.cpp
testunpack_LDFLAGS = -static  @BOOST_UNIT_TEST_FRAMEWORK_LDFLAGS@
testunpack_LDADD = $(OPENRAW_LIB) @BOOST_UNIT_TEST_FRAMEWORK_LIBS@
