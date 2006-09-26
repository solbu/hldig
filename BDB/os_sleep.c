/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 1998, 1999
 *	Sleepycat Software.  All rights reserved.
 */

#include "db_config.h"

#ifndef lint
static const char sccsid[] = "@(#)os_sleep.c	11.1 (Sleepycat) 7/25/99";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#if TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif

#ifndef _MSC_VER /* _WIN32 */
#include <unistd.h>
#endif
#endif

#include "db_int.h"
#include "os_jump.h"

/*
 * CDB___os_sleep --
 *	Yield the processor for a period of time.
 *
 * PUBLIC: int CDB___os_sleep __P((u_long, u_long));
 */
int
CDB___os_sleep(secs, usecs)
	u_long secs, usecs;		/* Seconds and microseconds. */
{
	struct timeval t;

	/* Don't require that the values be normalized. */
	for (; usecs >= 1000000; usecs -= 1000000)
		++secs;

	if (CDB___db_jump.j_sleep != NULL)
		return (CDB___db_jump.j_sleep(secs, usecs));

	/*
	 * It's important that we yield the processor here so that other
	 * processes or threads are permitted to run.
	 */
	t.tv_sec = secs;
	t.tv_usec = usecs;
	return (select(0, NULL, NULL, NULL, &t) == -1 ? CDB___os_get_errno() : 0);
}
