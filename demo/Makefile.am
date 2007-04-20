

noinst_PROGRAMS = thumb thumbc cfa

INCLUDES = -I$(top_srcdir)/include
LIBOPENRAW_LIBS = ../lib/libopenraw.la -lstdc++ -ljpeg

thumbc_LDADD = $(LIBOPENRAW_LIBS)
thumbc_LDFLAGS = -L$(top_builddir)/lib -static

thumbc_SOURCES = thumbc.c

thumb_LDADD = $(LIBOPENRAW_LIBS)
thumb_LDFLAGS = -L$(top_builddir)/lib -static

thumb_SOURCES = thumb.cpp

cfa_SOURCES = cfa.cpp
cfa_LDADD = $(LIBOPENRAW_LIBS)
cfa_LDFLAGS = -L$(top_builddir)/lib -static
