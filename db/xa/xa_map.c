/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997, 1998
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)xa_map.c	10.4 (Sleepycat) 10/20/98";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <string.h>
#endif

#include "db_int.h"
#include "shqueue.h"
#include "txn.h"

/*
 * This file contains all the mapping information that we need to support
 * the DB/XA interface.
 */

/*
 * __db_rmid_to_env
 *	Return the environment associated with a given XA rmid.
 *
 * PUBLIC: int __db_rmid_to_env __P((int rmid, DB_ENV **envp));
 */
int
__db_rmid_to_env(rmid, envp)
	int rmid;
	DB_ENV **envp;
{
	DB_ENV *env;

	env = TAILQ_FIRST(&DB_GLOBAL(db_envq));
	if (env->xa_rmid == rmid) {
		*envp = env;
		return (0);
	}

	/*
	 * When we map an rmid, move that environment to be the first one in
	 * the list of environments, so we pass the correct environment from
	 * the upcoming db_xa_open() call into db_open().
	 */
	for (; env != NULL; env = TAILQ_NEXT(env, links))
		if (env->xa_rmid == rmid) {
			TAILQ_REMOVE(&DB_GLOBAL(db_envq), env, links);
			TAILQ_INSERT_HEAD(&DB_GLOBAL(db_envq), env, links);
			*envp = env;
			return (0);
		}

	return (1);
}

/*
 * __db_xid_to_txn
 *	Return the txn that corresponds to this XID.
 *
 * PUBLIC: int __db_xid_to_txn __P((DB_ENV *, XID *, size_t *));
 */
int
__db_xid_to_txn(dbenv, xid, offp)
	DB_ENV *dbenv;
	XID *xid;
	size_t *offp;
{
	DB_TXNREGION *tmr;
	struct __txn_detail *td;

	/*
	 * Search the internal active transaction table to find the
	 * matching xid.  If this is a performance hit, then we
	 * can create a hash table, but I doubt it's worth it.
	 */
	tmr = dbenv->tx_info->region;

	LOCK_TXNREGION(dbenv->tx_info);
	for (td = SH_TAILQ_FIRST(&tmr->active_txn, __txn_detail);
	    td != NULL;
	    td = SH_TAILQ_NEXT(td, links, __txn_detail))
		if (memcmp(xid->data, td->xid, XIDDATASIZE) == 0)
			break;
	UNLOCK_TXNREGION(dbenv->tx_info);

	if (td == NULL)
		return (EINVAL);

	*offp = (u_int8_t *)td - (u_int8_t *)tmr;
	return (0);
}

/*
 * __db_map_rmid
 *	Create a mapping between the specified rmid and environment.
 *
 * PUBLIC: int __db_map_rmid __P((int, DB_ENV *));
 */
int
__db_map_rmid(rmid, env)
	int rmid;
	DB_ENV *env;
{
	env->xa_rmid = rmid;
	TAILQ_INSERT_TAIL(&DB_GLOBAL(db_envq), env, links);
	return (0);
}

/*
 * __db_unmap_rmid
 *	Destroy the mapping for the given rmid.
 *
 * PUBLIC: int __db_unmap_rmid __P((int));
 */
int
__db_unmap_rmid(rmid)
	int rmid;
{
	DB_ENV *e;

	for (e = TAILQ_FIRST(&DB_GLOBAL(db_envq));
	    e->xa_rmid != rmid;
	    e = TAILQ_NEXT(e, links));

	if (e == NULL)
		return (EINVAL);

	TAILQ_REMOVE(&DB_GLOBAL(db_envq), e, links);
	return (0);
}

/*
 * __db_map_xid
 *	Create a mapping between this XID and the transaction at
 *	"off" in the shared region.
 *
 * PUBLIC: int __db_map_xid __P((DB_ENV *, XID *, size_t));
 */
int
__db_map_xid(env, xid, off)
	DB_ENV *env;
	XID *xid;
	size_t off;
{
	DB_TXNMGR *tm;
	TXN_DETAIL *td;

	tm = env->tx_info;
	td = (TXN_DETAIL *)((u_int8_t *)tm->region + off);

	LOCK_TXNREGION(tm);
	memcpy(td->xid, xid->data, XIDDATASIZE);
	UNLOCK_TXNREGION(tm);

	return (0);
}

/*
 * __db_unmap_xid
 *	Destroy the mapping for the specified XID.
 *
 * PUBLIC: void __db_unmap_xid __P((DB_ENV *, XID *, size_t));
 */

void
__db_unmap_xid(env, xid, off)
	DB_ENV *env;
	XID *xid;
	size_t off;
{
	TXN_DETAIL *td;

	COMPQUIET(xid, NULL);

	td = (TXN_DETAIL *)((u_int8_t *)env->tx_info->region + off);
	memset(td->xid, 0, sizeof(td->xid));
}
