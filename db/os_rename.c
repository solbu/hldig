/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 1998, 1999
 *	Sleepycat Software.  All rights reserved.
 */

#include "db_config.h"

#ifndef lint
static const char sccsid[] = "@(#)os_rename.c	11.1 (Sleepycat) 7/25/99";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#ifndef _MSC_VER //_WIN32
#include <unistd.h>
#endif

#endif

#include "db_int.h"
#include "os_jump.h"

/*
 * CDB___os_rename --
 *	Rename a file.
 *
 * PUBLIC: int CDB___os_rename __P((const char *, const char *));
 */
int
CDB___os_rename(old, new)
	const char *old, *new;
{
	int ret;

	ret = CDB___db_jump.j_rename != NULL ?
	    CDB___db_jump.j_rename(old, new) : rename(old, new);
	return (ret == -1 ? CDB___os_get_errno() : 0);
}
