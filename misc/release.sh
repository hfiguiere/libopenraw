#!/bin/sh


DIR=`dirname $0`/..

VERSION=`head -1 $DIR/NEWS | cut -d ' ' -f 2`

if test -z '$VERSION'; then
    echo "Can't figure out version"
    exit 1
fi

if [ ! -f libopenraw-$VERSION.tar.bz2 -o ! -f libopenraw-$VERSION.tar.xz ] ; then
    make distcheck || exit 2
fi

echo "Signing packages. You'll be asked for the passphrase"

for f in libopenraw-$VERSION.tar.bz2 libopenraw-$VERSION.tar.xz ; do
    rm -f $f.asc
    gpg -ba $f
    echo "File $f ready"
    echo "Signature $f.asc"
done

echo "Copy the files above into 'static/download'"

# REMOTE_DEST=annarchy.freedesktop.org:/srv/libopenraw.freedesktop.org/www/download/

# scp libopenraw-$VERSION.tar.bz2 libopenraw-$VERSION.tar.bz2.asc annarchy.freedesktop.org:/srv/libopenraw.freedesktop.org/www/download/
