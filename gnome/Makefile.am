




INCLUDES = -I$(top_srcdir)/include \
	$(LIBGLIB_CFLAGS) \
	$(LIBGDKPIXBUF_CFLAGS)


pkgconfig_DATA = libopenraw-gnome-1.0.pc

lib_LTLIBRARIES = libopenrawgnome.la

libopenrawgnome_la_LDFLAGS = \
	-version-info @LIBOPENRAW_VERSION_INFO@

libopenrawgnome_la_LIBADD = ../lib/libopenraw.la \
	$(LIBGLIB_LIBS)                      \
	$(LIBGDKPIXBUF_LIBS)

libopenrawgnome_la_SOURCES = gdkpixbuf.c