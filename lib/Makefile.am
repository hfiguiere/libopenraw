
AM_CPPFLAGS = -I$(top_srcdir)/include @BOOST_CPPFLAGS@


EXTRA_DIST = \
	exif/exif-tags.pl

update_tags:
	$(srcdir)/exif/exif-tags.pl > $(srcdir)/../src/tiff/exif/generated.rs

pkgconfig_DATA = libopenraw-0.4.pc

noinst_HEADERS = \
	option.hpp \
	$(NULL)

lib_LTLIBRARIES = libopenraw.la

libopenraw_la_LDFLAGS = \
	$(BSYMBOLIC_LDFLAG) \
	-version-info @LIBOPENRAW_VERSION_INFO@ \
	-export-dynamic \
	-no-undefined \
	-export-symbols-regex "^or_.*" \
	$(AM_LDFLAGS)
libopenraw_la_LIBADD = ../libopenraw.la
libopenraw_la_SOURCES = \
	$(NULL)
