

noinst_PROGRAMS = thumb thumbc

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/lib

thumbc_LDADD = ../lib/libopenraw.la
thumbc_LDFLAGS = -L$(top_builddir)/lib

thumbc_SOURCES = thumbc.c

thumb_LDADD = ../lib/.libs/libopenraw.a ../lib/io/.libs/libio.a
thumb_LDFLAGS = -L$(top_builddir)/lib

thumb_SOURCES = thumb.cpp
