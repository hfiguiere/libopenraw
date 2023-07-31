

noinst_PROGRAMS = extensions thumbc ccfa ppmload

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_sr2cdir)/lib \
	@BOOST_CPPFLAGS@

LIBOPENRAW_LIBS = $(top_builddir)/lib/libopenraw.la -lm

extensions_SOURCES = extensions.cpp
extensions_LDADD = $(LIBOPENRAW_LIBS)

thumbc_SOURCES = thumbc.c
thumbc_LDADD = $(LIBOPENRAW_LIBS)

ccfa_SOURCES = ccfa.c
ccfa_LDADD = $(LIBOPENRAW_LIBS)

ppmload_SOURCES = ppmload.c
ppmload_LDADD = $(LIBOPENRAW_LIBS)
