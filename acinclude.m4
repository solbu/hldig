dnl
dnl Part of the ht://Dig package   <http://www.htdig.org/>
dnl Copyright (c) 1999, 2000 The ht://Dig Group
dnl For copyright details, see the file COPYING in your distribution
dnl or the GNU General Public License version 2 or later
dnl <http://www.gnu.org/copyleft/gpl.html>
dnl
dnl Local autoconf definitions. Try to follow the guidelines of the autoconf
dnl macro repository so that integration in the repository is easy.
dnl To submit a macro to the repository send the macro (one macro per mail)
dnl to Peter Simons <simons@cys.de>.
dnl The repository itself is at http://peti.cys.de/autoconf-archive/
dnl

dnl Process user-specified options.
AC_DEFUN(AM_OPTIONS_SET, [

AC_MSG_CHECKING(if --disable-bigfile option specified)
AC_ARG_ENABLE(bigfile,
	[  --disable-bigfile       Disable Linux, AIX, HP/UX, Solaris big files.],
	[db_cv_bigfile="$enable_bigfile"], [db_cv_bigfile="no"])
AC_MSG_RESULT($db_cv_bigfile)

AC_MSG_CHECKING(if --enable-debug option specified)
AC_ARG_ENABLE(debug,
	[  --enable-debug          Build a debugging version.],
	[db_cv_debug="$enable_debug"], [db_cv_debug="no"])
AC_MSG_RESULT($db_cv_debug)

AC_MSG_CHECKING(if --enable-debug_rop option specified)
AC_ARG_ENABLE(debug_rop,
	[  --enable-debug_rop      Build a version that logs read operations.],
	[db_cv_debug_rop="$enable_debug_rop"], [db_cv_debug_rop="no"])
AC_MSG_RESULT($db_cv_debug_rop)

AC_MSG_CHECKING(if --enable-debug_wop option specified)
AC_ARG_ENABLE(debug_wop,
	[  --enable-debug_wop      Build a version that logs write operations.],
	[db_cv_debug_wop="$enable_debug_wop"], [db_cv_debug_wop="no"])
AC_MSG_RESULT($db_cv_debug_wop)

AC_MSG_CHECKING(if --enable-diagnostic option specified)
AC_ARG_ENABLE(diagnostic,
	[  --enable-diagnostic     Build a version with run-time diagnostics.],
	[db_cv_diagnostic="$enable_diagnostic"], [db_cv_diagnostic="no"])
AC_MSG_RESULT($db_cv_diagnostic)

AC_MSG_CHECKING(if --enable-posixmutexes option specified)
AC_ARG_ENABLE(posixmutexes,
	[  --enable-posixmutexes   Use POSIX standard mutexes.],
	[db_cv_posixmutexes="$enable_posixmutexes"], [db_cv_posixmutexes="no"])
AC_MSG_RESULT($db_cv_posixmutexes)

AC_MSG_CHECKING(if --enable-uimutexes option specified)
AC_ARG_ENABLE(uimutexes,
	[  --enable-uimutexes      Use Unix International mutexes.],
	[db_cv_uimutexes="$enable_uimutexes"], [db_cv_uimutexes="no"])
AC_MSG_RESULT($db_cv_uimutexes)

])dnl

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
dnl @version $Id: acinclude.m4,v 1.7.2.7 2000/09/21 04:38:28 ghutchis Exp $
dnl @author Loic Dachary <loic@senga.org>
dnl

AC_DEFUN(CHECK_USER,
[AC_MSG_CHECKING(user name)
test -n "$LOGNAME" && USER=$LOGNAME
AC_SUBST(USER)
AC_MSG_RESULT($USER)
])

