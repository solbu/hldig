/*

 Part of the ht://Dig package   <http://www.htdig.org/>
 Copyright (c) 1999, 2000 The ht://Dig Group
 For copyright details, see the file COPYING in your distribution
 or the GNU General Public License version 2 or later
 <http://www.gnu.org/copyleft/gpl.html>

*/

#ifndef _config_h_
#define	_config_h_

#define VERSION 1

#define PACKAGE htdig
@TOP@

/* Define this to the type of the third argument of getpeername() */
#undef GETPEERNAME_LENGTH_T

/* Define if you need a prototype for gethostname() */
#undef NEED_PROTO_GETHOSTNAME

/* Define if the included regex doesn't work */
#undef HAVE_BROKEN_REGEX

/* Define if we should use rxposix.h instead of regex.h */
#undef USE_RX

@BOTTOM@

/* Define to the syslog level for htsearch logging. */
#define LOG_LEVEL LOG_INFO

/* Define to the syslog facility for htsearch logging. */
#define LOG_FACILITY LOG_LOCAL5

/* Define this if you're willing to allow htsearch to take -c even as a CGI */
/*  regardless of the security problems with this. */
#undef ALLOW_INSECURE_CGI_CONFIG

/* Define to remove the word count in db and WordRef struct. */
#undef NO_WORD_COUNT

/* Defined in case the compiler doesn't have TRUE and FALSE constants */
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#endif
