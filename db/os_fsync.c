/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 1998, 1999
 *	Sleepycat Software.  All rights reserved.
 */

#include "db_config.h"

#ifndef lint
static const char sccsid[] = "@(#)os_fsync.c	11.2 (Sleepycat) 9/22/99";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <fcntl.h>			/* XXX: Required by __hp3000s900 */

#ifndef _MSC_VER /* _WIN32 */
#include <unistd.h>
#endif
#endif

#include "db_int.h"
#include "os_jump.h"

#ifdef _MSC_VER /* _WIN32 */
#include <io.h>
#define fsync(fd)     _commit(fd);
#endif

#ifdef __hp3000s900
int
__mpe_fsync(fd)
	int fd;
{
	extern FCONTROL(short, short, void *);

	FCONTROL(_MPE_FILENO(fd), 2, NULL);	/* Flush the buffers */
	FCONTROL(_MPE_FILENO(fd), 6, NULL);	/* Write the EOF */
	return (0);
}
#endif

#ifdef __hp3000s900
#define	fsync(fd)	__mpe_fsync(fd);
#endif

/*
 * CDB___os_fsync --
 *	Flush a file descriptor.
 *
 * PUBLIC: int CDB___os_fsync __P((DB_FH *));
 */
int
CDB___os_fsync(fhp)
	DB_FH *fhp;
{
	int ret;

	/*
	 * Do nothing if the file descriptor has been marked as not requiring
	 * any sync to disk.
	 */
	if (F_ISSET(fhp, DB_FH_NOSYNC))
		return (0);

	ret = CDB___db_jump.j_fsync != NULL ?
	    CDB___db_jump.j_fsync(fhp->fd) : fsync(fhp->fd);

	return (ret == 0 ? 0 : CDB___os_get_errno());
}
