dnl aclocal.m4 generated automatically by aclocal 1.4a

dnl Copyright (C) 1994, 1995-8, 1999 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY, to the extent permitted by law; without
dnl even the implied warranty of MERCHANTABILITY or FITNESS FOR A
dnl PARTICULAR PURPOSE.

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
dnl @version $Id: aclocal.m4,v 1.2 2002/02/02 18:18:05 ghutchis Exp $
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
dnl @version $Id: aclocal.m4,v 1.2 2002/02/02 18:18:05 ghutchis Exp $
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

# Do all the work for Automake.  This macro actually does too much --
# some checks are only needed if your package does certain things.
# But this isn't really a big deal.

# serial 1

dnl Usage:
dnl AM_INIT_AUTOMAKE(package,version, [no-define])

AC_DEFUN(AM_INIT_AUTOMAKE,
[AC_REQUIRE([AC_PROG_INSTALL])
dnl We require 2.13 because we rely on SHELL being computed by configure.
AC_PREREQ([2.13])
PACKAGE=[$1]
AC_SUBST(PACKAGE)
VERSION=[$2]
AC_SUBST(VERSION)
dnl test to see if srcdir already configured
if test "`cd $srcdir && pwd`" != "`pwd`" && test -f $srcdir/config.status; then
  AC_MSG_ERROR([source directory already configured; run "make distclean" there first])
fi
ifelse([$3],,
AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE", [Name of package])
AC_DEFINE_UNQUOTED(VERSION, "$VERSION", [Version number of package]))
AC_REQUIRE([AM_SANITY_CHECK])
AC_REQUIRE([AC_ARG_PROGRAM])
dnl FIXME This is truly gross.
missing_dir=`cd $ac_aux_dir && pwd`
AM_MISSING_PROG(ACLOCAL, aclocal, $missing_dir)
AM_MISSING_PROG(AUTOCONF, autoconf, $missing_dir)
AM_MISSING_PROG(AUTOMAKE, automake, $missing_dir)
AM_MISSING_PROG(AUTOHEADER, autoheader, $missing_dir)
AM_MISSING_PROG(MAKEINFO, makeinfo, $missing_dir)
AC_REQUIRE([AC_PROG_MAKE_SET])])

#
# Check to make sure that the build environment is sane.
#

AC_DEFUN(AM_SANITY_CHECK,
[AC_MSG_CHECKING([whether build environment is sane])
# Just in case
sleep 1
echo timestamp > conftestfile
# Do `set' in a subshell so we don't clobber the current shell's
# arguments.  Must try -L first in case configure is actually a
# symlink; some systems play weird games with the mod time of symlinks
# (eg FreeBSD returns the mod time of the symlink's containing
# directory).
if (
   set X `ls -Lt $srcdir/configure conftestfile 2> /dev/null`
   if test "[$]*" = "X"; then
      # -L didn't work.
      set X `ls -t $srcdir/configure conftestfile`
   fi
   if test "[$]*" != "X $srcdir/configure conftestfile" \
      && test "[$]*" != "X conftestfile $srcdir/configure"; then

      # If neither matched, then we have a broken ls.  This can happen
      # if, for instance, CONFIG_SHELL is bash and it inherits a
      # broken ls alias from the environment.  This has actually
      # happened.  Such a system could not be considered "sane".
      AC_MSG_ERROR([ls -t appears to fail.  Make sure there is not a broken
alias in your environment])
   fi

   test "[$]2" = conftestfile
   )
then
   # Ok.
   :
else
   AC_MSG_ERROR([newly created file is older than distributed files!
Check your system clock])
fi
rm -f conftest*
AC_MSG_RESULT(yes)])

dnl AM_MISSING_PROG(NAME, PROGRAM, DIRECTORY)
dnl The program must properly implement --version.
AC_DEFUN(AM_MISSING_PROG,
[AC_MSG_CHECKING(for working $2)
# Run test in a subshell; some versions of sh will print an error if
# an executable is not found, even if stderr is redirected.
# Redirect stdin to placate older versions of autoconf.  Sigh.
if ($2 --version) < /dev/null > /dev/null 2>&1; then
   $1=$2
   AC_MSG_RESULT(found)
else
   $1="$3/missing $2"
   AC_MSG_RESULT(missing)
fi
AC_SUBST($1)])

# Like AC_CONFIG_HEADER, but automatically create stamp file.

