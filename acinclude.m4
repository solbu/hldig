dnl
dnl Local autoconf definitions. Try to follow the guidelines of the autoconf
dnl macro repository so that integration in the repository is easy.
dnl To submit a macro to the repository send the macro (one macro per mail)
dnl to Peter Simons <simons@cys.de>.
dnl

## ----------------------------------- ##
## Check if zlib library.              ##
## ----------------------------------- ##

AC_DEFUN(AM_WITH_ZLIB,
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
