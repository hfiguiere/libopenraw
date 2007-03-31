#!/bin/sh

#
# part of libopenraw
#


topsrcdir=`dirname $0`
if test x$topsrcdir = x ; then
        topsrcdir=.
fi

builddir=`pwd`

AUTOCONF=autoconf
if test -x /usr/bin/glibtool ; then
    LIBTOOL=glibtool
else
    LIBTOOL=libtool
fi
if test -x /usr/bin/glibtoolize ; then
    LIBTOOLIZE=glibtoolize
else
    LIBTOOLIZE=libtoolize
fi
AUTOMAKE=automake
ACLOCAL=aclocal

cd $topsrcdir

rm -f autogen.err
$ACLOCAL -I m4 >> autogen.err 2>&1

$AUTOMAKE --add-missing --copy --foreign 
$LIBTOOLIZE --force
autoheader --force
$AUTOCONF

cd $builddir

if test -z "$*"; then
        echo "I am going to run ./configure with --enable-maintainer-mode"
	echo "If you wish to pass any to it, please specify them on "
	echo "the $0 command line."
fi

echo "Running configure..."
$topsrcdir/configure --enable-maintainer-mode "$@"
