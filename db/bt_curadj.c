/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997, 1998, 1999, 2000
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char revid[] = "$Id: bt_curadj.c,v 1.1.2.2 2000/09/14 03:13:16 ghutchis Exp $";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>
#endif

#include "db_int.h"
#include "db_page.h"
#include "btree.h"

static int __bam_opd_cursor __P((DB *, DBC *, db_pgno_t, u_int32_t, u_int32_t));

#ifdef DEBUG
/*
 * CDB___bam_cprint --
 *	Display the current internal cursor.
 *
 * PUBLIC: void CDB___bam_cprint __P((DBC *));
 */
void
CDB___bam_cprint(dbc)
	DBC *dbc;
{
	BTREE_CURSOR *cp;

	cp = (BTREE_CURSOR *)dbc->internal;

	fprintf(stderr, "\tinternal: ovflsize: %lu", (u_long)cp->ovflsize);
	if (dbc->dbtype == DB_RECNO)
		fprintf(stderr, " recno: %lu", (u_long)cp->recno);
	if (F_ISSET(cp, C_DELETED))
		fprintf(stderr, " (deleted)");
	fprintf(stderr, "\n");
}
#endif

/*
 * CDB___bam_ca_delete --
 *	Update the cursors when items are deleted and when already deleted
 *	items are overwritten.  Return the number of relevant cursors found.
 *
 * PUBLIC: int CDB___bam_ca_delete __P((DB *, db_pgno_t, u_int32_t, int));
 */
int
CDB___bam_ca_delete(dbp, pgno, indx, delete)
	DB *dbp;
	db_pgno_t pgno;
	u_int32_t indx;
	int delete;
{
	BTREE_CURSOR *cp;
	DBC *dbc;
	int count;		/* !!!: Has to contain max number of cursors. */

	/*
	 * Adjust the cursors.  We don't have to review the cursors for any
	 * thread of control other than the current one, because we have the
	 * page write locked at this point, and any other thread of control
	 * had better be using a different locker ID, meaning only cursors in
	 * our thread of control can be on the page.
	 *
	 * It's possible for multiple cursors within the thread to have write
	 * locks on the same page, but, cursors within a thread must be single
	 * threaded, so all we're locking here is the cursor linked list.
	 */
	MUTEX_THREAD_LOCK(dbp->mutexp);
	for (count = 0, dbc = TAILQ_FIRST(&dbp->active_queue);
	    dbc != NULL; dbc = TAILQ_NEXT(dbc, links)) {
		cp = (BTREE_CURSOR *)dbc->internal;
		if (cp->pgno == pgno && cp->indx == indx) {
			if (delete)
				F_SET(cp, C_DELETED);
			else
				F_CLR(cp, C_DELETED);
			++count;
		}
	}
	MUTEX_THREAD_UNLOCK(dbp->mutexp);

	return (count);
}

/*
 * CDB___ram_ca_delete --
 *	Return the number of relevant cursors.
 *
 * PUBLIC: int CDB___ram_ca_delete __P((DB *, db_pgno_t));
 */
int
CDB___ram_ca_delete(dbp, root_pgno)
	DB *dbp;
	db_pgno_t root_pgno;
{
	DBC *dbc;

	/*
	 * Review the cursors.  See the comment in CDB___bam_ca_delete().
	 */
	MUTEX_THREAD_LOCK(dbp->mutexp);
	for (dbc = TAILQ_FIRST(&dbp->active_queue);
	    dbc != NULL; dbc = TAILQ_NEXT(dbc, links))
		if (dbc->internal->root == root_pgno)
			break;
	MUTEX_THREAD_UNLOCK(dbp->mutexp);
	return (dbc == NULL ? 0 : 1);
}

/*
 * CDB___bam_ca_di --
 *	Adjust the cursors during a delete or insert.
 *
 * PUBLIC: void CDB___bam_ca_di __P((DB *, db_pgno_t, u_int32_t, int));
 */
void
CDB___bam_ca_di(dbp, pgno, indx, adjust)
	DB *dbp;
	db_pgno_t pgno;
	u_int32_t indx;
	int adjust;
{
	DBC *dbc;
	DBC_INTERNAL *cp;

	/*
	 * Adjust the cursors.  See the comment in CDB___bam_ca_delete().
	 */
	MUTEX_THREAD_LOCK(dbp->mutexp);
	for (dbc = TAILQ_FIRST(&dbp->active_queue);
	    dbc != NULL; dbc = TAILQ_NEXT(dbc, links)) {
		if (dbc->dbtype == DB_RECNO)
			continue;
		cp = dbc->internal;
		if (cp->pgno == pgno && cp->indx >= indx) {
			/* Cursor indices should never be negative. */
			DB_ASSERT(cp->indx != 0 || adjust > 0);

			cp->indx += adjust;
		}
	}
	MUTEX_THREAD_UNLOCK(dbp->mutexp);
}

/*
 * __bam_opd_cursor -- create a new opd cursor.
 */
