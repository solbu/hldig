/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 1998, 1999, 2000
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char revid[] = "$Id: os_unlink.c,v 1.1.2.2 2000/09/14 03:13:22 ghutchis Exp $";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <string.h>
#include <unistd.h>
#endif

#include "db_int.h"
#include "os_jump.h"

/*
 * CDB___os_unlink --
 *	Remove a file.
 *
 * PUBLIC: int CDB___os_unlink __P((DB_ENV *, const char *));
 */
int
CDB___os_unlink(dbenv, path)
	DB_ENV *dbenv;
	const char *path;
{
	int ret;

	ret = CDB___db_jump.j_unlink != NULL ?
	    CDB___db_jump.j_unlink(path) : unlink(path);
	if (ret == -1) {
		ret = CDB___os_get_errno();
		/*
		 * XXX
		 * We really shouldn't be looking at this value ourselves,
		 * but ENOENT usually signals that a file is missing, and
		 * we attempt to unlink things (such as v. 2.x environment
		 * regions, in DB_ENV->remove) that we're expecting not to
		 * be there.  Reporting errors in these cases is annoying.
		 */
#ifdef HAVE_VXWORKS
		/*
		 * XXX
		 * The results of unlink are file system driver specific
		 * on VxWorks.  In the case of removing a file that did
		 * not exist, some, at least, return an error, but with
		 * an errno of 0, not ENOENT.
		 *
		 * Code below falls through to original if-statement only
		 * we didn't get a "successful" error.
		 */
		if (ret != 0)
		/* FALLTHROUGH */
#endif
		if (ret != ENOENT)
			CDB___db_err(dbenv, "Unlink: %s: %s", path, strerror(ret));
	}

	return (ret);
}
