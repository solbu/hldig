//
// lib.h
//
// lib: Contains typical declarations and header inclusions used by
//      most sources in this directory.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2003 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: lib.h,v 1.15 2004/01/12 12:48:24 lha Exp $
//

#ifndef _lib_h
#define _lib_h

#ifndef _MSC_VER /* _WIN32 */
#include "clib.h"
#endif

#include <string.h>

#ifdef _MSC_VER /* _WIN32 */
#include "dirent_local.h"
#define S_ISDIR(v)  ((v)&_S_IFDIR)
#define S_ISREG(v)  ((v)&_S_IFREG)
#else
#include <dirent.h> // for scandir
#endif

#ifdef _MSC_VER /* _WIN32 */
#include <io.h>
#include <stdlib.h>
#define S_IFIFO        _S_IFIFO // pipe
#define S_IFBLK        0060000  // block special
#define S_IFLNK        0120000  // symbolic link
#define S_IFSOCK       0140000  // socket
#define S_IFWHT        0160000  // whiteout
#define R_OK           02
#define popen          _popen
#define pclose         _pclose
#define lstat          stat
#define readlink(x,y,z) {-1}
#define sleep(t)       _sleep((t) * 1000)
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

//
// Other defines used throughout the library
//
#define	OK		0
#define	NOTOK		(-1)

//
// To get rid of inconsistencies between different machines we will ALWAYS
// use our own version of the following routines
//
int mystrcasecmp(const char *, const char *);
int mystrncasecmp(const char *, const char *, int);

//
// The standard strstr() function is limited in that it does case-sensitive
// searches.  This version will ignore case.
//
const char *mystrcasestr(const char *s, const char *pattern);

//
// Too many problems with system strptime() functions...  Just use our own
// version of it.
//
char *mystrptime(const char *buf, const char *fmt, struct tm *tm);

//
// timegm() is quite rare, so provide our own.
//
extern "C" time_t Httimegm(struct tm *tm);

#endif