static int
__bam_opd_cursor(dbp, dbc, first, tpgno, ti)
	DB *dbp;
	DBC *dbc;
	db_pgno_t tpgno;
	u_int32_t first, ti;
{
	BTREE_CURSOR *cp, *orig_cp;
	DBC *dbc_nopd;
	int ret;

	orig_cp = (BTREE_CURSOR *)dbc->internal;
	dbc_nopd = NULL;

	/*
	 * Allocate a new cursor and create the stack.  If duplicates
	 * are sorted, we've just created an off-page duplicate Btree.
	 * If duplicates aren't sorted, we've just created a Recno tree.
	 */
	if ((ret = CDB___db_icursor(dbp, dbc->txn,
	    dbp->dup_compare == NULL ? DB_RECNO : DB_BTREE,
	    tpgno, 1, &dbc_nopd)) != 0)
		return (ret);

	cp = (BTREE_CURSOR *)dbc_nopd->internal;
	cp->pgno = tpgno;
	cp->indx = ti;

	if (dbp->dup_compare == NULL) {
		/*
		 * Converting to off-page Recno trees is tricky.  The
		 * record number for the cursor is the index + 1 (to
		 * convert to 1-based record numbers).
		 */
		cp->recno = ti + 1;
	}

	/*
	 * Transfer the deleted flag from the top-level cursor to the
	 * created one.
	 */
	if (F_ISSET(orig_cp, C_DELETED)) {
		F_SET(cp, C_DELETED);
		F_CLR(orig_cp, C_DELETED);
	}

	/* Stack the cursors and reset the initial cursor's index. */
	orig_cp->opd = dbc_nopd;
	orig_cp->indx = first;
	return (0);
}

/*
 * CDB___bam_ca_dup --
 *	Adjust the cursors when moving items from a leaf page to a duplicates
 *	page.
 *
 * PUBLIC: int CDB___bam_ca_dup __P((DB *,
 * PUBLIC:    db_pgno_t, u_int32_t, u_int32_t, db_pgno_t, u_int32_t));
 */
int
CDB___bam_ca_dup(dbp, first, fpgno, fi, tpgno, ti)
	DB *dbp;
	db_pgno_t fpgno, tpgno;
	u_int32_t first, fi, ti;
{
	BTREE_CURSOR *orig_cp;
	DBC *dbc;
	int ret;

	/*
	 * Adjust the cursors.  See the comment in CDB___bam_ca_delete().
	 */
loop:
	MUTEX_THREAD_LOCK(dbp->mutexp);
	for (dbc = TAILQ_FIRST(&dbp->active_queue);
	    dbc != NULL; dbc = TAILQ_NEXT(dbc, links)) {
		/*
		 * Ignore matching entries that have already been moved,
		 * we move from the same location on the leaf page more
		 * than once.
		 */
		orig_cp = (BTREE_CURSOR *)dbc->internal;
		if (orig_cp->opd != NULL ||
		    orig_cp->pgno != fpgno || orig_cp->indx != fi)
			continue;

		MUTEX_THREAD_UNLOCK(dbp->mutexp);
		if ((ret = __bam_opd_cursor(dbp, dbc, first, tpgno, ti)) !=0)
			return (ret);
		/* We released the MUTEX to get a cursor, start over. */
		goto loop;
	}
	MUTEX_THREAD_UNLOCK(dbp->mutexp);

	return (0);
}

/*
 * CDB___bam_ca_rsplit --
 *	Adjust the cursors when doing reverse splits.
 *
 * PUBLIC: void CDB___bam_ca_rsplit __P((DB *, db_pgno_t, db_pgno_t));
 */
void
CDB___bam_ca_rsplit(dbp, fpgno, tpgno)
	DB *dbp;
	db_pgno_t fpgno, tpgno;
{
	DBC *dbc;

	/*
	 * Adjust the cursors.  See the comment in CDB___bam_ca_delete().
	 */
	MUTEX_THREAD_LOCK(dbp->mutexp);
	for (dbc = TAILQ_FIRST(&dbp->active_queue);
	    dbc != NULL; dbc = TAILQ_NEXT(dbc, links)) {
		if (dbc->dbtype == DB_RECNO)
			continue;
		if (dbc->internal->pgno == fpgno)
			dbc->internal->pgno = tpgno;
	}
	MUTEX_THREAD_UNLOCK(dbp->mutexp);
}

/*
 * CDB___bam_ca_split --
 *	Adjust the cursors when splitting a page.
 *
 * PUBLIC: void CDB___bam_ca_split __P((DB *,
 * PUBLIC:    db_pgno_t, db_pgno_t, db_pgno_t, u_int32_t, int));
 */
void
CDB___bam_ca_split(dbp, ppgno, lpgno, rpgno, split_indx, cleft)
	DB *dbp;
	db_pgno_t ppgno, lpgno, rpgno;
	u_int32_t split_indx;
	int cleft;
{
	DBC *dbc;
	DBC_INTERNAL *cp;

	/*
	 * Adjust the cursors.  See the comment in CDB___bam_ca_delete().
	 *
	 * If splitting the page that a cursor was on, the cursor has to be
	 * adjusted to point to the same record as before the split.  Most
	 * of the time we don't adjust pointers to the left page, because
	 * we're going to copy its contents back over the original page.  If
	 * the cursor is on the right page, it is decremented by the number of
	 * records split to the left page.
	 */
	MUTEX_THREAD_LOCK(dbp->mutexp);
	for (dbc = TAILQ_FIRST(&dbp->active_queue);
	    dbc != NULL; dbc = TAILQ_NEXT(dbc, links)) {
		if (dbc->dbtype == DB_RECNO)
			continue;
		cp = dbc->internal;
		if (cp->pgno == ppgno) {
			if (cp->indx < split_indx) {
				if (cleft)
					cp->pgno = lpgno;
			} else {
				cp->pgno = rpgno;
				cp->indx -= split_indx;
			}
		}
	}
	MUTEX_THREAD_UNLOCK(dbp->mutexp);
}
