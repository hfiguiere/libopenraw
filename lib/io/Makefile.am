
INCLUDES = -I$(top_srcdir)/include -I$(srcdir)/..

TESTS = teststream

check_PROGRAMS = teststream

noinst_LTLIBRARIES = libio.la
noinst_HEADERS = posix_io.h io_private.h \
	stream.h streamclone.h memstream.h file.h

libio_la_SOURCES = io.c \
	posix_io.c \
	stream.cpp \
	streamclone.cpp \
	memstream.cpp file.cpp

teststream_SOURCES = teststream.cpp
teststream_LDADD = ../libopenraw.la
 