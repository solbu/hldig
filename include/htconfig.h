/* include/htconfig.h.  Generated automatically by configure.  */
#ifndef _config_h_
#define	_config_h_

#define	HTDIG_VERSION	"3.0.8"
#define	MAX_WORD_LENGTH	12

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#define TIME_WITH_SYS_TIME	1

/* Define if your <sys/time.h> declares struct tm.  */
#define TM_IN_SYS_TIME	0

/* Define if you have the mktime function.  */
#define HAVE_MKTIME	1

/* Define if you have the timegm function.  */
#define HAVE_TIMEGM	0

/* Define if you have the localtime function.  */
#define HAVE_LOCALTIME	1

/* Define if you have the timelocal function.  */
#define HAVE_TIMELOCAL	0

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H	1

/* Define if you have the <limits.h> header file.  */
#define HAVE_LIMITS_H	1

/* Define if you have the <sys/file.h> header file.  */
#define HAVE_SYS_FILE_H	1

/* Define if you have the <sys/ioctl.h> header file.  */
#define HAVE_SYS_IOCTL_H	1

/* Define if you have the <sys/time.h> header file.  */
#define HAVE_SYS_TIME_H	1

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H	1

/* Define if you have the gdbm library (-lgdbm).  */
#define HAVE_LIBGDBM	0

/* Define if you have the tm.tm_gmtoff member of struct tm. */
#define	HAVE_TM_GMTOFF	0

/* Define if you need a prototype for gethostname() */
#define	NEED_PROTO_GETHOSTNAME	0

#endif
