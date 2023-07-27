

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/lib @BOOST_CPPFLAGS@

noinst_PROGRAMS = ordiag exifdump identify

ordiag_SOURCES = ordiag.cpp dumputils.cpp dumputils.hpp
ordiag_LDADD = $(top_builddir)/lib/libopenraw.la -ljpeg -lm

exifdump_SOURCES = exifdump.cpp dumputils.cpp dumputils.hpp
exifdump_LDADD = $(top_builddir)/lib/libopenraw.la -ljpeg -lm

identify_SOURCES = identify.cpp
identify_LDADD = $(top_builddir)/lib/libopenraw.la -ljpeg -lm
