
SUBDIRS = include

if BUILD_GNOME_SUPPORT
GNOME_TARGETS = libopenrawgnome.la
GDK_PIXBUF_PLUGIN = libopenraw_pixbuf.la
PKG_CONFIG_FILES = libopenraw-gnome-0.1.pc
endif

AM_CPPFLAGS = -I$(top_srcdir)/include \
	-I$(srcdir)/include \
	@BOOST_CPPFLAGS@ \
	$(LIBGLIB_CFLAGS) \
	$(LIBGDKPIXBUF_CFLAGS)

pkgconfig_DATA = $(PKG_CONFIG_FILES)

lib_LTLIBRARIES = $(GNOME_TARGETS)

loaderdir = $(GDK_PIXBUF_DIR)
loader_LTLIBRARIES = $(GDK_PIXBUF_PLUGIN)

libopenrawgnome_la_LDFLAGS = \
	-version-info @LIBOPENRAW_VERSION_INFO@

libopenrawgnome_la_LIBADD = ../lib/libopenraw.la \
	$(LIBGLIB_LIBS)                      \
	$(LIBGDKPIXBUF_LIBS)

libopenrawgnome_la_SOURCES = gdkpixbuf.c


libopenraw_pixbuf_la_LDFLAGS = -no-undefined -module -avoid-version
libopenraw_pixbuf_la_LIBADD = ../lib/libopenraw.la \
	$(LIBGLIB_LIBS)                      \
	$(LIBGDKPIXBUF_LIBS)
libopenraw_pixbuf_la_SOURCES = pixbuf-loader.c
