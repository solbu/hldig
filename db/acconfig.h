/*
 *	@(#)acconfig.h	11.16 (Sleepycat) 11/9/99
 */

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

/* Define if building big-file environment (e.g., Solaris, HP/UX). */
#undef HAVE_FILE_OFFSET_BITS

/* Define if building big-file environment (Linux). */
#undef HAVE_LARGEFILE_SOURCE

/* Mutex possibilities. */
#undef HAVE_MUTEX_FCNTL
#undef HAVE_MUTEX_TAS
#undef HAVE_MUTEX_PTHREAD

#undef HAVE_MUTEX_68K_GCC_ASSEMBLY
#undef HAVE_MUTEX_AIX_CHECK_LOCK
#undef HAVE_MUTEX_ALPHA_GCC_ASSEMBLY
#undef HAVE_MUTEX_HPPA_GCC_ASSEMBLY
#undef HAVE_MUTEX_HPPA_MSEM_INIT
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
#undef HAVE_MUTEX_VMS
#undef HAVE_MUTEX_WIN16
#undef HAVE_MUTEX_WIN32
#undef HAVE_MUTEX_X86_GCC_ASSEMBLY

/* Define if you have the sigfillset function.  */
#undef HAVE_SIGFILLSET

/* Define if your sprintf returns a pointer, not a length. */
#undef SPRINTF_RET_CHARPNT

@BOTTOM@

/*
 * Big-file configuration.
 */
#ifdef	HAVE_FILE_OFFSET_BITS
#define	_FILE_OFFSET_BITS	64
#endif

#ifdef	HAVE_LARGEFILE_SOURCE
#define	_LARGEFILE_SOURCE
#endif
