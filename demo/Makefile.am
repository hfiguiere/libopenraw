

if BUILD_GNOME_SUPPORT
GNOME_BINARIES = gdk pixbufload
endif

noinst_PROGRAMS = extensions thumb thumbc cfa ccfa ppmload $(GNOME_BINARIES)

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/lib \
	-I$(top_srcdir)/gnome/include \
	@LIBGDKPIXBUF_CFLAGS@ @BOOST_CPPFLAGS@

LIBOPENRAW_LIBS = ../lib/libopenraw.la -lstdc++ -ljpeg
LIBOPENRAWGNOME_LIBS = ../gnome/libopenrawgnome.la -lstdc++ -ljpeg \
	@LIBGDKPIXBUF_LIBS@

extensions_LDADD = $(LIBOPENRAW_LIBS)
extensions_LDFLAGS = -L$(top_builddir)/lib -static
extensions_SOURCES = extensions.cpp

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

gdk_SOURCES = gdk.c
gdk_LDADD = $(LIBOPENRAWGNOME_LIBS)
gdk_LDFLAGS = -L$(top_builddir)/gnome -static

pixbufload_SOURCES = pixbufload.c
pixbufload_LDADD = $(LIBOPENRAWGNOME_LIBS)
pixbufload_LDFLAGS = -L$(top_builddir)/gnome -static

ppmload_SOURCES = ppmload.c
ppmload_LDADD = $(LIBOPENRAW_LIBS)
ppmload_LDFLAGS = -L$(top_builddir)/lib -static
