ACLOCAL_AMFLAGS = -I m4

SUBDIRS = lib gnome doc test testsuite demo tools include

dox:
	make -C doc dox

webdist: dox
	$(top_srcdir)/scripts/webdist.sh ${top_builddir} ${VERSION}

EXTRA_DIST = RELEASE_NOTES
