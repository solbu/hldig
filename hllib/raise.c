/* Part of the ht://Dig package   <http://www.htdig.org/> */
/* Copyright (c) 1999-2004 The ht://Dig Group */
/* For copyright details, see the file COPYING in your distribution */
/* or the GNU Library General Public License (LGPL) version 2 or later */
/* <http://www.gnu.org/copyleft/lgpl.html> */

/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 1998, 1999
 *  Sleepycat Software.  All rights reserved.
 */

#ifdef HAVE_CONFIG_H
#include "hlconfig.h"
#endif /* HAVE_CONFIG_H */

#ifndef HAVE_RAISE

#ifndef NO_SYSTEM_INCLUDES
#include <signal.h>
#include <unistd.h>
#endif

/*
 * raise --
 *  Send a signal to the current process.
 *
 * PUBLIC: #ifndef HAVE_RAISE
 * PUBLIC: int raise __P((int));
 * PUBLIC: #endif
 */
int
raise (s)
     int s;
{
  return (kill (getpid (), s));
}
#endif /* HAVE_RAISE */
