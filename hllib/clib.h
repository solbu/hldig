/*
 Part of the ht://Dig package   <http://www.htdig.org/>
 Copyright (c) 1999-2004 The ht://Dig Group
 For copyright details, see the file COPYING in your distribution
 or the GNU Library General Public License (LGPL) version 2 or later
 <http://www.gnu.org/copyleft/lgpl.html>
*/
#ifndef _clib_h_
#define _clib_h_

#include <sys/types.h>
#include <stdarg.h>

extern "C"
{

#ifndef HAVE_GETCWD
  char *getcwd (char *, size_t);
#endif

#ifndef HAVE_MEMCMP
  int memcmp (const void *, const void *, size_t);
#endif

#ifndef HAVE_MEMCPY
  void *memcpy (void *, const void *, size_t);
#endif

#ifndef HAVE_MEMMOVE
  void *memmove (void *, const void *, size_t);
#endif

#ifndef HAVE_RAISE
  int raise (int);
#endif

#ifndef HAVE_SNPRINTF
  int snprintf (char *, size_t, const char *, ...);
#endif

#ifndef HAVE_STRERROR
  char *strerror (int);
#endif

#ifndef HAVE_VSNPRINTF
  int vsnprintf (char *, size_t, const char *, va_list);
#endif
}

#endif                          /* _clib_h_ */
