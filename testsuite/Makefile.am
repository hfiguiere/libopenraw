
TESTS = test_xmlhandler testsuite
TESTS_ENVIRONMENT =

OPENRAW_LIB = $(top_builddir)/lib/libopenraw.la

AM_CPPFLAGS = \
	@BOOST_CPPFLAGS@ \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/lib \
	@LIBXML_CFLAGS@ @CURL_CFLAGS@

noinst_PROGRAMS = testsuite
noinst_HEADERS = testsuitetags.h

check_PROGRAMS = test_xmlhandler

EXTRA_DIST = testsuitetags.pl testsuitetags.txt test.xml testsuite.xml
BUILT_SOURCES = $(srcdir)/testsuitetags.cpp $(srcdir)/testsuitetags.h
CLEANFILES = $(BUILT_SOURCES)

test_xmlhandler_SOURCES = test_xmlhandler.cpp xmlhandler.cpp xmlhandler.h
test_xmlhandler_LDADD = @LIBXML_LIBS@

testsuite_SOURCES =  testsuitetags.cpp testsuite.h \
	testsuite.cpp testsuitehandler.h testsuitehandler.cpp \
	xmlhandler.cpp xmlhandler.h
testsuite_LDFLAGS = -static
testsuite_LDADD = $(OPENRAW_LIB) @LIBXML_LIBS@ @CURL_LIBS@

$(srcdir)/testsuitetags.cpp: testsuitetags.pl testsuitetags.txt
	$(srcdir)/testsuitetags.pl -cpp < $(srcdir)/testsuitetags.txt > $@

$(srcdir)/testsuitetags.h: testsuitetags.pl testsuitetags.txt
	$(srcdir)/testsuitetags.pl -h < $(srcdir)/testsuitetags.txt > $@
