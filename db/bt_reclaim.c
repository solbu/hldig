/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1998, 1999, 2000
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char revid[] = "$Id: bt_reclaim.c,v 1.1.2.2 2000/09/14 03:13:16 ghutchis Exp $";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <string.h>
#endif

#include "db_int.h"
#include "db_page.h"
#include "db_shash.h"
#include "lock.h"
#include "btree.h"

/*
 * CDB___bam_reclaim --
 *	Free a database.
 *
 * PUBLIC: int CDB___bam_reclaim __P((DB *, DB_TXN *));
 */
int
CDB___bam_reclaim(dbp, txn)
	DB *dbp;
	DB_TXN *txn;
{
	DBC *dbc;
	int ret, t_ret;

	/* Acquire a cursor. */
	if ((ret = dbp->cursor(dbp, txn, &dbc, 0)) != 0)
		return (ret);

	/* Walk the tree, freeing pages. */
	ret = CDB___bam_traverse(dbc,
	    DB_LOCK_WRITE, dbc->internal->root, CDB___db_reclaim_callback, dbc);

	/* Discard the cursor. */
	if ((t_ret = dbc->c_close(dbc)) != 0 && ret == 0)
		ret = t_ret;

	return (ret);
}
