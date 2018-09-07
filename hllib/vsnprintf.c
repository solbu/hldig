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
#include "hlconfig.h"
#endif /* HAVE_CONFIG_H */

#ifndef HAVE_VSNPRINTF

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
 * vsnprintf --
 *  Bounded version of vsprintf.
 *
 * PUBLIC: #ifndef HAVE_VSNPRINTF
 * PUBLIC: int vsnprintf();
 * PUBLIC: #endif
 */

int
vsnprintf (str, n, fmt, ap)
     char *str;
     size_t n;
     const char *fmt;
     va_list ap;
{
  n = 0;

#ifdef SPRINTF_RET_CHARPNT
  (void) vsprintf (str, fmt, ap);
  return (strlen (str));
#else
  return (vsprintf (str, fmt, ap));
#endif
}
#endif /* HAVE_VSNPRINTF */
