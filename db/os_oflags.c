/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 1998, 1999
 *	Sleepycat Software.  All rights reserved.
 */

#include "db_config.h"

#ifndef lint
static const char sccsid[] = "@(#)os_oflags.c	11.1 (Sleepycat) 7/25/99";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>
#include <sys/stat.h>

#include <fcntl.h>
#endif

#include "db_int.h"

/*
 * CDB___db_oflags --
 *	Convert open(2) flags to DB flags.
 *
 * PUBLIC: u_int32_t CDB___db_oflags __P((int));
 */
u_int32_t
CDB___db_oflags(oflags)
	int oflags;
{
	u_int32_t dbflags;

	/*
	 * XXX
	 * Convert POSIX 1003.1 open(2) flags to DB flags.  Not an exact
	 * science as most POSIX implementations don't have a flag value
	 * for O_RDONLY, it's simply the lack of a write flag.
	 */
	dbflags = 0;
	if (oflags & O_CREAT)
		dbflags |= DB_CREATE;
	if (!(oflags & (O_RDWR | O_WRONLY)) || oflags & O_RDONLY)
		dbflags |= DB_RDONLY;
	if (oflags & O_TRUNC)
		dbflags |= DB_TRUNCATE;
	return (dbflags);
}

/*
 * CDB___db_omode --
 *	Convert a permission string to the correct open(2) flags.
 *
 * PUBLIC: int CDB___db_omode __P((const char *));
 */
int
CDB___db_omode(perm)
	const char *perm;
{
	int mode;

    mode = 0;
	if (perm[0] == 'r')
		mode |= S_IRUSR;
	if (perm[1] == 'w')
		mode |= S_IWUSR;
	if (perm[2] == 'r')
		mode |= S_IRGRP;
	if (perm[3] == 'w')
		mode |= S_IWGRP;
	if (perm[4] == 'r')
		mode |= S_IROTH;
	if (perm[5] == 'w')
		mode |= S_IWOTH;
	return (mode);
}
