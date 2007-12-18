

if BUILD_GNOME_SUPPORT
GNOME_BINARIES = gdk
endif

noinst_PROGRAMS = thumb thumbc cfa ccfa $(GNOME_BINARIES)

INCLUDES = -I$(top_srcdir)/include  -I$(top_srcdir)/gnome/include
LIBOPENRAW_LIBS = ../lib/libopenraw.la -lstdc++ -ljpeg
LIBOPENRAWGNOME_LIBS = ../gnome/libopenrawgnome.la -lstdc++ -ljpeg \
	@LIBGDKPIXBUF_LIBS@

thumbc_LDADD = $(LIBOPENRAW_LIBS)
thumbc_LDFLAGS = -L$(top_builddir)/lib -static
thumbc_SOURCES = thumbc.c

thumb_LDADD = $(LIBOPENRAW_LIBS) 
thumb_LDFLAGS = -L$(top_builddir)/lib -static
thumb_SOURCES = thumb.cpp

cfa_SOURCES = cfa.cpp
cfa_LDADD = $(LIBOPENRAW_LIBS)
cfa_LDFLAGS = -L$(top_builddir)/lib -static

ccfa_SOURCES = ccfa.c
ccfa_LDADD = $(LIBOPENRAW_LIBS)
ccfa_LDFLAGS = -L$(top_builddir)/lib -static

gdk_SOURCE = gdk.c
gdk_CPPFLAGS = @LIBGDKPIXBUF_CFLAGS@
gdk_LDADD = $(LIBOPENRAWGNOME_LIBS)
gdk_LDFLAGS = -L$(top_builddir)/gnome -static