AC_DEFUN(AM_CONFIG_HEADER,
[AC_PREREQ([2.12])
AC_CONFIG_HEADER([$1])
dnl When config.status generates a header, we must update the stamp-h file.
dnl This file resides in the same directory as the config header
dnl that is generated.  We must strip everything past the first ":",
dnl and everything past the last "/".
AC_OUTPUT_COMMANDS(changequote(<<,>>)dnl
ifelse(patsubst(<<$1>>, <<[^ ]>>, <<>>), <<>>,
<<test -z "<<$>>CONFIG_HEADERS" || echo timestamp > patsubst(<<$1>>, <<^\([^:]*/\)?.*>>, <<\1>>)stamp-h<<>>dnl>>,
<<am_indx=1
for am_file in <<$1>>; do
  case " <<$>>CONFIG_HEADERS " in
  *" <<$>>am_file "*<<)>>
    echo timestamp > `echo <<$>>am_file | sed -e 's%:.*%%' -e 's%[^/]*$%%'`stamp-h$am_indx
    ;;
  esac
  am_indx=`expr "<<$>>am_indx" + 1`
done<<>>dnl>>)
changequote([,]))])


# serial 39 AC_PROG_LIBTOOL
AC_DEFUN(AC_PROG_LIBTOOL,
[AC_REQUIRE([AC_LIBTOOL_SETUP])dnl

# Save cache, so that ltconfig can load it
AC_CACHE_SAVE

# Actually configure libtool.  ac_aux_dir is where install-sh is found.
CC="$CC" CFLAGS="$CFLAGS" CPPFLAGS="$CPPFLAGS" \
LD="$LD" LDFLAGS="$LDFLAGS" LIBS="$LIBS" \
LN_S="$LN_S" NM="$NM" RANLIB="$RANLIB" \
DLLTOOL="$DLLTOOL" AS="$AS" OBJDUMP="$OBJDUMP" \
${CONFIG_SHELL-/bin/sh} $ac_aux_dir/ltconfig --no-reexec \
$libtool_flags --no-verify $ac_aux_dir/ltmain.sh $host \
|| AC_MSG_ERROR([libtool configure failed])

# Reload cache, that may have been modified by ltconfig
AC_CACHE_LOAD

# This can be used to rebuild libtool when needed
LIBTOOL_DEPS="$ac_aux_dir/ltconfig $ac_aux_dir/ltmain.sh"

# Always use our own libtool.
LIBTOOL='$(SHELL) $(top_builddir)/libtool'
AC_SUBST(LIBTOOL)dnl

# Redirect the config.log output again, so that the ltconfig log is not
# clobbered by the next message.
exec 5>>./config.log
])

AC_DEFUN(AC_LIBTOOL_SETUP,
[AC_PREREQ(2.13)dnl
AC_REQUIRE([AC_ENABLE_SHARED])dnl
AC_REQUIRE([AC_ENABLE_STATIC])dnl
AC_REQUIRE([AC_ENABLE_FAST_INSTALL])dnl
AC_REQUIRE([AC_CANONICAL_HOST])dnl
AC_REQUIRE([AC_CANONICAL_BUILD])dnl
AC_REQUIRE([AC_PROG_RANLIB])dnl
AC_REQUIRE([AC_PROG_CC])dnl
AC_REQUIRE([AC_PROG_LD])dnl
AC_REQUIRE([AC_PROG_NM])dnl
AC_REQUIRE([AC_PROG_LN_S])dnl
dnl

# Check for any special flags to pass to ltconfig.
libtool_flags="--cache-file=$cache_file"
test "$enable_shared" = no && libtool_flags="$libtool_flags --disable-shared"
test "$enable_static" = no && libtool_flags="$libtool_flags --disable-static"
test "$enable_fast_install" = no && libtool_flags="$libtool_flags --disable-fast-install"
test "$ac_cv_prog_gcc" = yes && libtool_flags="$libtool_flags --with-gcc"
test "$ac_cv_prog_gnu_ld" = yes && libtool_flags="$libtool_flags --with-gnu-ld"
ifdef([AC_PROVIDE_AC_LIBTOOL_DLOPEN],
[libtool_flags="$libtool_flags --enable-dlopen"])
ifdef([AC_PROVIDE_AC_LIBTOOL_WIN32_DLL],
[libtool_flags="$libtool_flags --enable-win32-dll"])
AC_ARG_ENABLE(libtool-lock,
  [  --disable-libtool-lock  avoid locking (might break parallel builds)])
test "x$enable_libtool_lock" = xno && libtool_flags="$libtool_flags --disable-lock"
test x"$silent" = xyes && libtool_flags="$libtool_flags --silent"

# Some flags need to be propagated to the compiler or linker for good
# libtool support.
case "$host" in
*-*-irix6*)
  # Find out which ABI we are using.
  echo '[#]line __oline__ "configure"' > conftest.$ac_ext
  if AC_TRY_EVAL(ac_compile); then
    case "`/usr/bin/file conftest.o`" in
    *32-bit*)
      LD="${LD-ld} -32"
      ;;
    *N32*)
      LD="${LD-ld} -n32"
      ;;
    *64-bit*)
      LD="${LD-ld} -64"
      ;;
    esac
  fi
  rm -rf conftest*
  ;;

