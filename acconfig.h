/* Defined in case the compiler doesn't have TRUE and FALSE constants */
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define	VERSION	1

/* Define to the syslog level for htsearch logging. */
#define LOG_LEVEL LOG_INFO

/* Define to the syslog facility for htsearch logging. */
#define LOG_FACILITY LOG_LOCAL5

@TOP@

/* Define this to the type of the third argument of getpeername() */
#undef GETPEERNAME_LENGTH_T

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

