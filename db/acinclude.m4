dnl @(#)mutex.m4	11.9 (Sleepycat) 10/15/99

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
}], [db_cv_mutex="Solaris/lwp"], [:])
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
}], [db_cv_mutex="UI/threads/library"], [:])
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
}], [db_cv_mutex="UI/threads"], [:])
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
}], [db_cv_mutex="POSIX/pthreads"], [:])
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
}], [db_cv_mutex="POSIX/pthreads/library"], [:])
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
}], [db_cv_mutex="HP/msem_init"], [:])
fi

dnl msemaphore: OSF/1
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
}], [db_cv_mutex="UNIX/msem_init"], [:])
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
}], [db_cv_mutex="SCO/x86/cc-assembly"], [:])
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
[int x; _check_lock(x,0,1); _clear_lock(x,0);],
[db_cv_mutex="AIX/_check_lock"])
fi

dnl Alpha/gcc: OSF/1
if test "$db_cv_mutex" = no; then
AC_TRY_RUN([main(){
#if defined(__alpha)
#if defined(__GNUC__)
exit(0);
#endif
#endif
exit(1);}],
[db_cv_mutex="ALPHA/gcc-assembly"], [:], [:])
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
[db_cv_mutex="HPPA/gcc-assembly"], [:], [:])
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
}], [db_cv_mutex="Sparc/gcc-assembly"], [:], [:])
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
}], [db_cv_mutex="68K/gcc-assembly"], [:], [:])
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
}], [db_cv_mutex="x86/gcc-assembly"], [:], [:])
fi

dnl: uts/cc: UTS
if test "$db_cv_mutex" = no; then
AC_TRY_RUN([main(){
#if defined(_UTS)
	exit(0);
#endif
	exit(1);
}], [db_cv_mutex="UTS/cc-assembly"], [:], [:])
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
dnl @(#)options.m4	11.5 (Sleepycat) 10/15/99

dnl Process user-specified options.
AC_DEFUN(AM_OPTIONS_SET, [

AC_MSG_CHECKING(if --enable-bigfile option specified)
AC_ARG_ENABLE(bigfile,
	[  --enable-bigfile       Enable Linux, AIX, HP/UX, Solaris big files.],
	[db_cv_bigfile="$enable_bigfile"], [db_cv_bigfile="yes"])
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
dnl @(#)programs.m4	11.7 (Sleepycat) 9/3/99

dnl Check for programs used in building/installation.
AC_DEFUN(AM_PROGRAMS_SET, [

AC_PATH_PROG(db_cv_path_ar, ar, missing_ar)
if test "$db_cv_path_ar" = missing_ar; then
	AC_MSG_ERROR([No ar utility found.])
fi
AC_PATH_PROG(db_cv_path_chmod, chmod, missing_chmod)
if test "$db_cv_path_chmod" = missing_chmod; then
	AC_MSG_ERROR([No chmod utility found.])
fi
AC_PATH_PROG(db_cv_path_cp, cp, missing_cp)
if test "$db_cv_path_cp" = missing_cp; then
	AC_MSG_ERROR([No cp utility found.])
fi
AC_PATH_PROG(db_cv_path_ln, ln, missing_ln)
if test "$db_cv_path_ln" = missing_ln; then
	AC_MSG_ERROR([No ln utility found.])
fi
AC_PATH_PROG(db_cv_path_mkdir, mkdir, missing_mkdir)
if test "$db_cv_path_mkdir" = missing_mkdir; then
	AC_MSG_ERROR([No mkdir utility found.])
fi
AC_PATH_PROG(db_cv_path_ranlib, ranlib, missing_ranlib)
AC_PATH_PROG(db_cv_path_rm, rm, missing_rm)
if test "$db_cv_path_rm" = missing_rm; then
	AC_MSG_ERROR([No rm utility found.])
fi
AC_PATH_PROG(db_cv_path_sh, sh, missing_sh)
if test "$db_cv_path_sh" = missing_sh; then
	AC_MSG_ERROR([No sh utility found.])
fi
AC_PATH_PROG(db_cv_path_strip, strip, missing_strip)
if test "$db_cv_path_strip" = missing_strip; then
	AC_MSG_ERROR([No strip utility found.])
fi

dnl Check for programs used in testing.
if test "$db_cv_test" = "yes"; then
	AC_PATH_PROG(db_cv_path_cmp, cmp, missing_cmp)
	if test "$db_cv_path_cmp" = missing_cmp; then
		AC_MSG_ERROR([No cmp utility found.])
	fi
	AC_PATH_PROG(db_cv_path_grep, grep, missing_grep)
	if test "$db_cv_path_grep" = missing_grep; then
		AC_MSG_ERROR([No grep utility found.])
	fi
	AC_PATH_PROG(db_cv_path_kill, kill, missing_kill)
	if test "$db_cv_path_kill" = missing_kill; then
		AC_MSG_ERROR([No kill utility found.])
	fi
	AC_PATH_PROG(db_cv_path_mv, mv, missing_mv)
	if test "$db_cv_path_mv" = missing_mv; then
		AC_MSG_ERROR([No mv utility found.])
	fi
	AC_PATH_PROG(db_cv_path_sed, sed, missing_sed)
	if test "$db_cv_path_sed" = missing_sed; then
		AC_MSG_ERROR([No sed utility found.])
	fi
	AC_PATH_PROG(db_cv_path_sleep, sleep, missing_sleep)
	if test "$db_cv_path_sleep" = missing_sleep; then
		AC_MSG_ERROR([No sleep utility found.])
	fi
	AC_PATH_PROG(db_cv_path_sort, sort, missing_sort)
	if test "$db_cv_path_sort" = missing_sort; then
		AC_MSG_ERROR([No sort utility found.])
	fi
	AC_PATH_PROG(db_cv_path_tr, tr, missing_tr)
	if test "$db_cv_path_tr" = missing_tr; then
		AC_MSG_ERROR([No tr utility found.])
	fi

	dnl Programs we want, but don't necessarily have to have.
	AC_PATH_PROG(db_cv_path_diff, diff)
	AC_PATH_PROG(db_cv_path_perl5, perl5)
fi

])dnl
dnl @(#)types.m4	11.2 (Sleepycat) 10/5/99

dnl Check for the standard shorthand types.
AC_DEFUN(AM_SHORTHAND_TYPES, [dnl

AC_SUBST(ssize_t_decl)
AC_CACHE_CHECK([for ssize_t], db_cv_ssize_t, [dnl
AC_TRY_COMPILE([#include <sys/types.h>], ssize_t foo;,
	[db_cv_ssize_t=yes], [db_cv_ssize_t=no], [:])])
if test "$db_cv_ssize_t" = no; then
	ssize_t_decl="typedef int ssize_t;"
fi

AC_SUBST(u_char_decl)
AC_CACHE_CHECK([for u_char], db_cv_uchar, [dnl
AC_TRY_COMPILE([#include <sys/types.h>], u_char foo;,
	[db_cv_uchar=yes], [db_cv_uchar=no], [:])])
if test "$db_cv_uchar" = no; then
	u_char_decl="typedef unsigned char u_char;"
fi

AC_SUBST(u_short_decl)
AC_CACHE_CHECK([for u_short], db_cv_ushort, [dnl
AC_TRY_COMPILE([#include <sys/types.h>], u_short foo;,
	[db_cv_ushort=yes], [db_cv_ushort=no], [:])])
if test "$db_cv_ushort" = no; then
	u_short_decl="typedef unsigned short u_short;"
fi

AC_SUBST(u_int_decl)
AC_CACHE_CHECK([for u_int], db_cv_uint, [dnl
AC_TRY_COMPILE([#include <sys/types.h>], u_int foo;,
	[db_cv_uint=yes], [db_cv_uint=no], [:])])
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
	    [db_cv_uint8="unsigned char"], [db_cv_uint8=no], [:]))])
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
	[db_cv_uint16="unsigned int"], [db_cv_uint16=no], [:])))], [:])
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
	[db_cv_int16="int"], [db_cv_int16=no], [:])))], [:])
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
	[db_cv_uint32="unsigned long"], [db_cv_uint32=no], [:])))], [:])
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
	[db_cv_int32="long"], [db_cv_int32=no], [:])))], [:])