*-*-sco3.2v5*)
  # On SCO OpenServer 5, we need -belf to get full-featured binaries.
  SAVE_CFLAGS="$CFLAGS"
  CFLAGS="$CFLAGS -belf"
  AC_CACHE_CHECK([whether the C compiler needs -belf], lt_cv_cc_needs_belf,
    [AC_TRY_LINK([],[],[lt_cv_cc_needs_belf=yes],[lt_cv_cc_needs_belf=no])])
  if test x"$lt_cv_cc_needs_belf" != x"yes"; then
    # this is probably gcc 2.8.0, egcs 1.0 or newer; no need for -belf
    CFLAGS="$SAVE_CFLAGS"
  fi
  ;;

ifdef([AC_PROVIDE_AC_LIBTOOL_WIN32_DLL],
[*-*-cygwin* | *-*-mingw*)
  AC_CHECK_TOOL(DLLTOOL, dlltool, false)
  AC_CHECK_TOOL(AS, as, false)
  AC_CHECK_TOOL(OBJDUMP, objdump, false)
  ;;
])
esac
])

# AC_LIBTOOL_DLOPEN - enable checks for dlopen support
AC_DEFUN(AC_LIBTOOL_DLOPEN, [AC_BEFORE([$0],[AC_LIBTOOL_SETUP])])

# AC_LIBTOOL_WIN32_DLL - declare package support for building win32 dll's
AC_DEFUN(AC_LIBTOOL_WIN32_DLL, [AC_BEFORE([$0], [AC_LIBTOOL_SETUP])])

# AC_ENABLE_SHARED - implement the --enable-shared flag
# Usage: AC_ENABLE_SHARED[(DEFAULT)]
#   Where DEFAULT is either `yes' or `no'.  If omitted, it defaults to
#   `yes'.
AC_DEFUN(AC_ENABLE_SHARED, [dnl
define([AC_ENABLE_SHARED_DEFAULT], ifelse($1, no, no, yes))dnl
AC_ARG_ENABLE(shared,
changequote(<<, >>)dnl
<<  --enable-shared[=PKGS]  build shared libraries [default=>>AC_ENABLE_SHARED_DEFAULT],
changequote([, ])dnl
[p=${PACKAGE-default}
case "$enableval" in
yes) enable_shared=yes ;;
no) enable_shared=no ;;
*)
  enable_shared=no
  # Look at the argument we got.  We use all the common list separators.
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:,"
  for pkg in $enableval; do
    if test "X$pkg" = "X$p"; then
      enable_shared=yes
    fi
  done
  IFS="$ac_save_ifs"
  ;;
esac],
enable_shared=AC_ENABLE_SHARED_DEFAULT)dnl
])

# AC_DISABLE_SHARED - set the default shared flag to --disable-shared
AC_DEFUN(AC_DISABLE_SHARED, [AC_BEFORE([$0],[AC_LIBTOOL_SETUP])dnl
AC_ENABLE_SHARED(no)])

# AC_ENABLE_STATIC - implement the --enable-static flag
# Usage: AC_ENABLE_STATIC[(DEFAULT)]
#   Where DEFAULT is either `yes' or `no'.  If omitted, it defaults to
#   `yes'.
AC_DEFUN(AC_ENABLE_STATIC, [dnl
define([AC_ENABLE_STATIC_DEFAULT], ifelse($1, no, no, yes))dnl
AC_ARG_ENABLE(static,
changequote(<<, >>)dnl
<<  --enable-static[=PKGS]  build static libraries [default=>>AC_ENABLE_STATIC_DEFAULT],
changequote([, ])dnl
[p=${PACKAGE-default}
case "$enableval" in
yes) enable_static=yes ;;
no) enable_static=no ;;
*)
  enable_static=no
  # Look at the argument we got.  We use all the common list separators.
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:,"
  for pkg in $enableval; do
    if test "X$pkg" = "X$p"; then
      enable_static=yes
    fi
  done
  IFS="$ac_save_ifs"
  ;;
esac],
enable_static=AC_ENABLE_STATIC_DEFAULT)dnl
])

