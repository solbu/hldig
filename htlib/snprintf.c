/* Part of the ht://Dig package   <http://www.htdig.org/> */
/* Copyright (c) 1999-2004 The ht://Dig Group */
/* For copyright details, see the file COPYING in your distribution */
/* or the GNU Library General Public License (LGPL) version 2 or later */
/* <http://www.gnu.org/copyleft/lgpl.html> */


/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997, 1998, 1999
 *  Sleepycat Software.  All rights reserved.
 */

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#ifndef HAVE_SNPRINTF

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <stdio.h>
#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#endif

/*
 * snprintf --
 *  Bounded version of sprintf.
 *
 * PUBLIC: #ifndef HAVE_SNPRINTF
 * PUBLIC: #ifdef __STDC__
 * PUBLIC: int snprintf __P((char *, size_t, const char *, ...));
 * PUBLIC: #else
 * PUBLIC: int snprintf();
 * PUBLIC: #endif
 * PUBLIC: #endif
 */
int
#ifdef __STDC__
snprintf(char *str, size_t n, const char *fmt, ...)
#else
snprintf(str, n, fmt, va_alist)
  char *str;
  size_t n;
  const char *fmt;
  va_dcl
#endif
{
  va_list ap;
  int rval;

  n = 0;
#ifdef __STDC__
  va_start(ap, fmt);
#else
  va_start(ap);
#endif
#ifdef SPRINTF_RET_CHARPNT
  (void)vsprintf(str, fmt, ap);
  va_end(ap);
  return (strlen(str));
#else
  rval = vsprintf(str, fmt, ap);
  va_end(ap);
  return (rval);
#endif
}
#endif /* HAVE_SNPRINTF */

