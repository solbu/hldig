dnl
dnl Part of the hl://Dig package   <https://github.com/solbu/hldig>
dnl Copyright (c) 1999-2004 The ht://Dig Group
dnl For copyright details, see the file COPYING in your distribution
dnl or the GNU Library General Public License (LGPL) version 2 or later
dnl <http://www.gnu.org/copyleft/lgpl.html>
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
dnl @version $Id: acinclude.m4,v 1.19 2004/05/28 13:15:10 lha Exp $
dnl @author Loic Dachary <loic@senga.org>
dnl

AC_DEFUN([CHECK_USER],
[AC_MSG_CHECKING([user name])
  test -n "$LOGNAME" && USER=$LOGNAME
AC_SUBST(USER)
AC_MSG_RESULT($USER)
])

dnl
dnl Prevent accidental use of Run Time Type Information g++ builtin
dnl functions.
dnl
AC_DEFUN([NO_RTTI],
[AC_MSG_CHECKING(adding -fno-rtti to g++)
if test -n "$CXX"
then
  if test "$GXX" = "yes"
  then
    CXXFLAGS_save="$CXXFLAGS"
    CXXFLAGS="$CXXFLAGS -fno-rtti"
    AC_LANG_SAVE
    AC_LANG_CPLUSPLUS
    AC_TRY_COMPILE(,,,CXXFLAGS="$CXXFLAGS_save")
    AC_LANG_RESTORE
  fi
fi
AC_MSG_RESULT(ok)
])

dnl
dnl Prevent accidental use of Exceptions g++ builtin
dnl functions.
dnl
AC_DEFUN([NO_EXCEPTIONS],
[AC_MSG_CHECKING(adding -fno-exceptions to g++)
if test -n "$CXX"
then
  if test "$GXX" = "yes"
  then
    CXXFLAGS_save="$CXXFLAGS"
    CXXFLAGS="$CXXFLAGS -fno-exceptions"
    AC_LANG_SAVE
    AC_LANG_CPLUSPLUS
    AC_TRY_COMPILE(,,,CXXFLAGS="$CXXFLAGS_save")
    AC_LANG_RESTORE
  fi
fi
AC_MSG_RESULT(ok)
])

dnl @synopsis AC_COMPILE_WARNINGS
dnl
dnl Set the maximum warning verbosity according to compiler used.
dnl Currently supports g++ and gcc.
dnl This macro must be put after AC_PROG_CC and AC_PROG_CXX in
dnl configure.in
dnl
dnl @version $Id: acinclude.m4,v 1.19 2004/05/28 13:15:10 lha Exp $
dnl @author Loic Dachary <loic@senga.org>
dnl