if test "$db_cv_int32" = no; then
	AC_MSG_ERROR([No signed 32-bit integral type.])
fi
if test "$db_cv_int32" != yes; then
	int32_decl="typedef $db_cv_int32 int32_t;"
fi
])dnl
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
dnl @version $Id: acinclude.m4,v 1.1.2.5 2000/10/10 03:15:24 ghutchis Exp $
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
	ZLIB_OLD_LDFLAGS=$LDFLAGS
	ZLIB_OLD_CPPFLAGS=$LDFLAGS
	LDFLAGS="$LDFLAGS -L${ZLIB_HOME}/lib"
	CPPFLAGS="$CPPFLAGS -I${ZLIB_HOME}/include"
        AC_LANG_SAVE
        AC_LANG_C
	AC_CHECK_LIB(z, inflateEnd, [zlib_cv_libz=yes], [zlib_cv_libz=no])
        AC_CHECK_HEADER(zlib.h, [zlib_cv_zlib_h=yes], [zlib_cvs_zlib_h=no])
        AC_LANG_RESTORE
	if test "$zlib_cv_libz" = "yes" -a "$zlib_cv_zlib_h" = "yes"
	then
		#
		# If both library and header were found, use them
		#
		AC_CHECK_LIB(z, inflateEnd)
		AC_MSG_CHECKING(zlib in ${ZLIB_HOME})
		AC_MSG_RESULT(ok)
	else
		#
		# If either header or library was not found, revert and bomb
		#
		AC_MSG_CHECKING(zlib in ${ZLIB_HOME})
		LDFLAGS="$ZLIB_OLD_LDFLAGS"
		CPPFLAGS="$ZLIB_OLD_CPPFLAGS"
		AC_MSG_RESULT(failed)
		AC_MSG_ERROR(either specify a valid zlib installation with --with-zlib=DIR or disable zlib usage with --without-zlib)
	fi
fi

])
dnl @synopsis AC_COMPILE_WARNINGS
dnl
dnl Set the maximum warning verbosity according to compiler used.
dnl Currently supports g++ and gcc.
dnl This macro must be put after AC_PROG_CC and AC_PROG_CXX in
dnl configure.in
dnl
dnl @version $Id: acinclude.m4,v 1.1.2.5 2000/10/10 03:15:24 ghutchis Exp $
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
