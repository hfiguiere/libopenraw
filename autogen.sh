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
LIBTOOL=libtool
AUTOMAKE=automake-1.9
ACLOCAL=aclocal-1.9

cd $topsrcdir

rm -f autogen.err
$ACLOCAL >> autogen.err 2>&1

$AUTOMAKE --add-missing --copy --foreign 
libtoolize --force
$AUTOCONF

cd $builddir

if test -z "$*"; then
        echo "I am going to run ./configure with no arguments - if you wish "
        echo "to pass any to it, please specify them on the $0 command line."
fi

echo "Running configure..."
$topsrcdir/configure "$@"
