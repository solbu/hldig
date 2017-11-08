/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 1998, 1999
 *	Sleepycat Software.  All rights reserved.
 */

#include "db_config.h"

#ifndef lint
static const char sccsid[] = "@(#)os_abs.c	11.1 (Sleepycat) 7/25/99";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>
#endif

#include "db_int.h"

/*
 * CDB___os_abspath --
 *	Return if a path is an absolute path.
 *
 * PUBLIC: int CDB___os_abspath __P((const char *));
 */
int
CDB___os_abspath(path)
	const char *path;
{
#if defined (_WIN32) || defined (__MSDOS__) || defined (__DJGPP__) || defined (__CYGWIN__)
	return (path[0] == '/' || path[0] == '\\' || path [1] == ':');
#else
	return (path[0] == '/');
#endif
}
