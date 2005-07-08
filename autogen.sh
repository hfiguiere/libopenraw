#!/bin/sh

#
# part of libopenraw
#

rm -f autogen.err
aclocal >> autogen.err 2>&1

automake --add-missing --copy --foreign 
libtoolize --force
autoconf

