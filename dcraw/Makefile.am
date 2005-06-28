
noinst_LIBRARIES = libdcraw.a libdcparse.a

dcraw.c: $(top_srcdir)/dcraw/dcraw.c,v
	co $@

lib-dcraw.c: dcraw.c
	rm -f $@
	$(PERL) $(top_srcdir)/tools/dcraw-convert.pl < $< > $@ 

libdcraw_a_SOURCES = lib-dcraw.c

libdcparse_a_SOURCES = parse.c
