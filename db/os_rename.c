/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 1998, 1999, 2000
 *	Sleepycat Software.  All rights reserved.
 */

#include "htconfig.h"

#ifndef lint
static const char revid[] = "$Id: os_rename.c,v 1.1.2.3 2000/09/17 01:35:07 ghutchis Exp $";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <string.h>
#include <unistd.h>
#endif

#include "db_int.h"
#include "os_jump.h"

/*
 * CDB___os_rename --
 *	Rename a file.
 *
 * PUBLIC: int CDB___os_rename __P((DB_ENV *, const char *, const char *));
 */
int
CDB___os_rename(dbenv, old, new)
	DB_ENV *dbenv;
	const char *old, *new;
{
	int ret;

	ret = CDB___db_jump.j_rename != NULL ?
	    CDB___db_jump.j_rename(old, new) : rename(old, new);

	if (ret == -1) {
		ret = CDB___os_get_errno();
		CDB___db_err(dbenv, "Rename %s %s: %s", old, new, strerror(ret));
	}

	return (ret);
}
