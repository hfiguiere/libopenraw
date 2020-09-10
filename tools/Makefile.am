

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/lib @BOOST_CPPFLAGS@

noinst_PROGRAMS = ordiag exifdump

ordiag_SOURCES = ordiag.cpp dumputils.cpp dumputils.hpp
ordiag_LDADD = $(top_builddir)/lib/libopenraw.la -ljpeg

exifdump_SOURCES = exifdump.cpp dumputils.cpp dumputils.hpp
exifdump_LDADD = $(top_builddir)/lib/libopenraw.la -ljpeg
