#
# Part of the hl://Dig package   <https://andy5995.github.io/hldig/>
# Copyright (c) 1999-2017 The hl://Dig Group
# For copyright details, see the file COPYING in your distribution
# or the GNU Library General Public License (LGPL) version 2 or later
# <http://www.gnu.org/copyleft/lgpl.html>

AC_PREREQ(2.57)
#AC_INIT

#
# hl://Dig's version
#
VERSION=`cat ${srcdir}/.version`
HTDIG_MAJOR_VERSION=[`expr $VERSION : '\([0-9][0-9]*\)'`]
AC_SUBST(HTDIG_MAJOR_VERSION)
HTDIG_MINOR_VERSION=[`expr $VERSION : '[0-9][0-9]*\.\([0-9][0-9]*\)'`]
AC_SUBST(HTDIG_MINOR_VERSION)
HTDIG_MICRO_VERSION=[`expr $VERSION : '[0-9][0-9]*\.[0-9][0-9]*\.\([0-9][0-9]*\)'`]
AC_SUBST(HTDIG_MICRO_VERSION)

AC_INIT([htdig], 0.1.01)
AC_CONFIG_SRCDIR([htdig/htdig.cc])
AM_INIT_AUTOMAKE

AM_CONFIG_HEADER([include/config.h:include/config.in])
AC_CONFIG_LIBOBJ_DIR([htlib])
AC_PREFIX_DEFAULT([/opt/www])

AC_CONFIG_MACRO_DIRS([m4])

# Initialize maintainer mode
AM_MAINTAINER_MODE

AC_ARG_WITH(config-dir, [AC_HELP_STRING([--with-config-dir=DIR],
	[where your config directory is @<:@PREFIX/conf@:>@])],
 CONFIG_DIR="$withval", CONFIG_DIR='${prefix}/conf')
AC_SUBST(CONFIG_DIR)

# In the below, the strings @<:@ and @:>@ expand to [ and ]
AC_ARG_WITH(default-config-file,
	[AC_HELP_STRING([--with-default-config-file=FILE],
	[where the various programs will look for a configuration file
	@<:@PREFIX/conf/htdig.conf@:>@])],
 DEFAULT_CONFIG_FILE="$withval", DEFAULT_CONFIG_FILE='${CONFIG_DIR}/htdig.conf')
AC_SUBST(DEFAULT_CONFIG_FILE)

AC_ARG_WITH(common-dir, [AC_HELP_STRING([--with-common-dir=DIR],
	[where your .html templates are stored
	@<:@PREFIX/share/htdig@:>@])],
 COMMON_DIR="$withval", COMMON_DIR='${datadir}/htdig')
AC_SUBST(COMMON_DIR)

AC_ARG_WITH(database-dir, [AC_HELP_STRING([--with-database-dir=DIR],
	[where your database directory is
	@<:@PREFIX/var/htdig@:>@])],
 DATABASE_DIR="$withval", DATABASE_DIR='${localstatedir}/htdig')
AC_SUBST(DATABASE_DIR)

AC_ARG_WITH(cgi-bin-dir, [AC_HELP_STRING([--with-cgi-bin-dir=DIR],
	[where your web server cgi-bin directory is
	@<:@PREFIX/cgi-bin@:>@])],
 CGIBIN_DIR="$withval", CGIBIN_DIR='${prefix}/cgi-bin')
AC_SUBST(CGIBIN_DIR)

AC_ARG_WITH(search-dir, [AC_HELP_STRING([--with-search-dir=DIR],
	[where the sample search form should be installed
	@<:@PREFIX/htdocs/htdig@:>@])],
 SEARCH_DIR="$withval", SEARCH_DIR='${prefix}/htdocs/htdig')
AC_SUBST(SEARCH_DIR)

AC_ARG_WITH(search-form, [AC_HELP_STRING([--with-search-form=FILE],
	[the name for the sample search form @<:@search.html@:>@])],
 SEARCH_FORM="$withval", SEARCH_FORM='search.html')
