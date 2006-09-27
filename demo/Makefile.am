

noinst_PROGRAMS = thumb thumbc

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/lib

thumbc_LDADD = ../lib/libopenraw.la -lstdc++
thumbc_LDFLAGS = -L$(top_builddir)/lib -static

thumbc_SOURCES = thumbc.c

thumb_LDADD = ../lib/libopenraw.la -lstdc++
thumb_LDFLAGS = -L$(top_builddir)/lib -static

thumb_SOURCES = thumb.cpp
