dnl libopenraw configure
dnl written by Hubert Figuiere

AC_PREREQ(2.50)
AC_INIT(libopenraw, 0.1.2)
AM_CONFIG_HEADER(config.h)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign no-dist-gzip dist-bzip2 dist-xz subdir-objects])
AM_MAINTAINER_MODE
m4_ifndef([AM_SILENT_RULES], [m4_define([AM_SILENT_RULES],[])])
AM_SILENT_RULES([yes])


dnl ---------------------------------------------------------------------------
dnl Advanced information about versioning:
dnl   * "Writing shared libraries" by Mike Hearn
dnl         http://plan99.net/~mike/writing-shared-libraries.html
dnl   * libtool.info chapter "Versioning"
dnl   * libtool.info chapter "Updating library version information"
dnl ---------------------------------------------------------------------------
dnl Versioning:
dnl  - CURRENT (Major):  Increment if the interface has changes. AGE is always
dnl                      *changed* at the same time.
dnl  - AGE (Micro):      Increment if any interfaces have been added; set to 0
dnl                      if any interfaces have been removed. Removal has
dnl                      precedence over adding, so set to 0 if both happened.
dnl                      It denotes upward compatibility.
dnl  - REVISION (Minor): Increment any time the source changes; set to
dnl                      0 if you incremented CURRENT.
dnl
dnl  To summarize. Any interface *change* increment CURRENT. If that interface
dnl  change does not break upward compatibility (ie it is an addition),
dnl  increment AGE, Otherwise AGE is reset to 0. If CURRENT has changed,
dnl  REVISION is set to 0, otherwise REVISION is incremented.
dnl ---------------------------------------------------------------------------
dnl version 0.0.3 is 3 2 0
dnl version 0.0.4 is 4 3 0
dnl version 0.0.5 is 5 4 0
dnl version 0.0.6 is 6 5 0
dnl version 0.0.7 is 6.5.1
dnl version 0.0.8 is 6.5.2
dnl version 0.0.9 is 6.5.3
dnl version 0.1.0 is 7.0.0
dnl version 0.1.1 is 7.0.1
dnl version 0.1.2 is 7.0.2
AC_SUBST([LIBOPENRAW_REVISION], [2])
AC_SUBST([LIBOPENRAW_AGE],      [0])
AC_SUBST([LIBOPENRAW_CURRENT],  [7])
AC_SUBST([LIBOPENRAW_CURRENT_MIN],
         [`expr $LIBOPENRAW_CURRENT - $LIBOPENRAW_AGE`])
AC_SUBST([LIBOPENRAW_VERSION_INFO],
         [$LIBOPENRAW_CURRENT:$LIBOPENRAW_REVISION:$LIBOPENRAW_AGE])

dnl the major version. used for package names, etc
LIBOPENRAW_MAJOR_VERSION=$LIBOPENRAW_CURRENT_MIN.0
AC_SUBST(LIBOPENRAW_MAJOR_VERSION)

dnl the path where includes are installed
LIBOPENRAW_INCLUDE_BASE=libopenraw-0.1
AC_SUBST(LIBOPENRAW_INCLUDE_BASE)


dnl --------------------------------------------------------------------
dnl guess directory to install *.pc into
dnl --------------------------------------------------------------------
pkgconfigdir='${libdir}/pkgconfig'
AC_SUBST(pkgconfigdir)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AX_CXX_COMPILE_STDCXX_11(noext,mandatory)

dnl Requirements
EXEMPI_REQUIRED=1.99.5

dnl Needed for GdkPixbuf
LIBGLIB_REQUIRED=2.0.0
dnl Optional GdkPixbuf support
LIBGDKPIXBUF_REQUIRED=2.0.0

AC_SUBST(LIBGLIB_REQUIRED)
AC_SUBST(LIBGDKPIXBUF_REQUIRED)

AC_ARG_WITH(darwinports,[  --with-darwinports     add /opt/local/... to CPP/LDFLAGS (Mac OSX)],[
        if test "x$withval" != "xno"; then
                CPPFLAGS="$CPPFLAGS -I/opt/local/include"
                LDFLAGS="$LDFLAGS -L/opt/local/lib"
		CC=g++
        fi
])
AC_ARG_WITH(fink,[  --with-fink          add /sw/... to CPP/LDFLAGS (Mac OSX)],[
        if test "x$withval" != "xno"; then
                CPPFLAGS="$CPPFLAGS -I/sw/include"
                LDFLAGS="$LDFLAGS -L/sw/lib"
		CC=g++
        fi
])

BOOST_REQUIRE([1.33.0])
BOOST_TEST([s])
BOOST_STRING_ALGO
BOOST_VARIANT

