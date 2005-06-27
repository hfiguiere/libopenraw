#!/bin/sh



aclocal

automake --add-missing --copy --foreign 
autoconf

