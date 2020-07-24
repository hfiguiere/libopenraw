

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/lib @BOOST_CPPFLAGS@

noinst_PROGRAMS = ordiag exifdump

ordiag_SOURCES = ordiag.cpp dumputils.cpp
ordiag_LDADD = $(top_builddir)/lib/libopenraw.la -ljpeg

exifdump_SOURCES = exifdump.cpp dumputils.cpp exif_tags.cpp exif_tags.hpp
exifdump_LDADD = $(top_builddir)/lib/libopenraw.la -ljpeg

EXTRA_DIST = exif-tags.pl

update_tags:
	$(srcdir)/exif-tags.pl > $(srcdir)/exif_tags.cpp
