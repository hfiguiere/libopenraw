

noinst_PROGRAMS = thumb thumbc

INCLUDES = -I$(top_srcdir)/include

thumbc_LDADD = ../lib/libopenraw.la -lstdc++ -ljpeg
thumbc_LDFLAGS = -L$(top_builddir)/lib -static

thumbc_SOURCES = thumbc.c

thumb_LDADD = ../lib/libopenraw.la -lstdc++ -ljpeg
thumb_LDFLAGS = -L$(top_builddir)/lib -static

thumb_SOURCES = thumb.cpp
