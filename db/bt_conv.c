/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997, 1998, 1999, 2000
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char revid[] = "$Id: bt_conv.c,v 1.1.2.2 2000/09/14 03:13:16 ghutchis Exp $";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>
#endif

#include "db_int.h"
#include "db_page.h"
#include "db_swap.h"
#include "btree.h"

/*
 * CDB___bam_pgin --
 *	Convert host-specific page layout from the host-independent format
 *	stored on disk.
 *
 * PUBLIC: int CDB___bam_pgin __P((DB_ENV *, db_pgno_t, void *, DBT *));
 */
int
CDB___bam_pgin(dbenv, pg, pp, cookie)
	DB_ENV *dbenv;
	db_pgno_t pg;
	void *pp;
	DBT *cookie;
{
	DB_PGINFO *pginfo;
	PAGE *h;

	pginfo = (DB_PGINFO *)cookie->data;
	if (!pginfo->needswap)
		return (0);

	h = pp;
	return (TYPE(h) == P_BTREEMETA ?  CDB___bam_mswap(pp) :
	     CDB___db_byteswap(dbenv, pg, pp, pginfo->db_pagesize, 1));
}

/*
 * CDB___bam_pgout --
 *	Convert host-specific page layout to the host-independent format
 *	stored on disk.
 *
 * PUBLIC: int CDB___bam_pgout __P((DB_ENV *, db_pgno_t, void *, DBT *));
 */
int
CDB___bam_pgout(dbenv, pg, pp, cookie)
	DB_ENV *dbenv;
	db_pgno_t pg;
	void *pp;
	DBT *cookie;
{
	DB_PGINFO *pginfo;
	PAGE *h;

	pginfo = (DB_PGINFO *)cookie->data;
	if (!pginfo->needswap)
		return (0);

	h = pp;
	return (TYPE(h) == P_BTREEMETA ?  CDB___bam_mswap(pp) :
	    CDB___db_byteswap(dbenv, pg, pp, pginfo->db_pagesize, 0));
}

/*
 * CDB___bam_mswap --
 *	Swap the bytes on the btree metadata page.
 *
 * PUBLIC: int CDB___bam_mswap __P((PAGE *));
 */
int
CDB___bam_mswap(pg)
	PAGE *pg;
{
	u_int8_t *p;

	CDB___db_metaswap(pg);

	p = (u_int8_t *)pg + sizeof(DBMETA);

	SWAP32(p);		/* maxkey */
	SWAP32(p);		/* minkey */
	SWAP32(p);		/* re_len */
	SWAP32(p);		/* re_pad */
	SWAP32(p);		/* root */

	return (0);
}
