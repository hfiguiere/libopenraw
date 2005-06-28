
TESTS = fileio

INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/lib

check_PROGRAMS = fileio 


CFLAGS = -g
LDFLAGS = -L$(top_builddir)/lib/io
LDADD = -lio

fileio_SOURCES = fileio.t.c
