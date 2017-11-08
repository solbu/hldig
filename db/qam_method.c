/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1999
 *	Sleepycat Software.  All rights reserved.
 */

#include "db_config.h"

#ifndef lint
static const char sccsid[] = "@(#)qam_method.c	11.1 (Sleepycat) 8/19/99";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>
#endif

#include "db_int.h"
#include "db_page.h"
#include "qam.h"

/*
 * CDB___qam_db_create --
 *	Queue specific initialization of the DB structure.
 *
 * PUBLIC: int CDB___qam_db_create __P((DB *));
 */
int
CDB___qam_db_create(dbp)
	DB *dbp;
{
	QUEUE *t;
	int ret;

	/* Allocate and initialize the private queue structure. */
	if ((ret = CDB___os_calloc(1, sizeof(QUEUE), &t)) != 0)
		return (ret);
	dbp->q_internal = t;

	t->re_pad = ' ';

	return (0);
}

/*
 * CDB___qam_db_close --
 *	Queue specific discard of the DB structure.
 *
 * PUBLIC: int CDB___qam_db_close __P((DB *));
 */
int
CDB___qam_db_close(dbp)
	DB *dbp;
{
	QUEUE *t;

	t = dbp->q_internal;

	CDB___os_free(t, sizeof(QUEUE));
	dbp->q_internal = NULL;

	return (0);
}
