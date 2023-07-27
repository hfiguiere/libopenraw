

if BUILD_GNOME_SUPPORT
GNOME_BINARIES = gdk pixbufload
endif

noinst_PROGRAMS = extensions thumbc ccfa ppmload $(GNOME_BINARIES)

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/lib \
	-I$(top_srcdir)/gnome/include \
	@LIBGDKPIXBUF_CFLAGS@ @BOOST_CPPFLAGS@

LIBOPENRAW_LIBS = $(top_builddir)/lib/libopenraw.la -lm
LIBOPENRAWGNOME_LIBS = $(top_builddir)/gnome/libopenrawgnome.la \
	@LIBGDKPIXBUF_LIBS@

extensions_SOURCES = extensions.cpp
extensions_LDADD = $(LIBOPENRAW_LIBS)

thumbc_SOURCES = thumbc.c
thumbc_LDADD = $(LIBOPENRAW_LIBS)

ccfa_SOURCES = ccfa.c
ccfa_LDADD = $(LIBOPENRAW_LIBS)

gdk_SOURCES = gdk.c
gdk_LDADD = $(LIBOPENRAW_LIBS) $(LIBOPENRAWGNOME_LIBS)

pixbufload_SOURCES = pixbufload.c
pixbufload_LDADD = $(LIBOPENRAW_LIBS) $(LIBOPENRAWGNOME_LIBS)

ppmload_SOURCES = ppmload.c
ppmload_LDADD = $(LIBOPENRAW_LIBS)
