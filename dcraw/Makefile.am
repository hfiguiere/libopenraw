
noinst_LIBRARIES = libdcraw.a libdcparse.a

CLEANFILES = dcraw.c lib-dcraw.c lib-parse.c

dcraw.c: $(top_srcdir)/dcraw/dcraw.c,v
	co $<

lib-dcraw.c: dcraw.c
	rm -f $@
	$(PERL) $(top_srcdir)/tools/dcraw-convert.pl < $< > $@ 

lib-parse.c: parse.c
	rm -f $@
	$(PERL) $(top_srcdir)/tools/dcparse-convert.pl < $< > $@ 

libdcraw_a_SOURCES = lib-dcraw.c

libdcparse_a_SOURCES = lib-parse.c