dnl
dnl Prevent accidental use of Run Time Type Information g++ builtin
dnl functions.
dnl
AC_DEFUN(NO_RTTI,
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
AC_DEFUN(NO_EXCEPTIONS,
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

dnl @synopsis AC_MANDATORY_HEADER(HEADER-FILE, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl
dnl Same semantic as AC_CHECK_HEADER except that it aborts the configuration
dnl script if the header file is not found.
dnl
dnl @version $Id: acinclude.m4,v 1.7.2.7 2000/09/21 04:38:28 ghutchis Exp $
dnl @author Loic Dachary <loic@senga.org>
dnl
AC_DEFUN([AC_MANDATORY_HEADER],
[dnl Do the transliteration at runtime so arg 1 can be a shell variable.
ac_safe=`echo "$1" | sed 'y%./+-%__p_%'`
AC_MSG_CHECKING([for $1])
AC_CACHE_VAL(ac_cv_header_$ac_safe,
[AC_TRY_CPP([#include <$1>], eval "ac_cv_header_$ac_safe=yes",
  eval "ac_cv_header_$ac_safe=no")])dnl
if eval "test \"`echo '$ac_cv_header_'$ac_safe`\" = yes"; then
  AC_MSG_RESULT(yes)
  ifelse([$2], , :, [$2])
else
  AC_MSG_ERROR(header not found check config.log)
ifelse([$3], , , [$3
])dnl
fi
])

dnl @synopsis AC_MANDATORY_HEADERS(HEADER-FILE... [, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl
dnl Same semantic as AC_CHECK_HEADERS except that it aborts the configuration
dnl script if one of the headers file is not found.
dnl
dnl @version $Id: acinclude.m4,v 1.7.2.7 2000/09/21 04:38:28 ghutchis Exp $
dnl @author Loic Dachary <loic@senga.org>
dnl 
AC_DEFUN([AC_MANDATORY_HEADERS],
[for ac_hdr in $1
do
AC_MANDATORY_HEADER($ac_hdr,
[changequote(, )dnl
  ac_tr_hdr=HAVE_`echo $ac_hdr | sed 'y%abcdefghijklmnopqrstuvwxyz./-%ABCDEFGHIJKLMNOPQRSTUVWXYZ___%'`
changequote([, ])dnl
  AC_DEFINE_UNQUOTED($ac_tr_hdr) $2], $3)dnl
done
])

dnl @synopsis AC_MANDATORY_LIB(LIBRARY, FUNCTION [, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, OTHER-LIBRARIES]]])
dnl
dnl Same semantic as AC_CHECK_LIB except that it aborts the configuration
dnl script if the library is not found or compilation fails.
dnl
dnl @version $Id: acinclude.m4,v 1.7.2.7 2000/09/21 04:38:28 ghutchis Exp $
dnl @author Loic Dachary <loic@senga.org>
dnl
AC_DEFUN([AC_MANDATORY_LIB],
[AC_MSG_CHECKING([for $2 in -l$1])
dnl Use a cache variable name containing both the library and function name,
dnl because the test really is for library $1 defining function $2, not
dnl just for library $1.  Separate tests with the same $1 and different $2s
dnl may have different results.
ac_lib_var=`echo $1['_']$2 | sed 'y%./+-%__p_%'`
AC_CACHE_VAL(ac_cv_lib_$ac_lib_var,
[ac_save_LIBS="$LIBS"
LIBS="-l$1 $5 $LIBS"
AC_TRY_LINK(dnl
ifelse(AC_LANG, [FORTRAN77], ,
ifelse([$2], [main], , dnl Avoid conflicting decl of main.
[/* Override any gcc2 internal prototype to avoid an error.  */
]ifelse(AC_LANG, CPLUSPLUS, [#ifdef __cplusplus
extern "C"
#endif
])dnl
[/* We use char because int might match the return type of a gcc2
    builtin and then its argument prototype would still apply.  */
char $2();
])),
	    [$2()],
	    eval "ac_cv_lib_$ac_lib_var=yes",
	    eval "ac_cv_lib_$ac_lib_var=no")
LIBS="$ac_save_LIBS"
])dnl
if eval "test \"`echo '$ac_cv_lib_'$ac_lib_var`\" = yes"; then
  AC_MSG_RESULT(yes)
  ifelse([$3], ,
[changequote(, )dnl
  ac_tr_lib=HAVE_LIB`echo $1 | sed -e 's/[^a-zA-Z0-9_]/_/g' \
    -e 'y/abcdefghijklmnopqrstuvwxyz/ABCDEFGHIJKLMNOPQRSTUVWXYZ/'`
changequote([, ])dnl
  AC_DEFINE_UNQUOTED($ac_tr_lib)
  LIBS="-l$1 $LIBS"
], [$3])
else
  AC_MSG_ERROR(library not found check config.log)
ifelse([$4], , , [$4
])dnl
fi
])

dnl @synopsis AC_COMPILE_WARNINGS
dnl
dnl Set the maximum warning verbosity according to compiler used.
dnl Currently supports g++ and gcc.
dnl This macro must be put after AC_PROG_CC and AC_PROG_CXX in
dnl configure.in
dnl
dnl @version $Id: acinclude.m4,v 1.7.2.7 2000/09/21 04:38:28 ghutchis Exp $
dnl @author Loic Dachary <loic@senga.org>
dnl

AC_DEFUN(AC_COMPILE_WARNINGS,
[AC_MSG_CHECKING(maximum warning verbosity option)
if test -n "$CXX"
then
  if test "$GXX" = "yes"
  then
    ac_compile_warnings_opt='-Wall -Woverloaded-virtual'
  fi
  CXXFLAGS="$CXXFLAGS $ac_compile_warnings_opt"
  ac_compile_warnings_msg="$ac_compile_warnings_opt for C++"
fi

if test -n "$CC"
then
  if test "$GCC" = "yes"
  then
    ac_compile_warnings_opt='-Wall -Wmissing-declarations -Wmissing-prototypes'
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
dnl @version $Id: acinclude.m4,v 1.7.2.7 2000/09/21 04:38:28 ghutchis Exp $
dnl @author Loic Dachary <loic@senga.org>
dnl
AC_DEFUN([CHECK_ZLIB],
#
# Handle user hints
#
[AC_MSG_CHECKING(if zlib is wanted)
AC_ARG_WITH(zlib,
[  --with-zlib=DIR         root directory path of zlib installation [defaults to
		          /usr/local or /usr if not found in /usr/local]
  --without-zlib          to disable zlib usage completely],
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
	#
	# Adding /usr/lib or /usr/include to the flags/libs may
	# hurt if using a compiler not installed in the standard 
	# place.
	#
        if test "${ZLIB_HOME}" != "/usr"
	then
		LDFLAGS="$LDFLAGS -L${ZLIB_HOME}/lib"
		CPPFLAGS="$CPPFLAGS -I${ZLIB_HOME}/include"
	fi
        AC_LANG_SAVE
        AC_LANG_C
	AC_MANDATORY_LIB(z, inflateEnd)
        AC_MANDATORY_HEADERS(zlib.h)
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
dnl @version $Id: acinclude.m4,v 1.7.2.7 2000/09/21 04:38:28 ghutchis Exp $
dnl @author Loic Dachary <loic@senga.org>
dnl

AC_DEFUN(AC_PROG_APACHE,
#
# Handle user hints
#
[
 AC_MSG_CHECKING(if apache is wanted)
 AC_ARG_WITH(apache,
  [  --with-apache=PATH absolute path name of apache server (default is to search httpd in
    /usr/local/apache/bin:/usr/local/bin:/usr/local/sbin:/usr/bin:/usr/sbin),
    --without-apache to disable apache detection],
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
  if test $APACHE_WANTED = yes ; then
    #
    # If not specified by caller, search in standard places
    #
    if test -z "$APACHE" ; then
      AC_PATH_PROG(APACHE, httpd, , /usr/local/apache/bin:/usr/local/bin:/usr/local/sbin:/usr/bin:/usr/sbin)
    fi
    AC_SUBST(APACHE)
    if test -z "$APACHE" ; then
	AC_MSG_ERROR("apache server executable not found");
    fi
    #
    # Collect apache version number. If for nothing else, this
    # guaranties that httpd is a working apache executable.
    #
    changequote(<<, >>)dnl
    APACHE_READABLE_VERSION=`$APACHE -v | grep 'Server version' | sed -e 's;.*/\([0-9\.][0-9\.]*\).*;\1;'`
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
    for dir in libexec modules lib/apache
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

AC_DEFUN(AM_PROG_TIME, [
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

dnl Figure out mutexes for this compiler/architecture.
AC_DEFUN(AM_DEFINE_MUTEXES, [

AC_CACHE_CHECK([for mutexes], db_cv_mutex, [dnl
db_cv_mutex=no

orig_libs=$LIBS

dnl User-specified POSIX mutexes.
dnl
dnl Assume that -lpthread exists when the user specifies POSIX mutexes.  (I
dnl only expect this option to be used on Solaris, which has -lpthread.)
if test "$db_cv_posixmutexes" = yes; then
	db_cv_mutex="POSIX/pthreads/library"
fi

dnl User-specified UI mutexes.
dnl
dnl Assume that -lthread exists when the user specifies UI mutexes.  (I only
dnl expect this option to be used on Solaris, which has -lthread.)
if test "$db_cv_uimutexes" = yes; then
	db_cv_mutex="UI/threads/library"
fi

dnl LWP threads: _lwp_XXX
dnl
dnl Test for LWP threads before testing for UI/POSIX threads, we prefer them
dnl on Solaris.  There are two reasons: the Solaris C library has UI/POSIX
dnl interface stubs, but they're broken, configuring them for inter-process
dnl mutexes doesn't return an error, but it doesn't work either.  Second,
dnl there's a bug in SunOS 5.7 where applications get pwrite, not pwrite64,
dnl if they load the C library before the appropriate threads library, e.g.,
dnl tclsh using dlopen to load the DB library.  Anyway, by using LWP threads
dnl we avoid answering lots of user questions, not to mention the bugs.
if test "$db_cv_mutex" = no; then
AC_TRY_RUN([
#include <synch.h>
main(){
	static lwp_mutex_t mi = SHAREDMUTEX;
	static lwp_cond_t ci = SHAREDCV;
	lwp_mutex_t mutex = mi;
	lwp_cond_t cond = ci;
	exit (
	_lwp_mutex_lock(&mutex) ||
	_lwp_mutex_unlock(&mutex));
}], [db_cv_mutex="Solaris/lwp"])
fi

dnl UI threads: thr_XXX
dnl
dnl Try with and without the -lthread library.
if test "$db_cv_mutex" = no; then
LIBS="-lthread $LIBS"
AC_TRY_RUN([
#include <thread.h>
#include <synch.h>
main(){
	mutex_t mutex;
	cond_t cond;
	int type = USYNC_PROCESS;
	exit (
	mutex_init(&mutex, type, NULL) ||
	cond_init(&cond, type, NULL) ||
	mutex_lock(&mutex) ||
	mutex_unlock(&mutex));
}], [db_cv_mutex="UI/threads/library"])
LIBS="$orig_libs"
fi
if test "$db_cv_mutex" = no; then
AC_TRY_RUN([
#include <thread.h>
#include <synch.h>
main(){
	mutex_t mutex;
	cond_t cond;
	int type = USYNC_PROCESS;
	exit (
	mutex_init(&mutex, type, NULL) ||
	cond_init(&cond, type, NULL) ||
	mutex_lock(&mutex) ||
	mutex_unlock(&mutex));
}], [db_cv_mutex="UI/threads"])
fi

dnl POSIX.1 pthreads: pthread_XXX
dnl
dnl Try with and without the -lpthread library.
if test "$db_cv_mutex" = no; then
AC_TRY_RUN([
#include <pthread.h>
main(){
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	pthread_condattr_t condattr;
	pthread_mutexattr_t mutexattr;
	exit (
	pthread_condattr_init(&condattr) ||
	pthread_condattr_setpshared(&condattr, PTHREAD_PROCESS_SHARED) ||
	pthread_mutexattr_init(&mutexattr) ||
	pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED) ||
	pthread_cond_init(&cond, &condattr) ||
	pthread_mutex_init(&mutex, &mutexattr) ||
	pthread_mutex_lock(&mutex) ||
	pthread_mutex_unlock(&mutex) ||
	pthread_mutex_destroy(&mutex) ||
	pthread_cond_destroy(&cond) ||
	pthread_condattr_destroy(&condattr) ||
	pthread_mutexattr_destroy(&mutexattr));
}], [db_cv_mutex="POSIX/pthreads"])
fi
if test "$db_cv_mutex" = no; then
LIBS="-lpthread $LIBS"
AC_TRY_RUN([
#include <pthread.h>
main(){
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	pthread_condattr_t condattr;
	pthread_mutexattr_t mutexattr;
	exit (
	pthread_condattr_init(&condattr) ||
	pthread_condattr_setpshared(&condattr, PTHREAD_PROCESS_SHARED) ||
	pthread_mutexattr_init(&mutexattr) ||
	pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED) ||
	pthread_cond_init(&cond, &condattr) ||
	pthread_mutex_init(&mutex, &mutexattr) ||
	pthread_mutex_lock(&mutex) ||
	pthread_mutex_unlock(&mutex) ||
	pthread_mutex_destroy(&mutex) ||
	pthread_cond_destroy(&cond) ||
	pthread_condattr_destroy(&condattr) ||
	pthread_mutexattr_destroy(&mutexattr));
}], [db_cv_mutex="POSIX/pthreads/library"])
LIBS="$orig_libs"
fi

dnl msemaphore: HPPA only
dnl Try HPPA before general msem test, it needs special alignment.
if test "$db_cv_mutex" = no; then
AC_TRY_RUN([
#include <sys/mman.h>
main(){
#if defined(__hppa)
	typedef msemaphore tsl_t;
	msemaphore x;
	msem_init(&x, 0);
	msem_lock(&x, 0);
	msem_unlock(&x, 0);
	exit(0);
#else
	exit(1);
#endif
}], [db_cv_mutex="HP/msem_init"])
fi

dnl msemaphore: AIX, OSF/1
if test "$db_cv_mutex" = no; then
AC_TRY_RUN([
#include <sys/types.h>
#include <sys/mman.h>;
main(){
	typedef msemaphore tsl_t;
	msemaphore x;
	msem_init(&x, 0);
	msem_lock(&x, 0);
	msem_unlock(&x, 0);
	exit(0);
}], [db_cv_mutex="UNIX/msem_init"])
fi

dnl ReliantUNIX
if test "$db_cv_mutex" = no; then
LIBS="$LIBS -lmproc"
AC_TRY_LINK([#include <ulocks.h>],
[typedef spinlock_t tsl_t;
spinlock_t x; initspin(&x, 1); cspinlock(&x); spinunlock(&x);],
[db_cv_mutex="ReliantUNIX/initspin"])
LIBS="$orig_libs"
fi

dnl SCO: UnixWare has threads in libthread, but OpenServer doesn't.
if test "$db_cv_mutex" = no; then
AC_TRY_RUN([
main(){
#if defined(__USLC__)
	exit(0);
#endif
	exit(1);
}], [db_cv_mutex="SCO/x86/cc-assembly"])
fi

dnl abilock_t: SGI
if test "$db_cv_mutex" = no; then
AC_TRY_LINK([#include <abi_mutex.h>],
[typedef abilock_t tsl_t;
abilock_t x; init_lock(&x); acquire_lock(&x); release_lock(&x);],
[db_cv_mutex="SGI/init_lock"])
fi

dnl sema_t: Solaris
dnl The sema_XXX calls do not work on Solaris 5.5.  I see no reason to ever
dnl turn this test on, unless we find some other platform that uses the old
dnl POSIX.1 interfaces.  (I plan to move directly to pthreads on Solaris.)
if test "$db_cv_mutex" = DOESNT_WORK; then
AC_TRY_LINK([#include <synch.h>],
[typedef sema_t tsl_t;
 sema_t x;
 sema_init(&x, 1, USYNC_PROCESS, NULL); sema_wait(&x); sema_post(&x);],
[db_cv_mutex="UNIX/sema_init"])
fi

dnl _lock_try/_lock_clear: Solaris
dnl On Solaris systems without Pthread or UI mutex interfaces, DB uses the
dnl undocumented _lock_try _lock_clear function calls instead of either the
dnl sema_trywait(3T) or sema_wait(3T) function calls.  This is because of
dnl problems in those interfaces in some releases of the Solaris C library.
if test "$db_cv_mutex" = no; then
AC_TRY_LINK([#include <sys/machlock.h>],
[typedef lock_t tsl_t;
 lock_t x;
 _lock_try(&x); _lock_clear(&x);],
[db_cv_mutex="Solaris/_lock_try"])
fi

dnl _check_lock/_clear_lock: AIX
if test "$db_cv_mutex" = no; then
AC_TRY_LINK([#include <sys/atomic_op.h>],
[int x; _check_lock(&x,0,1); _clear_lock(&x,0);],
[db_cv_mutex="AIX/_check_lock"])
fi

dnl Alpha/gcc: OSF/1
dnl The alpha/gcc code doesn't work as far as I know.  There are
dnl two versions, both have problems.  See Support Request #1583.
if test "$db_cv_mutex" = DOESNT_WORK; then
AC_TRY_RUN([main(){
#if defined(__alpha)
#if defined(__GNUC__)
exit(0);
#endif
#endif
exit(1);}],
[db_cv_mutex="ALPHA/gcc-assembly"])
fi

dnl PaRisc/gcc: HP/UX
if test "$db_cv_mutex" = no; then
AC_TRY_RUN([main(){
#if defined(__hppa)
#if defined(__GNUC__)
exit(0);
#endif
#endif
exit(1);}],
[db_cv_mutex="HPPA/gcc-assembly"])
fi

dnl Sparc/gcc: SunOS, Solaris
dnl The sparc/gcc code doesn't always work, specifically, I've seen assembler
dnl failures from the stbar instruction on SunOS 4.1.4/sun4c and gcc 2.7.2.2.
if test "$db_cv_mutex" = DOESNT_WORK; then
AC_TRY_RUN([main(){
#if defined(__sparc__)
#if defined(__GNUC__)
	exit(0);
#endif
#endif
	exit(1);
}], [db_cv_mutex="Sparc/gcc-assembly"])
fi

dnl 68K/gcc: SunOS
if test "$db_cv_mutex" = no; then
AC_TRY_RUN([main(){
#if (defined(mc68020) || defined(sun3))
#if defined(__GNUC__)
	exit(0);
#endif
#endif
	exit(1);
}], [db_cv_mutex="68K/gcc-assembly"])
fi

dnl x86/gcc: FreeBSD, NetBSD, BSD/OS, Linux
if test "$db_cv_mutex" = no; then
AC_TRY_RUN([main(){
#if defined(i386)
#if defined(__GNUC__)
	exit(0);
#endif
#endif
	exit(1);
}], [db_cv_mutex="x86/gcc-assembly"])
fi

dnl ia86/gcc: Linux
if test "$db_cv_mutex" = no; then
AC_TRY_RUN([main(){
#if defined(__ia64)
#if defined(__GNUC__)
	exit(0);
#endif
#endif
	exit(1);
}], [db_cv_mutex="ia64/gcc-assembly"])
fi

dnl: uts/cc: UTS
if test "$db_cv_mutex" = no; then
AC_TRY_RUN([main(){
#if defined(_UTS)
	exit(0);
#endif
	exit(1);
}], [db_cv_mutex="UTS/cc-assembly"])
fi
])

if test "$db_cv_mutex" = no; then
	AC_MSG_WARN(
	    [THREAD MUTEXES NOT AVAILABLE FOR THIS COMPILER/ARCHITECTURE.])
	AC_DEFINE(HAVE_MUTEX_FCNTL)
else
	AC_DEFINE(HAVE_MUTEX_THREADS)
fi

case "$db_cv_mutex" in
68K/gcc-assembly)	AC_DEFINE(HAVE_MUTEX_TAS)
			AC_DEFINE(HAVE_MUTEX_68K_GCC_ASSEMBLY);;
AIX/_check_lock)	AC_DEFINE(HAVE_MUTEX_TAS)
			AC_DEFINE(HAVE_MUTEX_AIX_CHECK_LOCK);;
ALPHA/gcc-assembly)	AC_DEFINE(HAVE_MUTEX_TAS)
			AC_DEFINE(HAVE_MUTEX_ALPHA_GCC_ASSEMBLY);;
HP/msem_init)		AC_DEFINE(HAVE_MUTEX_TAS)
			AC_DEFINE(HAVE_MUTEX_HPPA_MSEM_INIT);;
HPPA/gcc-assembly)	AC_DEFINE(HAVE_MUTEX_TAS)
			AC_DEFINE(HAVE_MUTEX_HPPA_GCC_ASSEMBLY);;
ia64/gcc-assembly)	AC_DEFINE(HAVE_MUTEX_TAS)
			AC_DEFINE(HAVE_MUTEX_IA64_GCC_ASSEMBLY);;
POSIX/pthreads)		AC_DEFINE(HAVE_MUTEX_PTHREAD)
			AC_DEFINE(HAVE_MUTEX_PTHREADS);;
POSIX/pthreads/library)	LIBS="-lpthread $LIBS"
			AC_DEFINE(HAVE_MUTEX_PTHREAD)
			AC_DEFINE(HAVE_MUTEX_PTHREADS);;
ReliantUNIX/initspin)	LIBS="$LIBS -lmproc"
			AC_DEFINE(HAVE_MUTEX_TAS)
			AC_DEFINE(HAVE_MUTEX_RELIANTUNIX_INITSPIN);;
SCO/x86/cc-assembly)	AC_DEFINE(HAVE_MUTEX_TAS)
			AC_DEFINE(HAVE_MUTEX_SCO_X86_CC_ASSEMBLY);;
SGI/init_lock)		AC_DEFINE(HAVE_MUTEX_TAS)
			AC_DEFINE(HAVE_MUTEX_SGI_INIT_LOCK);;
Solaris/_lock_try)	AC_DEFINE(HAVE_MUTEX_TAS)
			AC_DEFINE(HAVE_MUTEX_SOLARIS_LOCK_TRY);;
Solaris/lwp)		AC_DEFINE(HAVE_MUTEX_PTHREAD)
			AC_DEFINE(HAVE_MUTEX_SOLARIS_LWP);;
Sparc/gcc-assembly)	AC_DEFINE(HAVE_MUTEX_TAS)
			AC_DEFINE(HAVE_MUTEX_SPARC_GCC_ASSEMBLY);;
UI/threads)		AC_DEFINE(HAVE_MUTEX_PTHREAD)
			AC_DEFINE(HAVE_MUTEX_UI_THREADS);;
UI/threads/library)	LIBS="-lthread $LIBS"
			AC_DEFINE(HAVE_MUTEX_PTHREAD)
			AC_DEFINE(HAVE_MUTEX_UI_THREADS);;
UNIX/msem_init)		AC_DEFINE(HAVE_MUTEX_TAS)
			AC_DEFINE(HAVE_MUTEX_MSEM_INIT);;
UNIX/sema_init)		AC_DEFINE(HAVE_MUTEX_TAS)
			AC_DEFINE(HAVE_MUTEX_SEMA_INIT);;
UTS/cc-assembly)	ADDITIONAL_OBJS="$ADDITIONAL_OBJS uts4.cc${o}"
			AC_DEFINE(HAVE_MUTEX_UTS_CC_ASSEMBLY);;
