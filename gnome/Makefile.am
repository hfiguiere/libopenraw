
SUBDIRS = include

if BUILD_GNOME_SUPPORT
GNOME_TARGETS = libopenrawgnome.la
PKG_CONFIG_FILES = libopenraw-gnome-1.0.pc
endif

INCLUDES = -I$(top_srcdir)/include \
	-I$(srcdir)/include \
	$(LIBGLIB_CFLAGS) \
	$(LIBGDKPIXBUF_CFLAGS)

pkgconfig_DATA = $(PKG_CONFIG_FILES)

lib_LTLIBRARIES = $(GNOME_TARGETS)

libopenrawgnome_la_LDFLAGS = \
	-version-info @LIBOPENRAW_VERSION_INFO@

libopenrawgnome_la_LIBADD = ../lib/libopenraw.la \
	$(LIBGLIB_LIBS)                      \
	$(LIBGDKPIXBUF_LIBS)

libopenrawgnome_la_SOURCES = gdkpixbuf.c