# AC_DISABLE_STATIC - set the default static flag to --disable-static
AC_DEFUN(AC_DISABLE_STATIC, [AC_BEFORE([$0],[AC_LIBTOOL_SETUP])dnl
AC_ENABLE_STATIC(no)])


# AC_ENABLE_FAST_INSTALL - implement the --enable-fast-install flag
# Usage: AC_ENABLE_FAST_INSTALL[(DEFAULT)]
#   Where DEFAULT is either `yes' or `no'.  If omitted, it defaults to
#   `yes'.
AC_DEFUN(AC_ENABLE_FAST_INSTALL, [dnl
define([AC_ENABLE_FAST_INSTALL_DEFAULT], ifelse($1, no, no, yes))dnl
AC_ARG_ENABLE(fast-install,
changequote(<<, >>)dnl
<<  --enable-fast-install[=PKGS]  optimize for fast installation [default=>>AC_ENABLE_FAST_INSTALL_DEFAULT],
changequote([, ])dnl
[p=${PACKAGE-default}
case "$enableval" in
yes) enable_fast_install=yes ;;
no) enable_fast_install=no ;;
*)
  enable_fast_install=no
  # Look at the argument we got.  We use all the common list separators.
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}:,"
  for pkg in $enableval; do
    if test "X$pkg" = "X$p"; then
      enable_fast_install=yes
    fi
  done
  IFS="$ac_save_ifs"
  ;;
esac],
enable_fast_install=AC_ENABLE_FAST_INSTALL_DEFAULT)dnl
])

# AC_ENABLE_FAST_INSTALL - set the default to --disable-fast-install
AC_DEFUN(AC_DISABLE_FAST_INSTALL, [AC_BEFORE([$0],[AC_LIBTOOL_SETUP])dnl
AC_ENABLE_FAST_INSTALL(no)])

# AC_PROG_LD - find the path to the GNU or non-GNU linker
AC_DEFUN(AC_PROG_LD,
[AC_ARG_WITH(gnu-ld,
[  --with-gnu-ld           assume the C compiler uses GNU ld [default=no]],
test "$withval" = no || with_gnu_ld=yes, with_gnu_ld=no)
AC_REQUIRE([AC_PROG_CC])dnl
AC_REQUIRE([AC_CANONICAL_HOST])dnl
AC_REQUIRE([AC_CANONICAL_BUILD])dnl
ac_prog=ld
if test "$ac_cv_prog_gcc" = yes; then
  # Check if gcc -print-prog-name=ld gives a path.
  AC_MSG_CHECKING([for ld used by GCC])
  ac_prog=`($CC -print-prog-name=ld) 2>&5`
  case "$ac_prog" in
    # Accept absolute paths.
changequote(,)dnl
    [\\/]* | [A-Za-z]:[\\/]*)
      re_direlt='/[^/][^/]*/\.\./'
changequote([,])dnl
      # Canonicalize the path of ld
      ac_prog=`echo $ac_prog| sed 's%\\\\%/%g'`
      while echo $ac_prog | grep "$re_direlt" > /dev/null 2>&1; do
	ac_prog=`echo $ac_prog| sed "s%$re_direlt%/%"`
      done
      test -z "$LD" && LD="$ac_prog"
      ;;
  "")
    # If it fails, then pretend we aren't using GCC.
    ac_prog=ld
    ;;
  *)
    # If it is relative, then search for the first ld in PATH.
    with_gnu_ld=unknown
    ;;
  esac
elif test "$with_gnu_ld" = yes; then
  AC_MSG_CHECKING([for GNU ld])
else
  AC_MSG_CHECKING([for non-GNU ld])