x86/gcc-assembly)	AC_DEFINE(HAVE_MUTEX_TAS)
			AC_DEFINE(HAVE_MUTEX_X86_GCC_ASSEMBLY);;
esac
])dnl

dnl Check for the standard shorthand types.
AC_DEFUN(AM_SHORTHAND_TYPES, [dnl

AC_SUBST(ssize_t_decl)
AC_CACHE_CHECK([for ssize_t], db_cv_ssize_t, [dnl
AC_TRY_COMPILE([#include <sys/types.h>], ssize_t foo;,
	[db_cv_ssize_t=yes], [db_cv_ssize_t=no])])
if test "$db_cv_ssize_t" = no; then
	ssize_t_decl="typedef int ssize_t;"
fi

AC_SUBST(u_char_decl)
AC_CACHE_CHECK([for u_char], db_cv_uchar, [dnl
AC_TRY_COMPILE([#include <sys/types.h>], u_char foo;,
	[db_cv_uchar=yes], [db_cv_uchar=no])])
if test "$db_cv_uchar" = no; then
	u_char_decl="typedef unsigned char u_char;"
fi

AC_SUBST(u_short_decl)
AC_CACHE_CHECK([for u_short], db_cv_ushort, [dnl
AC_TRY_COMPILE([#include <sys/types.h>], u_short foo;,
	[db_cv_ushort=yes], [db_cv_ushort=no])])
if test "$db_cv_ushort" = no; then
	u_short_decl="typedef unsigned short u_short;"
fi

AC_SUBST(u_int_decl)
AC_CACHE_CHECK([for u_int], db_cv_uint, [dnl
AC_TRY_COMPILE([#include <sys/types.h>], u_int foo;,
	[db_cv_uint=yes], [db_cv_uint=no])])
if test "$db_cv_uint" = no; then
	u_int_decl="typedef unsigned int u_int;"
fi

AC_SUBST(u_long_decl)
AC_CACHE_CHECK([for u_long], db_cv_ulong, [dnl
AC_TRY_COMPILE([#include <sys/types.h>], u_long foo;,
	[db_cv_ulong=yes], [db_cv_ulong=no])])
if test "$db_cv_ulong" = no; then
	u_long_decl="typedef unsigned long u_long;"
fi

dnl DB/Vi use specific integer sizes.
AC_SUBST(u_int8_decl)
AC_CACHE_CHECK([for u_int8_t], db_cv_uint8, [dnl
AC_TRY_COMPILE([#include <sys/types.h>], u_int8_t foo;,
	[db_cv_uint8=yes],
	AC_TRY_RUN([main(){exit(sizeof(unsigned char) != 1);}],
	    [db_cv_uint8="unsigned char"], [db_cv_uint8=no]))])
if test "$db_cv_uint8" = no; then
	AC_MSG_ERROR(No unsigned 8-bit integral type.)
fi
if test "$db_cv_uint8" != yes; then
	u_int8_decl="typedef $db_cv_uint8 u_int8_t;"
fi

AC_SUBST(u_int16_decl)
AC_CACHE_CHECK([for u_int16_t], db_cv_uint16, [dnl
AC_TRY_COMPILE([#include <sys/types.h>], u_int16_t foo;,
	[db_cv_uint16=yes],
AC_TRY_RUN([main(){exit(sizeof(unsigned short) != 2);}],
	[db_cv_uint16="unsigned short"],
AC_TRY_RUN([main(){exit(sizeof(unsigned int) != 2);}],
	[db_cv_uint16="unsigned int"], [db_cv_uint16=no])))])
if test "$db_cv_uint16" = no; then
	AC_MSG_ERROR([No unsigned 16-bit integral type.])
fi
if test "$db_cv_uint16" != yes; then
	u_int16_decl="typedef $db_cv_uint16 u_int16_t;"
fi

AC_SUBST(int16_decl)
AC_CACHE_CHECK([for int16_t], db_cv_int16, [dnl
AC_TRY_COMPILE([#include <sys/types.h>], int16_t foo;,
	[db_cv_int16=yes],
AC_TRY_RUN([main(){exit(sizeof(short) != 2);}],
	[db_cv_int16="short"],
AC_TRY_RUN([main(){exit(sizeof(int) != 2);}],
	[db_cv_int16="int"], [db_cv_int16=no])))])
if test "$db_cv_int16" = no; then
	AC_MSG_ERROR([No signed 16-bit integral type.])
fi
if test "$db_cv_int16" != yes; then
	int16_decl="typedef $db_cv_int16 int16_t;"
fi

AC_SUBST(u_int32_decl)
AC_CACHE_CHECK([for u_int32_t], db_cv_uint32, [dnl
AC_TRY_COMPILE([#include <sys/types.h>], u_int32_t foo;,
	[db_cv_uint32=yes],
AC_TRY_RUN([main(){exit(sizeof(unsigned int) != 4);}],
	[db_cv_uint32="unsigned int"],
AC_TRY_RUN([main(){exit(sizeof(unsigned long) != 4);}],
	[db_cv_uint32="unsigned long"], [db_cv_uint32=no])))])
if test "$db_cv_uint32" = no; then
	AC_MSG_ERROR([No unsigned 32-bit integral type.])
fi
if test "$db_cv_uint32" != yes; then
	u_int32_decl="typedef $db_cv_uint32 u_int32_t;"
fi

AC_SUBST(int32_decl)
AC_CACHE_CHECK([for int32_t], db_cv_int32, [dnl
AC_TRY_COMPILE([#include <sys/types.h>], int32_t foo;,
	[db_cv_int32=yes],
AC_TRY_RUN([main(){exit(sizeof(int) != 4);}],
	[db_cv_int32="int"],
AC_TRY_RUN([main(){exit(sizeof(long) != 4);}],
	[db_cv_int32="long"], [db_cv_int32=no])))])
if test "$db_cv_int32" = no; then
	AC_MSG_ERROR([No signed 32-bit integral type.])
fi
if test "$db_cv_int32" != yes; then
	int32_decl="typedef $db_cv_int32 int32_t;"
fi

dnl Figure out largest integral type.
AC_SUBST(db_align_t_decl)
AC_CACHE_CHECK([for largest integral type], db_cv_align_t, [dnl
AC_TRY_COMPILE([#include <sys/types.h>], long long foo;,
	[db_cv_align_t="unsigned long long"], [db_cv_align_t="unsigned long"])])
db_align_t_decl="typedef $db_cv_align_t db_align_t;"

dnl Figure out integral type the same size as a pointer.
AC_SUBST(db_alignp_t_decl)
AC_CACHE_CHECK([for integral type equal to pointer size], db_cv_alignp_t, [dnl
db_cv_alignp_t=$db_cv_align_t
AC_TRY_RUN([main(){exit(sizeof(unsigned int) != sizeof(char *));}],
	[db_cv_alignp_t="unsigned int"])
AC_TRY_RUN([main(){exit(sizeof(unsigned long) != sizeof(char *));}],
	[db_cv_alignp_t="unsigned long"])
AC_TRY_RUN([main(){exit(sizeof(unsigned long long) != sizeof(char *));}],
	[db_cv_alignp_t="unsigned long long"])])
db_alignp_t_decl="typedef $db_cv_alignp_t db_alignp_t;"

])dnl