AC_SUBST(SEARCH_FORM)

AC_ARG_WITH(image-dir, [AC_HELP_STRING([--with-image-dir=DIR],
	[where the hl://Dig images are installed
	@<:@PREFIX/htdocs/htdig@:>@])],
 IMAGE_DIR="$withval", IMAGE_DIR='${prefix}/htdocs/htdig')
AC_SUBST(IMAGE_DIR)

AC_ARG_WITH(image-url-prefix,
	[AC_HELP_STRING([--with-image-url-prefix=LOCATION],
	[the URL path to the installed images @<:@/htdig@:>@])],
 IMAGE_URL_PREFIX="$withval", IMAGE_URL_PREFIX='/htdig')
AC_SUBST(IMAGE_URL_PREFIX)

AC_ARG_WITH(rx, [AC_HELP_STRING([--with-rx],
	[with system rx instead of regex @<:@no@:>@])],
 WITH_RX="$withval", WITH_RX='no')

# If the user doesn't want tests
AC_ARG_ENABLE(tests,
	[AC_HELP_STRING([--enable-tests],
		[do build a version with run-time tests.])],
	[htdig_cv_tests="$enable_tests"], [htdig_cv_tests="no"])
AM_CONDITIONAL(TESTS, test x$htdig_cv_tests = xyes)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_AIX
AM_PROG_LEX
AC_PROG_YACC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AC_COMPILE_WARNINGS
NO_RTTI
NO_EXCEPTIONS
AC_PATH_PROG(AR, ar, ar)
AC_PATH_PROG(SHELL, sh, /bin/sh)
AC_PATH_PROG(SED, sed, /bin/sed)
AC_PATH_PROG(PERL, perl, /usr/bin/perl)
AC_PATH_PROG(FIND, find, /bin/find)
AC_PATH_PROG(GUNZIP, gunzip, /bin/gunzip)
AC_CHECK_PROGS(RRDTOOL, rrdtool)
AC_CHECK_PROGS(TAR, tar gtar gnutar, tar)
AC_PATH_PROG(MV, mv, /bin/mv)
AC_PATH_PROG(SENDMAIL, sendmail, /usr/lib/sendmail,
   $PATH:/usr/libexec:/usr/sbin:/usr/lib:/usr/etc:etc)
AM_PROG_TIME


# Checks for libraries.
#AC_CHECK_LIB([z], [main])
AC_CHECK_LIB(socket, socket)
AC_CHECK_LIB(nsl, t_accept)
CHECK_SSL
CHECK_ZLIB
AC_SUBST(CXXFLAGS)
AC_SUBST(LDFLAGS)

# Checks for header files.
AC_HEADER_STDBOOL
AC_HEADER_TIME
AC_FUNC_ALLOCA
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h float.h langinfo.h libintl.h limits.h locale.h malloc.h netdb.h netinet/in.h stddef.h stdlib.h string.h strings.h sys/file.h sys/ioctl.h sys/socket.h sys/time.h sys/timeb.h syslog.h unistd.h wchar.h wctype.h])
#AC_CHECK_HEADERS([zlib.h sys/wait.h wait.h alloca.h])
AC_CHECK_HEADERS([sys/wait.h wait.h alloca.h])
#AC_CHECK_HEADERS(ssl.h)

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STAT
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_CHECK_MEMBERS([struct stat.st_blksize])
AC_HEADER_TIME
AC_STRUCT_TM
AC_CHECK_SIZEOF(unsigned long long int, 8)
AC_CHECK_SIZEOF(unsigned long int, 4)
AC_CHECK_SIZEOF(unsigned int, 4)
AC_CHECK_SIZEOF(unsigned short int, 2)
AC_CHECK_SIZEOF(unsigned char, 1)

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
#AC_FUNC_ERROR_AT_LINE
#AC_FUNC_LSTAT
#AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_MBRTOWC
AC_FUNC_MEMCMP
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
#AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_STRPTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([alarm btowc bzero ftime getcwd gettimeofday localtime_r memcmp memcpy memmove memset mkstemp munmap pstat_getdynamic raise re_comp regcomp select setlocale strchr strcspn strdup strerror strrchr strstr strtol strtoul timegm])
AC_REPLACE_FUNCS(snprintf vsnprintf)

