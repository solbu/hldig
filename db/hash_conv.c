/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997, 1998, 1999, 2000
 *	Sleepycat Software.  All rights reserved.
 */
#include "htconfig.h"

#ifndef lint
static const char revid[] = "$Id: hash_conv.c,v 1.1.2.3 2000/09/17 01:35:06 ghutchis Exp $";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>
#endif

#include "db_int.h"
#include "db_page.h"
#include "db_swap.h"
#include "hash.h"

/*
 * CDB___ham_pgin --
 *	Convert host-specific page layout from the host-independent format
 *	stored on disk.
 *
 * PUBLIC: int CDB___ham_pgin __P((DB_ENV *, db_pgno_t, void *, DBT *));
 */
int
CDB___ham_pgin(dbenv, pg, pp, cookie)
	DB_ENV *dbenv;
	db_pgno_t pg;
	void *pp;
	DBT *cookie;
{
	DB_PGINFO *pginfo;
	PAGE *h;

	h = pp;
	pginfo = (DB_PGINFO *)cookie->data;

	/*
	 * The hash access method does blind reads of pages, causing them
	 * to be created.  If the type field isn't set it's one of them,
	 * initialize the rest of the page and return.
	 */
	if (TYPE(h) != P_HASHMETA && h->pgno == PGNO_INVALID) {
		P_INIT(pp, pginfo->db_pagesize,
		    pg, PGNO_INVALID, PGNO_INVALID, 0, P_HASH, 0);
		return (0);
	}

	if (!pginfo->needswap)
		return (0);

	return (TYPE(h) == P_HASHMETA ?  CDB___ham_mswap(pp) :
	    CDB___db_byteswap(dbenv, pg, pp, pginfo->db_pagesize, 1));
}

/*
 * CDB___ham_pgout --
 *	Convert host-specific page layout to the host-independent format
 *	stored on disk.
 *
 * PUBLIC: int CDB___ham_pgout __P((DB_ENV *, db_pgno_t, void *, DBT *));
 */
int
CDB___ham_pgout(dbenv, pg, pp, cookie)
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
	return (TYPE(h) == P_HASHMETA ?  CDB___ham_mswap(pp) :
	     CDB___db_byteswap(dbenv, pg, pp, pginfo->db_pagesize, 0));
}

/*
 * CDB___ham_mswap --
 *	Swap the bytes on the hash metadata page.
 *
 * PUBLIC: int CDB___ham_mswap __P((void *));
 */
int
CDB___ham_mswap(pg)
	void *pg;
{
	u_int8_t *p;
	int i;

	CDB___db_metaswap(pg);

	p = (u_int8_t *)pg + sizeof(DBMETA);

	SWAP32(p);		/* max_bucket */
	SWAP32(p);		/* high_mask */
	SWAP32(p);		/* low_mask */
	SWAP32(p);		/* ffactor */
	SWAP32(p);		/* nelem */
	SWAP32(p);		/* h_charkey */
	for (i = 0; i < NCACHED; ++i)
		SWAP32(p);	/* spares */
	return (0);
}