AC_LANG(C)
AC_CHECK_HEADER(jpeglib.h, ,
	AC_MSG_ERROR([jpeglib headers not found]))
dnl PKG_CHECK_MODULES(EXEMPI, exempi-2.0 >= $EXEMPI_REQUIRED)
PKG_CHECK_MODULES(LIBXML, libxml-2.0 >= 2.5.0)

PKG_CHECK_MODULES(CURL, libcurl,
			[AC_DEFINE(HAVE_CURL, 1, [Define to 1 to enable CURL support for testsuite])
			HAVE_CURL=yes],
			[HAVE_CURL=no])
AC_CHECK_FUNCS_ONCE(get_current_dir_name)

#
dnl do we want GNOME ?
#
AC_ARG_ENABLE([gnome],	
              [AC_HELP_STRING([--disable-gnome],[disable support for Gnome])],,
              [enable_gnome=yes])
#
# Checks for libgdkpixbuf
#
if test x$enable_gnome = xyes ; then
   #
   # Checks for libglib-2.0
   # 
   HAVE_LIBGLIB=no
   PKG_CHECK_MODULES(LIBGLIB, glib-2.0 >= $LIBGLIB_REQUIRED,
                    [AC_DEFINE(HAVE_LIBGLIB, 1, [Define to 1 to enable libglib support])
                     HAVE_LIBGLIB=yes],
                    [HAVE_LIBGLIB=no])

   HAVE_LIBGDKPIXBUF=no
   if test x$HAVE_LIBGLIB = xyes ; then
      PKG_CHECK_MODULES(LIBGDKPIXBUF, gdk-pixbuf-2.0 >= $LIBGDKPIXBUF_REQUIRED,
                     [AC_DEFINE(HAVE_LIBGDKPIXBUF, 1, [Define to 1 to enable libgdkpixbuf support])
		      GDK_PIXBUF_DIR=`$PKG_CONFIG --variable=gdk_pixbuf_moduledir gdk-pixbuf-2.0`
			dnl This is because the variable is only defined on recent gdk-pixbuf. 
			dnl Revert to the old method otherwise. (https://bugs.freedesktop.org/show_bug.cgi?id=30013)
			if test -z "$GDK_PIXBUF_DIR" ; then
				GTK_VERSION=`$PKG_CONFIG --variable=gtk_binary_version gtk+-2.0`
				GDK_PIXBUF_DIR="$libdir/gtk-2.0/$GTK_VERSION/loaders"
			fi
		      AC_SUBST(GDK_PIXBUF_DIR)
		      HAVE_LIBGDKPIXBUF=yes],
                     [HAVE_LIBGDKPIXBUF=no])
   else
      AC_MSG_WARN([glib-2.0 missing, ignore GdkPixbuf support.])
   fi
   if test x$HAVE_LIBGDKPIXBUF = xyes ; then
     HAVE_GNOME=yes
   else
     HAVE_GNOME=no
   fi
else
   HAVE_GNOME=no
   AC_MSG_CHECKING([for Gnome])
   AC_MSG_RESULT([disabled])
fi

AM_CONDITIONAL(BUILD_GNOME_SUPPORT, test x$HAVE_GNOME = xyes)

AX_CFLAGS_GCC_OPTION([-Wextra])
AX_CXXFLAGS_GCC_OPTION([-Wextra  -Wsign-compare -Wpointer-arith -Wchar-subscripts -Wwrite-strings -Wunused -Wpointer-arith -Wsuggest-override])

CXXFLAGS="$CXXFLAGS -pedantic -g -Wall -Wshadow"
CFLAGS="$CFLAGS -pedantic -g -Wall"

AC_ARG_ENABLE(asan,[  --enable-asan    Turn on address sanitizer],[
    AX_CFLAGS_GCC_OPTION([-fsanitize=address -fno-omit-frame-pointer])
    AX_CXXFLAGS_GCC_OPTION([-fsanitize=address -fno-omit-frame-pointer])
]
,
[

])

dnl we need to disable gdk pixbuf for distcheck
DISTCHECK_CONFIGURE_FLAGS="--disable-gnome"
AC_SUBST([DISTCHECK_CONFIGURE_FLAGS])

AC_OUTPUT([
Makefile
include/Makefile
include/libopenraw/Makefile
include/libopenraw++/Makefile
lib/Makefile
lib/libopenraw-0.1.pc
gnome/Makefile
gnome/libopenraw-gnome-0.1.pc
gnome/include/Makefile
gnome/include/libopenraw-gnome/Makefile
doc/Makefile
doc/Doxyfile
doc/doxygen/Makefile
test/Makefile
testsuite/Makefile
tools/Makefile
demo/Makefile
])


echo "
Options:

  Gnome support:        ${HAVE_GNOME}
  Testsuite booststrap: ${HAVE_CURL}
"