fi
AC_CACHE_VAL(ac_cv_path_LD,
[if test -z "$LD"; then
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}${PATH_SEPARATOR-:}"
  for ac_dir in $PATH; do
    test -z "$ac_dir" && ac_dir=.
    if test -f "$ac_dir/$ac_prog" || test -f "$ac_dir/$ac_prog$ac_exeext"; then
      ac_cv_path_LD="$ac_dir/$ac_prog"
      # Check to see if the program is GNU ld.  I'd rather use --version,
      # but apparently some GNU ld's only accept -v.
      # Break only if it was the GNU/non-GNU ld that we prefer.
      if "$ac_cv_path_LD" -v 2>&1 < /dev/null | egrep '(GNU|with BFD)' > /dev/null; then
	test "$with_gnu_ld" != no && break
      else
	test "$with_gnu_ld" != yes && break
      fi
    fi
  done
  IFS="$ac_save_ifs"
else
  ac_cv_path_LD="$LD" # Let the user override the test with a path.
fi])
LD="$ac_cv_path_LD"
if test -n "$LD"; then
  AC_MSG_RESULT($LD)
else
  AC_MSG_RESULT(no)
fi
test -z "$LD" && AC_MSG_ERROR([no acceptable ld found in \$PATH])
AC_SUBST(LD)
AC_PROG_LD_GNU
])

AC_DEFUN(AC_PROG_LD_GNU,
[AC_CACHE_CHECK([if the linker ($LD) is GNU ld], ac_cv_prog_gnu_ld,
[# I'd rather use --version here, but apparently some GNU ld's only accept -v.
if $LD -v 2>&1 </dev/null | egrep '(GNU|with BFD)' 1>&5; then
  ac_cv_prog_gnu_ld=yes
else
  ac_cv_prog_gnu_ld=no
fi])
])

# AC_PROG_NM - find the path to a BSD-compatible name lister
AC_DEFUN(AC_PROG_NM,
[AC_MSG_CHECKING([for BSD-compatible nm])
AC_CACHE_VAL(ac_cv_path_NM,
[if test -n "$NM"; then
  # Let the user override the test.
  ac_cv_path_NM="$NM"
else
  IFS="${IFS= 	}"; ac_save_ifs="$IFS"; IFS="${IFS}${PATH_SEPARATOR-:}"
  for ac_dir in $PATH /usr/ccs/bin /usr/ucb /bin; do
    test -z "$ac_dir" && ac_dir=.
    if test -f $ac_dir/nm || test -f $ac_dir/nm$ac_exeext ; then
      # Check to see if the nm accepts a BSD-compat flag.
      # Adding the `sed 1q' prevents false positives on HP-UX, which says:
      #   nm: unknown option "B" ignored
      if ($ac_dir/nm -B /dev/null 2>&1 | sed '1q'; exit 0) | egrep /dev/null >/dev/null; then
	ac_cv_path_NM="$ac_dir/nm -B"
	break
      elif ($ac_dir/nm -p /dev/null 2>&1 | sed '1q'; exit 0) | egrep /dev/null >/dev/null; then
	ac_cv_path_NM="$ac_dir/nm -p"
	break
      else
	ac_cv_path_NM=${ac_cv_path_NM="$ac_dir/nm"} # keep the first match, but
	continue # so that we can try to find one that supports BSD flags
      fi
    fi
  done
  IFS="$ac_save_ifs"
  test -z "$ac_cv_path_NM" && ac_cv_path_NM=nm
fi])
NM="$ac_cv_path_NM"
AC_MSG_RESULT([$NM])
AC_SUBST(NM)
])

# AC_CHECK_LIBM - check for math library
AC_DEFUN(AC_CHECK_LIBM,
[AC_REQUIRE([AC_CANONICAL_HOST])dnl
LIBM=
case "$host" in
*-*-beos* | *-*-cygwin*)
  # These system don't have libm
  ;;
*-ncr-sysv4.3*)
  AC_CHECK_LIB(mw, _mwvalidcheckl, LIBM="-lmw")
  AC_CHECK_LIB(m, main, LIBM="$LIBM -lm")
  ;;
*)
  AC_CHECK_LIB(m, main, LIBM="-lm")
  ;;
esac
])

