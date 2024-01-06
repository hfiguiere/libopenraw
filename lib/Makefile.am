
AM_CPPFLAGS = -I$(top_srcdir)/include @BOOST_CPPFLAGS@


RUST_FILES = mp4/Cargo.lock \
	mp4/Cargo.toml \
	mp4/lib.rs \
	mp4/mp4parse_capi/src/lib.rs \
	mp4/mp4parse_capi/Cargo.toml \
	mp4/mp4parse/src/boxes.rs \
	mp4/mp4parse/src/lib.rs \
	mp4/mp4parse/src/macros.rs \
	mp4/mp4parse/src/tests.rs \
	mp4/mp4parse/src/craw.rs \
	mp4/mp4parse/src/unstable.rs \
	mp4/mp4parse/Cargo.toml \
	$(NULL)


EXTRA_DIST = \
	exif/exif-tags.pl \
	mp4/mp4parse/README.md \
	$(RUST_FILES)


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