# More header checks--here use C++
AC_LANG([C++])
AC_CXX_HAVE_STD

# Initialize gettext for translations
AM_GNU_GETTEXT([external], [need-formatstring-macros])
AM_GNU_GETTEXT_VERSION([0.19.8])

AC_MSG_CHECKING(whether we need gethostname() prototype?)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <sys/file.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdlib.h>
extern "C" int gethostname(char *, int);
]],[[
gethostname("sdsu.edu", (int) 8);
]])],[AC_MSG_RESULT(yes);AC_DEFINE([NEED_PROTO_GETHOSTNAME],,[Define if you need a prototype for gethostname()])],[AC_MSG_RESULT(no)])

# We're still using the C++ compiler for this test
AC_MSG_CHECKING(how to call getpeername?)
for sock_t in 'struct sockaddr' 'void'; do
  for getpeername_length_t in 'size_t' 'int' 'unsigned int' 'long unsigned int' 'socklen_t'
  do
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <sys/socket.h>
 extern "C" int getpeername(int, $sock_t *, $getpeername_length_t *);
 $sock_t s;  $getpeername_length_t l; ]], [[ getpeername(0, &s, &l); ]])],[ac_found=yes ; break 2],[ac_found=no])
  done
done

if test "$ac_found" = no
then
  AC_MSG_WARN([can't determine, using size_t])
  getpeername_length_t="size_t"
else
  AC_MSG_RESULT($getpeername_length_t)
fi
AC_DEFINE_UNQUOTED([GETPEERNAME_LENGTH_T],[$getpeername_length_t],[Define this to the type of the third argument of getpeername()])

AC_MSG_CHECKING(how to call select?)
for fd_set_t in 'fd_set' 'int'
  do
  for timeval_t in 'struct timeval' 'const struct timeval'
  do
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
 extern "C" int select(int, $fd_set_t *, $fd_set_t *, $fd_set_t *, $timeval_t *);
 $fd_set_t fd; ]], [[ select(0, &fd, 0, 0, 0); ]])],[ac_found=yes ; break 2],[ac_found=no])
 done
done

if test "$ac_found" = no
then
  AC_MSG_WARN([can't determine argument type using int])
  fd_set_t="int"
else
  AC_MSG_RESULT($fd_set_t)
fi
AC_DEFINE_UNQUOTED([FD_SET_T],[$fd_set_t],[Define this to the type of the second argument of select()])

# Replacements for the library functions (i.e. regex)
# Regex is compiled by the C compiler so we need to do it before we switch
if test $WITH_RX = yes; then
 AC_CHECK_HEADER(rxposix.h, AC_DEFINE(USE_RX,,[Use posix regexp]), WITH_RX='no')
fi
if test $WITH_RX = no; then
 AC_MSG_CHECKING(if we should use the included regex?)
 OLD_CFLAGS=$CFLAGS
 CFLAGS="$CFLAGS -I${srcdir}/htlib"

AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include "${srcdir}/htlib/regex.c"
int main() {
    regex_t re;
    return regcomp(&re, "ht.*Dig", REG_ICASE);
}
]])],
   [AC_MSG_RESULT(yes);AC_LIBOBJ([regex])],
   [AC_MSG_RESULT(no);AC_DEFINE(HAVE_BROKEN_REGEX,,[Broken regexp])],
   [AC_MSG_RESULT(unknown);AC_DEFINE(HAVE_BROKEN_REGEX,,[Unknown regexp])])
 CFLAGS=$OLD_CFLAGS
fi

# Enable big files?
AC_MSG_CHECKING(if --enable-bigfile option specified)
AC_ARG_ENABLE(bigfile,
	[AC_HELP_STRING([--enable-bigfile], [enable Linux, AIX, HP/UX, Solaris big files.])],
	[db_cv_bigfile="$enable_bigfile"], [db_cv_bigfile="yes"])
