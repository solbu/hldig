//
// lib.h
//
// lib: Contains typical declarations and header inclusions used by
//      most sources in this directory.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: lib.h,v 1.11.2.5 2000/11/30 20:12:05 grdetil Exp $
//

#ifndef _lib_h
#define _lib_h

#include "clib.h"

#include <string.h>
#include <dirent.h> // for scandir

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
