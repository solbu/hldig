dnl
dnl Local autoconf definitions. Try to follow the guidelines of the autoconf
dnl macro repository so that integration in the repository is easy.
dnl To submit a macro to the repository send the macro (one macro per mail)
dnl to Peter Simons <simons@cys.de>.
dnl The repository itself is at http://peti.cys.de/autoconf-archive/
dnl

dnl @synopsis CHECK_USER()
dnl
dnl Defines the USER symbol from LOGNAME or USER environment variable,
dnl depending on which one is filled.
dnl
dnl Usage example in Makefile.am:
dnl   
dnl   program $(USER)
dnl
dnl or in Makefile.in:
dnl 
dnl   program @USER@
dnl
dnl @version $Id: acinclude.m4,v 1.7 1999/09/29 13:07:17 loic Exp $
dnl @author Loic Dachary <loic@senga.org>
dnl

AC_DEFUN(CHECK_USER,
[AC_MSG_CHECKING(user name)
test -n "$LOGNAME" && USER=$LOGNAME
AC_SUBST(USER)
AC_MSG_RESULT($USER)
])

dnl @synopsis AC_COMPILE_WARNINGS
dnl
dnl Set the maximum warning verbosity according to compiler used.
dnl Currently supports g++ and gcc.
dnl This macro must be put after AC_PROG_CC and AC_PROG_CXX in
dnl configure.in
dnl
dnl @version $Id: acinclude.m4,v 1.7 1999/09/29 13:07:17 loic Exp $
dnl @author Loic Dachary <loic@senga.org>
dnl

AC_DEFUN(AC_COMPILE_WARNINGS,
[AC_MSG_CHECKING(maximum warning verbosity option)
if test -n "$CXX"
then
  if test "$GXX" = "yes"
  then
    ac_compile_warnings_opt='-Wall'
  fi
  CXXFLAGS="$CXXFLAGS $ac_compile_warnings_opt"
  ac_compile_warnings_msg="$ac_compile_warnings_opt for C++"
fi

if test -n "$CC"
then
  if test "$GCC" = "yes"
  then
    ac_compile_warnings_opt='-Wall'
  fi
  CFLAGS="$CFLAGS $ac_compile_warnings_opt"
  ac_compile_warnings_msg="$ac_compile_warnings_msg $ac_compile_warnings_opt for C"
fi
AC_MSG_RESULT($ac_compile_warnings_msg)
unset ac_compile_warnings_msg
unset ac_compile_warnings_opt
])

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
dnl @version $Id: acinclude.m4,v 1.7 1999/09/29 13:07:17 loic Exp $
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

dnl @synopsis AC_PROG_APACHE()
dnl
dnl This macro searches for an installed apache server. If nothing
dnl was specified when calling configure, it searches in 
dnl /usr/local/bin:/usr/local/sbin:/usr/bin:/usr/sbin.
dnl If the --with-zlib=DIR is specified, it will try
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
dnl @version $Id: acinclude.m4,v 1.7 1999/09/29 13:07:17 loic Exp $
dnl @author Loic Dachary <loic@senga.org>
dnl

AC_DEFUN(AC_PROG_APACHE,
#
# Handle user hints
#
[AC_MSG_CHECKING(if apache is wanted)
AC_ARG_WITH(apache,
[  --with-apache=DIR root directory path of apache installation [defaults to
		    /usr/local or /usr if not found in /usr/local]
  --without-apache to disable apache detection],
[if test "$withval" != no ; then
  AC_MSG_RESULT(yes)
  APACHE_PATH="$withval/bin:$withval/sbin"
else
  AC_MSG_RESULT(no)
fi], [
AC_MSG_RESULT(yes)
APACHE_PATH=/usr/local/apache/bin:/usr/local/bin:/usr/local/sbin:/usr/bin:/usr/sbin
])

#
# Locate apache, if wanted
#
if test -n "${APACHE_PATH}"
then
	AC_PATH_PROG(APACHE, httpd, , $APACHE_PATH)
fi

])

## ----------------------------------------------- ##
## Check time prog path library and options.       ##
## ----------------------------------------------- ##

AC_DEFUN(AM_PROG_TIME, [
AC_PATH_PROG(TIME, time, time)
#
# Try various flags for verbose time information, 
# if none works TIMEV is the same as TIME
#
AC_MSG_CHECKING(verbose time flag)
for timev in "$TIME -v" "$TIME -l" $TIME
do
	if $timev >/dev/null 2>&1
	then
		TIMEV=$timev
		break
	fi
done
AC_MSG_RESULT($TIMEV)
AC_SUBST(TIMEV)
])

dnl @synopsis AC_FUNC_STRPTIME()
dnl
dnl This macro checks that the function strptime exists and that
dnl it is declared in the time.h header. 
dnl
dnl Here is an example of its use:
dnl
dnl strptime.c replacement:
dnl
dnl #ifndef HAVE_STRPTIME
dnl ....
dnl #endif /* HAVE_STRPTIME */
dnl
dnl In sources using strptime
dnl
dnl #ifndef HAVE_STRPTIME_DECL
dnl extern char *strptime(const char *__s, const char *__fmt, struct tm *__tp);
dnl #endif /* HAVE_STRPTIME_DECL */
dnl
dnl @author Loic Dachary <loic@senga.org>
dnl @version 1.0
dnl

AC_DEFUN(AC_FUNC_STRPTIME, [
AC_CHECK_FUNCS(strptime)
AC_MSG_CHECKING(for strptime declaration in time.h)
AC_EGREP_HEADER(strptime, time.h, [
 AC_DEFINE(HAVE_STRPTIME_DECL)
 AC_MSG_RESULT(yes)
], [
 AC_MSG_RESULT(no)
])

])
