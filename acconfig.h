/*

 Part of the ht://Dig package   <http://www.htdig.org/>
 Copyright (c) 1999, 2000 The ht://Dig Group
 For copyright details, see the file COPYING in your distribution
 or the GNU General Public License version 2 or later
 <http://www.gnu.org/copyleft/gpl.html>

*/
/* Defined in case the compiler doesn't have TRUE and FALSE constants */

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* Define to the syslog level for htsearch logging. */
#define LOG_LEVEL LOG_INFO

/* Define to the syslog facility for htsearch logging. */
#define LOG_FACILITY LOG_LOCAL5

@TOP@

/* Define if you are building a version for running the test suite. */
#undef CONFIG_TEST

/* Define if you want a debugging version. */
#undef DEBUG

/* Define if you want a version that logs read operations. */
#undef DEBUG_ROP

/* Define if you want a version that logs write operations. */
#undef DEBUG_WOP

/* Define if you want a version with run-time diagnostic checking. */
#undef DIAGNOSTIC

/* Define if fcntl/F_SETFD denies child access to file descriptors. */
#undef HAVE_FCNTL_F_SETFD

/* Mutex possibilities. */
#undef HAVE_MUTEX_FCNTL
#undef HAVE_MUTEX_TAS
#undef HAVE_MUTEX_PTHREAD

#undef HAVE_MUTEX_68K_GCC_ASSEMBLY
#undef HAVE_MUTEX_AIX_CHECK_LOCK
#undef HAVE_MUTEX_ALPHA_GCC_ASSEMBLY
#undef HAVE_MUTEX_HPPA_GCC_ASSEMBLY
#undef HAVE_MUTEX_HPPA_MSEM_INIT
#undef HAVE_MUTEX_IA64_GCC_ASSEMBLY
#undef HAVE_MUTEX_MACOS
#undef HAVE_MUTEX_MSEM_INIT
#undef HAVE_MUTEX_PTHREADS
#undef HAVE_MUTEX_RELIANTUNIX_INITSPIN
#undef HAVE_MUTEX_SCO_X86_CC_ASSEMBLY
#undef HAVE_MUTEX_SEMA_INIT
#undef HAVE_MUTEX_SGI_INIT_LOCK
#undef HAVE_MUTEX_SOLARIS_LOCK_TRY
#undef HAVE_MUTEX_SOLARIS_LWP
#undef HAVE_MUTEX_SPARC_GCC_ASSEMBLY
#undef HAVE_MUTEX_THREADS
#undef HAVE_MUTEX_UI_THREADS
#undef HAVE_MUTEX_UTS_CC_ASSEMBLY
#undef HAVE_MUTEX_VXWORKS
#undef HAVE_MUTEX_VMS
#undef HAVE_MUTEX_WIN16
#undef HAVE_MUTEX_WIN32
#undef HAVE_MUTEX_X86_GCC_ASSEMBLY

/* Define if your sprintf returns a pointer, not a length. */
#undef SPRINTF_RET_CHARPNT

/* Define this to the type of the third argument of getpeername() */
#undef GETPEERNAME_LENGTH_T

/* Define if building big-file environment (e.g., Solaris, HP/UX). */
#undef HAVE_FILE_OFFSET_BITS

/* Define if building big-file environment (Linux). */
#undef HAVE_LARGEFILE_SOURCE

/* Define this to the type of the second argument of select() */
#undef FD_SET_T

/* Define if you have the bool type */
#undef HAVE_BOOL

#ifndef HAVE_BOOL
typedef char bool;
#endif

/* Define if you C++ compiler doesn't know true and false */
#undef HAVE_TRUE
#undef HAVE_FALSE

#ifndef HAVE_TRUE
#define true  TRUE
#endif
#ifndef HAVE_FALSE
#define false FALSE
#endif

/* Define if you need a prototype for gethostname() */
#undef NEED_PROTO_GETHOSTNAME

/* Define if the function strptime is declared in <time.h> */
#undef HAVE_STRPTIME_DECL

/* Define if the included regex doesn't work */
#undef HAVE_BROKEN_REGEX

/*
 * Don't step on the namespace.  Other libraries may have their own
 * implementations of these functions, we don't want to use their
 * implementations or force them to use ours based on the load order.
 */
#ifndef	HAVE_GETCWD
#define	getcwd		__db_Cgetcwd
#endif
#ifndef	HAVE_MEMCMP
#define	memcmp		__db_Cmemcmp
#endif
#ifndef	HAVE_MEMCPY
#define	memcpy		__db_Cmemcpy
#endif
#ifndef	HAVE_MEMMOVE
#define	memmove		__db_Cmemmove
#endif
#ifndef	HAVE_RAISE
#define	raise		__db_Craise
#endif
#ifndef HAVE_SNPRINTF
#define	snprintf	__db_Csnprintf
#endif
#ifndef	HAVE_STRERROR
#define	strerror	__db_Cstrerror
#endif
#ifndef HAVE_VSNPRINTF
#define	vsnprintf	__db_Cvsnprintf
#endif
#ifndef HAVE_STRNCOLL
#define	strncoll	__db_Cstrncoll
#endif

@BOTTOM@

/*
 * Big-file configuration.
 */
#ifdef	HAVE_FILE_OFFSET_BITS
#ifndef _FILE_OFFSET_BITS
#define	_FILE_OFFSET_BITS	64
#endif /* _FILE_OFFSET_BITS */
#endif /* HAVE_FILE_OFFSET_BITS */

#ifdef	HAVE_LARGEFILE_SOURCE
#ifndef _LARGEFILE_SOURCE
#define	_LARGEFILE_SOURCE
#endif /* _LARGEFILE_SOURCE */
#endif /* HAVE_LARGEFILE_SOURCE */
