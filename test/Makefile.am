
TESTS = testoption extensions
TESTS_ENVIRONMENT =

OPENRAW_LIB = $(top_builddir)/lib/libopenraw.la

INTERNAL_OPENRAW_LIB = $(top_builddir)/lib/libopenraw_internals.la \
	@abs_top_builddir@/lib/mp4/target/@CARGO_TARGET_SUBDIR@/libmp4.a \
	-ljpeg -ldl -lpthread \
	$(NULL)

AM_CPPFLAGS = \
	@BOOST_CPPFLAGS@ \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/lib

check_PROGRAMS = testoption \
	extensions

EXTRA_DIST = ljpegtest1.jpg

fileio_SOURCES = fileio.t.cpp
fileio_LDADD = $(INTERNAL_OPENRAW_LIB)

testoption_SOURCES = option.t.cpp
testoption_LDFLAGS = @BOOST_UNIT_TEST_FRAMEWORK_LDFLAGS@

testbititerator_SOURCES = bititerator.t.cpp
testbititerator_LDADD = $(INTERNAL_OPENRAW_LIB)
testbititerator_LDFLAGS = @BOOST_UNIT_TEST_FRAMEWORK_LDFLAGS@

extensions_SOURCES = extensions.cpp
extensions_LDADD = $(OPENRAW_LIB)

ciffcontainertest_SOURCES = ciffcontainertest.cpp
ciffcontainertest_LDADD = \
	$(INTERNAL_OPENRAW_LIB) \
	$(NULL)

ljpegtest_SOURCES = ljpegtest.cpp
ljpegtest_LDFLAGS = @BOOST_UNIT_TEST_FRAMEWORK_LDFLAGS@
ljpegtest_LDADD = \
	$(INTERNAL_OPENRAW_LIB) @BOOST_UNIT_TEST_FRAMEWORK_LIBS@ \
	$(NULL)

testunpack_SOURCES = testunpack.cpp
testunpack_LDFLAGS = @BOOST_UNIT_TEST_FRAMEWORK_LDFLAGS@
testunpack_LDADD = $(INTERNAL_OPENRAW_LIB) @BOOST_UNIT_TEST_FRAMEWORK_LIBS@

missingcoefficients_SOURCES = missingcoefficients.cpp
missingcoefficients_LDADD = $(INTERNAL_OPENRAW_LIB)
