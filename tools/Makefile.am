

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/lib @BOOST_CPPFLAGS@

noinst_PROGRAMS = ordiag


ordiag_SOURCES = ordiag.cpp
ordiag_LDADD = $(top_builddir)/lib/libopenraw.la  -lstdc++ -ljpeg
ordiag_LDFLAGS = -L$(top_builddir)/lib -static