AC_DEFUN([AC_COMPILE_WARNINGS],
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

ac_compile_warnings_opt=
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
dnl If either the header file (zlib.h) or the library (libz) is not
dnl found, the configuration exits on error, asking for a valid
dnl zlib installation directory or --without-zlib.
dnl
dnl The macro defines the symbol HAVE_LIBZ if the library is found. You should
dnl use autoheader to include a definition for this symbol in a config.h
dnl file. Sample usage in a C/C++ source is as follows:
dnl
dnl   #ifdef HAVE_LIBZ
dnl   #include <zlib.h>
dnl   #endif /* HAVE_LIBZ */
dnl
dnl @version $Id: acinclude.m4,v 1.19 2004/05/28 13:15:10 lha Exp $
dnl @author Loic Dachary <loic@senga.org>
dnl

AC_DEFUN([CHECK_ZLIB],
#
# Handle user hints
#
[AC_MSG_CHECKING(if zlib is wanted)
AC_ARG_WITH(zlib,
[  --with-zlib=DIR         root directory path of zlib installation
  --without-zlib          to disable zlib usage completely],
[if test "$withval" = no ; then
  AC_MSG_RESULT(no)
else
  AC_MSG_RESULT(yes)
  if test "$withval" = yes ; then
    ZLIB_HOME="default path"
  else
    LDFLAGS="$LDFLAGS -L$withval/lib"
    CPPFLAGS="$CPPFLAGS -I$withval/include"
    ZLIB_HOME="$withval"
  fi
fi], [
AC_MSG_RESULT(yes)
ZLIB_HOME="default path"
])

#
# Locate zlib, if wanted
#
if test -n "${ZLIB_HOME}"
then
    AC_LANG_SAVE
    AC_LANG_C
    AC_MSG_CHECKING(for zlib in $ZLIB_HOME)
    AC_CHECK_HEADER(zlib.h, [zlib_cv_zlib_h=yes], [zlib_cv_zlib_h=no])
    dnl Only check for library if header is found.  This check sets HAVE_LIBZ
    if test "$zlib_cv_zlib_h" = yes; then
	AC_DEFINE([HAVE_ZLIB_H],,[Define if Zlib is enabled])
	AC_CHECK_LIB(z, inflateEnd)
    fi
    if test "${ac_cv_lib_z_inflateEnd:+yes}" != yes
    then
	#
	# If either header or library was not found, bomb
	#
	AC_MSG_RESULT(failed)
	AC_MSG_ERROR(Either specify a valid zlib installation with --with-zlib=DIR or disable zlib   usage with --without-zlib.)
    fi
    AC_LANG_RESTORE
fi

])

dnl @synopsis AC_PROG_APACHE([version])
dnl
dnl This macro searches for an installed apache server. If nothing
dnl was specified when calling configure or just --with-apache, it searches in
dnl /usr/local/apache/bin:/usr/local/bin:/usr/local/sbin:/usr/bin:/usr/sbin
dnl The argument of --with-apache specifies the full pathname of the
dnl httpd argument. For instance --with-apache=/usr/sbin/httpd.
dnl
dnl If the version argument is given, AC_PROG_APACHE checks that the
dnl apache server is this version number or higher.
dnl
dnl If the apache server is not found, abort configuration with error
dnl message.
dnl
dnl It defines the symbol APACHE if the server is found.
dnl
dnl Files using apache should do the following:
dnl
dnl   @APACHE@ -d /etc/httpd
dnl
dnl It defines the symbol APACHE_MODULES if a directory containing mod_env.*
dnl is found in the default server root directory (obtained with httpd -V).
dnl
dnl The httpd.conf file listing modules to be loaded dynamicaly can use
dnl @APACHE_MODULES@ to grab them in the appropriate sub directory. For
dnl instance:
dnl ...
dnl <IfModule mod_so.c>
dnl LoadModule env_module         @APACHE_MODULES@/mod_env.so
dnl LoadModule config_log_module  @APACHE_MODULES@/mod_log_config.so
dnl ...
dnl
dnl @version $Id: acinclude.m4,v 1.19 2004/05/28 13:15:10 lha Exp $
dnl @author Loic Dachary <loic@senga.org>
dnl

AC_DEFUN([AC_PROG_APACHE],
#
# Handle user hints
#
[
 AC_MSG_CHECKING(if apache is wanted)
 AC_ARG_WITH(apache,
  [  --with-apache=PATH      absolute path name of apache server
			    (default is to search httpd in
    /usr/local/apache/bin:/usr/local/bin:/usr/local/sbin:/usr/bin:/usr/sbin),
  --without-apache        to disable apache detection],
  [
    #
    # Run this if -with or -without was specified
    #
    if test "$withval" != no ; then
       AC_MSG_RESULT(yes)
       APACHE_WANTED=yes
       if test "$withval" != yes ; then
         APACHE="$withval"
       fi
    else
       APACHE_WANTED=no
       AC_MSG_RESULT(no)
    fi
  ], [
    #
    # Run this if nothing was said
    #
    APACHE_WANTED=yes
    AC_MSG_RESULT(yes)
  ])
  #
  # Now we know if we want apache or not, only go further if
  # it's wanted.
  #
  if test "$APACHE_WANTED" = yes ; then
    #
    # If not specified by caller, search in standard places
    #
    if test -z "$APACHE" ; then
      AC_PATH_PROGS(APACHE, httpd apache, , /usr/local/apache/bin:/usr/local/bin:/usr/local/sbin:/usr/bin:/usr/sbin:/usr/apache/bin)
    fi
    AC_SUBST(APACHE)
    if test -z "$APACHE" ; then
	AC_MSG_ERROR("apache server executable not found");
    fi
    #
    # Collect apache version number. If for nothing else, this
    # guaranties that httpd is a working apache executable.
    # (Want at least 3 chars in version, to skip  "(Mandrake Linux/4mdk)")
    #
    changequote(<<, >>)dnl
    APACHE_READABLE_VERSION=`$APACHE -v | grep 'Server version' | sed -e 's;.*/\([0-9\.][0-9\.][0-9\.][0-9\.]*\).*;\1;'`
    changequote([, ])dnl
    APACHE_VERSION=`echo $APACHE_READABLE_VERSION | sed -e 's/\.//g'`
    if test -z "$APACHE_VERSION" ; then
	AC_MSG_ERROR("could not determine apache version number");
    fi
    APACHE_MAJOR=`expr $APACHE_VERSION : '\(..\)'`
    APACHE_MINOR=`expr $APACHE_VERSION : '..\(.*\)'`
    #
    # Check that apache version matches requested version or above
    #
    if test -n "$1" ; then
      AC_MSG_CHECKING(apache version >= $1)
      APACHE_REQUEST=`echo $1 | sed -e 's/\.//g'`
      APACHE_REQUEST_MAJOR=`expr $APACHE_REQUEST : '\(..\)'`
      APACHE_REQUEST_MINOR=`expr $APACHE_REQUEST : '..\(.*\)'`
      if test "$APACHE_MAJOR" -lt "$APACHE_REQUEST_MAJOR" -o "$APACHE_MINOR" -lt "$APACHE_REQUEST_MINOR" ; then
        AC_MSG_RESULT(no)
        AC_MSG_ERROR(apache version is $APACHE_READABLE_VERSION)
      else
        AC_MSG_RESULT(yes)
      fi
    fi
    #
    # Find out if .so modules are in libexec/module.so or modules/module.so
    #
    HTTP_ROOT=`$APACHE -V | grep HTTPD_ROOT | sed -e 's/.*"\(.*\)"/\1/'`
    AC_MSG_CHECKING(apache modules)
    for dir in libexec modules lib/apache libexec/httpd lib/apache/1.3
    do
      if test -f $HTTP_ROOT/$dir/mod_env.*
      then
	APACHE_MODULES=$dir
      fi
    done
    if test -z "$APACHE_MODULES"
    then
      AC_MSG_RESULT(not found)
    else
      AC_MSG_RESULT(in $HTTP_ROOT/$APACHE_MODULES)
    fi
    AC_SUBST(APACHE_MODULES)
  fi
])

## ----------------------------------------------- ##
## Check time prog path library and options.       ##
## ----------------------------------------------- ##

AC_DEFUN([AM_PROG_TIME], [
AC_PATH_PROG(TIME, time, time)
#
# Try various flags for verbose time information,
# if none works TIMEV is the same as TIME
#
AC_MSG_CHECKING(verbose time flag)
for timev in "$TIME -v" "$TIME -l" $TIME
do
	if $timev echo >/dev/null 2>&1
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

AC_DEFUN([AC_FUNC_STRPTIME], [
    AC_CHECK_FUNCS(strptime)
    AC_MSG_CHECKING(for strptime declaration in time.h)
    AC_EGREP_HEADER(strptime, time.h, [
        AC_DEFINE([HAVE_STRPTIME_DECL],,[Define if the function strptime is declared in <time.h>])
    AC_MSG_RESULT(yes)], [AC_MSG_RESULT(no)
    ])
])

dnl If the compiler supports ISO C++ standard library (i.e., can include the
dnl files iostream, map, iomanip and cmath), define HAVE_STD.
AC_DEFUN([AC_CXX_HAVE_STD],
[AC_CACHE_CHECK(whether the compiler supports ISO C++ standard library,
ac_cv_cxx_have_std,
[AC_REQUIRE([AC_CXX_NAMESPACES])
 AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([#include <iostream>
#include <map>
#include <iomanip>
#include <cmath>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif],[return 0;],
 ac_cv_cxx_have_std=yes, ac_cv_cxx_have_std=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_have_std" = yes; then
  AC_DEFINE(HAVE_STD,,[define if the compiler supports ISO C++ standard
library])
fi
])

dnl If the compiler can prevent names clashes using namespaces, define
dnl HAVE_NAMESPACES.
AC_DEFUN([AC_CXX_NAMESPACES],
[AC_CACHE_CHECK(whether the compiler implements namespaces,
ac_cv_cxx_namespaces,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([namespace Outer { namespace Inner { int i = 0; }}],
                [using namespace Outer::Inner; return i;],
 ac_cv_cxx_namespaces=yes, ac_cv_cxx_namespaces=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_namespaces" = yes; then
  AC_DEFINE(HAVE_NAMESPACES,,[define if the compiler
implements namespaces])
fi
])