AC_MSG_RESULT($db_cv_bigfile)

# Vendors are doing 64-bit lseek in different ways.
# Linux, AIX, HP/UX and Solaris all use _FILE_OFFSET_BITS to specify a "big-file"
# environment.
if test "$db_cv_bigfile" = yes; then
	case "$host_os" in
	bsdi*|aix*|hpux*|solaris*)	AC_DEFINE(HAVE_FILE_OFFSET_BITS,,['big-file' environment]);;
	linux*)	AC_DEFINE(HAVE_FILE_OFFSET_BITS,,['big-file' environment])
		AC_DEFINE(HAVE_LARGEFILE_SOURCE,,[large file sources])
		;;
	esac
fi

# Add the -mimpure-text option on Solaris with GCC and libstc++ that is not shared
if test "$GXX" = "yes"
then
  case "$host" in
    *-sun-solaris* ) extra_ldflags="-mimpure-text";;
  esac
fi

AC_SUBST(extra_ldflags)
# Define to the syslog level for htsearch logging
AC_DEFINE([LOG_LEVEL],[LOG_INFO],[Define to the syslog level for htsearch logging])

# Define to the syslog facility for htsearch logging
AC_DEFINE([LOG_FACILITY],[LOG_LOCAL5],[Define to the syslog facility for htsearch logging])

# Any remaining tests probably want the C compiler
AC_LANG_C
if test ! -d "./db"; then
    mkdir db
fi

if test x$htdig_cv_tests = xyes
then
    CHECK_USER
    AC_PROG_APACHE(1.3.1)
fi

echo configuring hl://Dig version $VERSION

AC_CONFIG_SUBDIRS(db)

### HACK: copied from  autoconf  output, to ensure the expansion below works
test "x$prefix" = xNONE && prefix=$ac_default_prefix
# Let make expand exec_prefix.
test "x$exec_prefix" = xNONE && exec_prefix='${prefix}'

# expand variables which appear in  attrs.html  and the man pages
while echo $bindir | grep '\$' >/dev/null ; do
  eval "bindir=$bindir"
done
while echo $COMMON_DIR | grep '\$' >/dev/null ; do
  eval "COMMON_DIR=$COMMON_DIR"
done
while echo $CONFIG_DIR | grep '\$' >/dev/null ; do
  eval "CONFIG_DIR=$CONFIG_DIR"
done
while echo $DEFAULT_CONFIG_FILE | grep '\$' >/dev/null ; do
  eval "DEFAULT_CONFIG_FILE=$DEFAULT_CONFIG_FILE"
done
while echo $DATABASE_DIR | grep '\$' >/dev/null ; do
  eval "DATABASE_DIR=$DATABASE_DIR"
done
while echo $IMAGE_URL_PREFIX | grep '\$' >/dev/null ; do
  eval "IMAGE_URL_PREFIX=$IMAGE_URL_PREFIX"
done
while echo $VERSION | grep '\$' >/dev/null ; do
  eval "VERSION=$VERSION"
done

AC_CONFIG_FILES([Makefile
                 htcommon/Makefile
                 htdb/Makefile
                 htdig/Makefile
                 installdir/htdig.1
                 installdir/htdig-pdfparser.1
                 installdir/htfuzzy.1
                 installdir/htmerge.1
                 installdir/htpurge.1
                 installdir/htstat.1
                 installdir/htdigconfig.8
                 installdir/htdump.1
                 installdir/htload.1
                 installdir/htnotify.1
                 installdir/htsearch.1
                 installdir/rundig.1
                 htfuzzy/Makefile
                 htlib/Makefile
                 htnet/Makefile
                 htsearch/Makefile
                 httools/Makefile
                 htword/Makefile
                 include/Makefile
                 installdir/Makefile
                 test/Makefile
                  po/Makefile.in
                 test/test_functions])
AC_OUTPUT

echo ""
echo ""
echo "Now you must run 'make' followed by 'make install'"
echo ""
