//
// $Id: lib.h,v 1.4.2.1 2002/01/03 23:12:25 grdetil Exp $
//
// $Log: lib.h,v $
// Revision 1.4.2.1  2002/01/03 23:12:25  grdetil
// * htlib/Makefile.in, htlib/lib.h: Removed references to timegm,
//   mytimegm and strptime functions. Removed C source for these.
//
// Revision 1.4  1998/11/02 20:31:07  ghutchis
//
// Call mytimegm.cc instead of timegm.c.
//
// Revision 1.3  1998/10/28 06:51:37  turtle
// *  added extern "C" to mytimegm declaration.  still doesn't seem to help...
//
// Revision 1.2  1998/01/05 05:22:38  turtle
// Added own replacement of timegm()
//
// Revision 1.1.1.1  1997/02/03 17:11:04  turtle
// Initial CVS
//
//
#ifndef _lib_h
#define _lib_h

#include <string.h>
#include <htconfig.h>
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
#define	NOTOK	(-1)

//
// To get rid of inconsistencies between different machines we will ALWAYS
// use our own version of the following routines
//
int mystrcasecmp(char *, const char *);
int mystrncasecmp(char *, const char *, int);

//
// strdup is a really handy function except that the standard version
// relies on malloc and free.  Since it is sometimes hard to keep track
// of when strdup is used and when new is used, we will solve the problem
// be using our own version of strdup which uses new to allocate memory.
//
char *strdup(char *);

//
// The standard strstr() function is limited in that it does case-sensitive
// searches.  This version will ignore case.
//
char *mystrcasestr(char *s, char *pattern);

#endif
