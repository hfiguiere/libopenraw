dnl libopenraw configure
dnl written by Hubert Figuiere

AC_PREREQ([2.69])
AC_INIT([libopenraw],[0.4.0-alpha.2])
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
dnl version 0.1.3 is 7.0.3
dnl version 0.2.0 is 8.0.0
dnl version 0.2.1 is 8.0.1
dnl version 0.2.2 is 8.0.2
dnl version 0.2.3 is 8.0.3
dnl version 0.3.0 is 9.0.0
dnl version 0.3.1 is 9.0.1
dnl version 0.3.2 is 9.0.2
dnl version 0.3.3 is 9.0.3
dnl version 0.3.4 is 9.0.4
dnl version 0.3.5 is 9.0.5
dnl version 0.3.6 is 9.0.6
dnl version 0.3.7 is 9.0.7
dnl This version is now built in Rust. Break ABI.
dnl version 0.4.0 is 10.0.0
AC_SUBST([LIBOPENRAW_REVISION], [0])
AC_SUBST([LIBOPENRAW_AGE],      [0])
AC_SUBST([LIBOPENRAW_CURRENT],  [10])
AC_SUBST([LIBOPENRAW_CURRENT_MIN],
         [`expr $LIBOPENRAW_CURRENT - $LIBOPENRAW_AGE`])
AC_SUBST([LIBOPENRAW_VERSION_INFO],
         [$LIBOPENRAW_CURRENT:$LIBOPENRAW_REVISION:$LIBOPENRAW_AGE])

dnl the major version. used for package names, etc
LIBOPENRAW_MAJOR_VERSION=$LIBOPENRAW_CURRENT_MIN.0
AC_SUBST(LIBOPENRAW_MAJOR_VERSION)

dnl the path where includes are installed
LIBOPENRAW_INCLUDE_BASE=libopenraw-0.4
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
LT_INIT
AX_CXX_COMPILE_STDCXX_14(noext,mandatory)

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

dnl Approximate minimum version.
dnl Feel free to adjust.
BOOST_REQUIRE([1.60.0])
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

AC_ARG_ENABLE(debug,[  --enable-debug    Turn on debugging],[
        case "${enableval}" in
           yes) debug=true ;
                DEBUG_CFLAGS="-DDEBUG -g" ;
                OPTIMIZE_CFLAGS="" ;;
            no) debug=false ;
                DEBUG_CFLAGS="-DNDEBUG" ;;
             *) AC_MSG_ERROR(bad value ${enableval} for --enable-debug) ;;
        esac
],[     debug=false
        DEBUG_CFLAGS="-DNDEBUG"
])
AM_CONDITIONAL(DEBUG, test x$debug = xtrue)

AC_LANG([C++])
AC_LANG_COMPILER_REQUIRE

AX_APPEND_COMPILE_FLAGS([-fvisibility=hidden])

AX_CHECK_COMPILE_FLAG([-Werror=unknown-warning-option], [
        ax_compiler_flags_test="-Werror=unknown-warning-option"
], [
        ax_compiler_flags_test=""
])
AX_APPEND_COMPILE_FLAGS([ \
   -Wchar-subscripts \
   -Wcast-align \
   -Wextra \
   -Wexceptions \
   -Wheader-guard \
   -Wformat \
   -Wformat-nonliteral \
   -Wformat-security \
   -Wno-gnu-zero-variadic-macro-arguments \
   -Wlogical-not-parentheses \
   -Wmisleading-indentation \
   -Wmissing-noreturn \
   -Woverloaded-virtual \
   -Wpointer-arith \
   -Wredundant-decls \
   -Wpointer-bool-conversion \
   -Wshadow \
   -Wsign-compare \
   -Wno-suggest-attribute=noreturn \
   -Wsuggest-override \
   -Wnull-conversion \
   -Wunreachable-code \
   -Wunused \
   -Wwrite-strings \
], [], [$ax_compiler_flags_test])

AC_ARG_ENABLE(asan,[  --enable-asan    Turn on address sanitizer],[
    AX_APPEND_COMPILE_FLAGS([-fsanitize=address -fno-omit-frame-pointer])
    AC_LANG([C])
    AX_APPEND_COMPILE_FLAGS([-fsanitize=address -fno-omit-frame-pointer])
]
,
[

])

AC_CHECK_PROG(CARGO, [cargo], [yes], [no])
AS_IF(test x$CARGO = xno,
    AC_MSG_ERROR([cargo is required])
)
AC_CHECK_PROG(RUSTC, [rustc], [yes], [no])
AS_IF(test x$RUSTC = xno,
    AC_MSG_ERROR([rustc is required])
)

dnl Release build we do.
if test x$debug = xtrue; then
CARGO_TARGET_SUBDIR=debug
else
CARGO_TARGET_SUBDIR=release
fi
AC_SUBST(CARGO_TARGET_SUBDIR)

AC_CONFIG_FILES([
Makefile
include/Makefile
include/libopenraw/Makefile
lib/Makefile
lib/libopenraw-0.4.pc
doc/Makefile
doc/Doxyfile
doc/doxygen/Makefile
test/Makefile
testsuite/Makefile
tools/Makefile
demo/Makefile
])
AC_OUTPUT


echo "
Options:

  Testsuite booststrap: ${HAVE_CURL}
"