# AC_LIBLTDL_CONVENIENCE[(dir)] - sets LIBLTDL to the link flags for
# the libltdl convenience library, adds --enable-ltdl-convenience to
# the configure arguments.  Note that LIBLTDL is not AC_SUBSTed, nor
# is AC_CONFIG_SUBDIRS called.  If DIR is not provided, it is assumed
# to be `${top_builddir}/libltdl'.  Make sure you start DIR with
# '${top_builddir}/' (note the single quotes!) if your package is not
# flat, and, if you're not using automake, define top_builddir as
# appropriate in the Makefiles.
AC_DEFUN(AC_LIBLTDL_CONVENIENCE, [AC_BEFORE([$0],[AC_LIBTOOL_SETUP])dnl
  case "$enable_ltdl_convenience" in
  no) AC_MSG_ERROR([this package needs a convenience libltdl]) ;;
  "") enable_ltdl_convenience=yes
      ac_configure_args="$ac_configure_args --enable-ltdl-convenience" ;;
  esac
  LIBLTDL=ifelse($#,1,$1,['${top_builddir}/libltdl'])/libltdlc.la
])

# AC_LIBLTDL_INSTALLABLE[(dir)] - sets LIBLTDL to the link flags for
# the libltdl installable library, and adds --enable-ltdl-install to
# the configure arguments.  Note that LIBLTDL is not AC_SUBSTed, nor
# is AC_CONFIG_SUBDIRS called.  If DIR is not provided, it is assumed
# to be `${top_builddir}/libltdl'.  Make sure you start DIR with
# '${top_builddir}/' (note the single quotes!) if your package is not
# flat, and, if you're not using automake, define top_builddir as
# appropriate in the Makefiles.
# In the future, this macro may have to be called after AC_PROG_LIBTOOL.
AC_DEFUN(AC_LIBLTDL_INSTALLABLE, [AC_BEFORE([$0],[AC_LIBTOOL_SETUP])dnl
  AC_CHECK_LIB(ltdl, main,
  [test x"$enable_ltdl_install" != xyes && enable_ltdl_install=no],
  [if test x"$enable_ltdl_install" = xno; then
     AC_MSG_WARN([libltdl not installed, but installation disabled])
   else
     enable_ltdl_install=yes
   fi
  ])
  if test x"$enable_ltdl_install" = x"yes"; then
    ac_configure_args="$ac_configure_args --enable-ltdl-install"
    LIBLTDL=ifelse($#,1,$1,['${top_builddir}/libltdl'])/libltdl.la
  else
    ac_configure_args="$ac_configure_args --enable-ltdl-install=no"
    LIBLTDL="-lltdl"
  fi
])

dnl old names
AC_DEFUN(AM_PROG_LIBTOOL, [indir([AC_PROG_LIBTOOL])])dnl
AC_DEFUN(AM_ENABLE_SHARED, [indir([AC_ENABLE_SHARED], $@)])dnl
AC_DEFUN(AM_ENABLE_STATIC, [indir([AC_ENABLE_STATIC], $@)])dnl
AC_DEFUN(AM_DISABLE_SHARED, [indir([AC_DISABLE_SHARED], $@)])dnl
AC_DEFUN(AM_DISABLE_STATIC, [indir([AC_DISABLE_STATIC], $@)])dnl
AC_DEFUN(AM_PROG_LD, [indir([AC_PROG_LD])])dnl
AC_DEFUN(AM_PROG_NM, [indir([AC_PROG_NM])])dnl

dnl This is just to quiet aclocal about the macro not being used
if(a,b,[AC_DISABLE_FAST_INSTALL])dnl

# Add --enable-maintainer-mode option to configure.
# From Jim Meyering

# serial 1

AC_DEFUN(AM_MAINTAINER_MODE,
[AC_MSG_CHECKING([whether to enable maintainer-specific portions of Makefiles])
  dnl maintainer-mode is disabled by default
  AC_ARG_ENABLE(maintainer-mode,
[  --enable-maintainer-mode enable make rules and dependencies not useful
                          (and sometimes confusing) to the casual installer],
      USE_MAINTAINER_MODE=$enableval,
      USE_MAINTAINER_MODE=no)
  AC_MSG_RESULT($USE_MAINTAINER_MODE)
  AM_CONDITIONAL(MAINTAINER_MODE, test $USE_MAINTAINER_MODE = yes)
  MAINT=$MAINTAINER_MODE_TRUE
  AC_SUBST(MAINT)dnl
]
)

# Define a conditional.

AC_DEFUN(AM_CONDITIONAL,
[AC_SUBST($1_TRUE)
AC_SUBST($1_FALSE)
if $2; then
  $1_TRUE=
  $1_FALSE='#'
else
  $1_TRUE='#'
  $1_FALSE=
fi])

