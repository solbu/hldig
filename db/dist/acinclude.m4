dnl @synopsis CHECK_ZLIB()
dnl
dnl This macro searches for an installed zlib library. If nothing
dnl was specified when calling configure, it searches first in /usr/local
dnl and then in /usr. If the --with-zlib=DIR is specified, it will try
dnl to find it in DIR/include/zlib.h and DIR/lib/libz.a. If --without-zlib
dnl is specified, the library is not searched at all.
dnl
dnl It defines the symbol HAVE_LIBZ if the library is found. You should
dnl use autoheader to include a definition for this symbol in a config.h
dnl file.
dnl
dnl Sources files should then use something like
dnl
dnl   #ifdef HAVE_LIBZ
dnl   #include <zlib.h>
dnl   #endif /* HAVE_LIBZ */
dnl
dnl @version $Id: acinclude.m4,v 1.2 1999/09/01 13:32:31 loic Exp $
dnl @author Loic Dachary <loic@senga.org>
dnl

AC_DEFUN(CHECK_ZLIB,
#
# Handle user hints
#
[AC_MSG_CHECKING(if zlib is wanted)
AC_ARG_WITH(zlib,
[  --with-zlib=DIR root directory path of zlib installation [defaults to
		    /usr/local or /usr if not found in /usr/local]
  --without-zlib to disable zlib usage completely],
[if test "$withval" != no ; then
  AC_MSG_RESULT(yes)
  ZLIB_HOME="$withval"
else
  AC_MSG_RESULT(no)
fi], [
AC_MSG_RESULT(yes)
ZLIB_HOME=/usr/local
if test ! -f "${ZLIB_HOME}/include/zlib.h"
then
	ZLIB_HOME=/usr
fi
])

#
# Locate zlib, if wanted
#
if test -n "${ZLIB_HOME}"
then
	LDFLAGS="$LDFLAGS -L${ZLIB_HOME}/lib"
	CPPFLAGS="$CPPFLAGS -I${ZLIB_HOME}/include"
	AC_CHECK_LIB(z, inflateEnd)
fi

])
