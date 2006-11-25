
TESTS = fileio

INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/lib

check_PROGRAMS = fileio ciffcontainertest


fileio_LDFLAGS = -L$(top_builddir)/lib/io
fileio_LDADD = -lio
fileio_SOURCES = fileio.t.c

ciffcontainertest_LDFLAGS = -L$(top_builddir)/lib
ciffcontainertest_LDADD = -lopenraw
ciffcontainertest_SOURCES = ciffcontainertest.cpp
