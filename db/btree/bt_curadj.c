/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997, 1998
 *	Sleepycat Software.  All rights reserved.
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)bt_curadj.c	10.68 (Sleepycat) 10/3/98";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>
#endif

#include "db_int.h"
#include "db_page.h"
#include "btree.h"

#ifdef DEBUG
/*
 * __bam_cprint --
 *	Display the current btree cursor list.
 *
 * PUBLIC: int __bam_cprint __P((DB *));
 */
int
__bam_cprint(dbp)
	DB *dbp;
{
	CURSOR *cp;
	DBC *dbc;

	DB_THREAD_LOCK(dbp);
	for (dbc = TAILQ_FIRST(&dbp->active_queue);
	    dbc != NULL; dbc = TAILQ_NEXT(dbc, links)) {
		cp = (CURSOR *)dbc->internal;
		fprintf(stderr,
	    "%#0x: page: %lu index: %lu dpage %lu dindex: %lu recno: %lu",
		    (u_int)cp, (u_long)cp->pgno, (u_long)cp->indx,
		    (u_long)cp->dpgno, (u_long)cp->dindx, (u_long)cp->recno);
		if (F_ISSET(cp, C_DELETED))
			fprintf(stderr, "(deleted)");
		fprintf(stderr, "\n");
	}
	DB_THREAD_UNLOCK(dbp);

	return (0);
}
#endif /* DEBUG */

/*
 * __bam_ca_delete --
 * 	Check if any of the cursors refer to the item we are about to delete,
 *	returning the number of cursors that refer to the item in question.
 *
 * PUBLIC: int __bam_ca_delete __P((DB *, db_pgno_t, u_int32_t, CURSOR *));
 */
int
__bam_ca_delete(dbp, pgno, indx, curs)
	DB *dbp;
	db_pgno_t pgno;
	u_int32_t indx;
	CURSOR *curs;
{
	DBC *dbc;
	CURSOR *cp;
	int count;		/* !!!: Has to contain max number of cursors. */

	/*
	 * Adjust the cursors.  We don't have to review the cursors for any
	 * process other than the current one, because we have the page write
	 * locked at this point, and any other process had better be using a
	 * different locker ID, meaning that only cursors in our process can
	 * be on the page.
	 *
	 * It's possible for multiple cursors within the thread to have write
	 * locks on the same page, but, cursors within a thread must be single
	 * threaded, so all we're locking here is the cursor linked list.
	 */
	DB_THREAD_LOCK(dbp);
	for (count = 0, dbc = TAILQ_FIRST(&dbp->active_queue);
	    dbc != NULL; dbc = TAILQ_NEXT(dbc, links)) {
		cp = (CURSOR *)dbc->internal;

		/*
		 * Optionally, a cursor passed in is the one initiating the
		 * delete, so we don't want to count it or set its deleted
		 * flag.  Otherwise, if a cursor refers to the item, then we
		 * set its deleted flag.
		 */
		if (curs == cp)
			continue;

		if ((cp->pgno == pgno && cp->indx == indx) ||
		    (cp->dpgno == pgno && cp->dindx == indx)) {
			++count;
			F_SET(cp, C_DELETED);
		}
	}
	DB_THREAD_UNLOCK(dbp);

	return (count);
}

/*
 * __bam_ca_di --
 *	Adjust the cursors during a delete or insert.
 *
 * PUBLIC: void __bam_ca_di __P((DB *, db_pgno_t, u_int32_t, int));
 */
void
__bam_ca_di(dbp, pgno, indx, adjust)
	DB *dbp;
	db_pgno_t pgno;
	u_int32_t indx;
	int adjust;
{
	CURSOR *cp;
	DBC *dbc;

	/* Recno is responsible for its own adjustments. */
	if (dbp->type == DB_RECNO)
		return;

	/*
	 * Adjust the cursors.  See the comment in __bam_ca_delete().
	 */
	DB_THREAD_LOCK(dbp);
	for (dbc = TAILQ_FIRST(&dbp->active_queue);
	    dbc != NULL; dbc = TAILQ_NEXT(dbc, links)) {
		cp = (CURSOR *)dbc->internal;
		if (cp->pgno == pgno && cp->indx >= indx)
			cp->indx += adjust;
		if (cp->dpgno == pgno && cp->dindx >= indx)
			cp->dindx += adjust;
	}
	DB_THREAD_UNLOCK(dbp);
}

/*
 * __bam_ca_dup --
 *	Adjust the cursors when moving data items to a duplicates page.
 *
 * PUBLIC: void __bam_ca_dup __P((DB *,
 * PUBLIC:    db_pgno_t, u_int32_t, u_int32_t, db_pgno_t, u_int32_t));
 */
void
__bam_ca_dup(dbp, fpgno, first, fi, tpgno, ti)
	DB *dbp;
	db_pgno_t fpgno, tpgno;
	u_int32_t first, fi, ti;
{
	CURSOR *cp;
	DBC *dbc;

	/*
	 * Adjust the cursors.  See the comment in __bam_ca_delete().
	 *
	 * No need to test duplicates, this only gets called when moving
	 * leaf page data items onto a duplicates page.
	 */
	DB_THREAD_LOCK(dbp);
	for (dbc = TAILQ_FIRST(&dbp->active_queue);
	    dbc != NULL; dbc = TAILQ_NEXT(dbc, links)) {
		cp = (CURSOR *)dbc->internal;
		/*
		 * Ignore matching entries that have already been moved,
		 * we move from the same location on the leaf page more
		 * than once.
		 */
		if (cp->dpgno == PGNO_INVALID &&
		    cp->pgno == fpgno && cp->indx == fi) {
			cp->indx = first;
			cp->dpgno = tpgno;
			cp->dindx = ti;
		}
	}
	DB_THREAD_UNLOCK(dbp);
}

/*
 * __bam_ca_move --
 *	Adjust the cursors when moving data items to another page.
 *
 * PUBLIC: void __bam_ca_move __P((DB *, db_pgno_t, db_pgno_t));
 */
void
__bam_ca_move(dbp, fpgno, tpgno)
	DB *dbp;
	db_pgno_t fpgno, tpgno;
{
	CURSOR *cp;
	DBC *dbc;

	/* Recno is responsible for its own adjustments. */
	if (dbp->type == DB_RECNO)
		return;

	/*
	 * Adjust the cursors.  See the comment in __bam_ca_delete().
	 *
	 * No need to test duplicates, this only gets called when copying
	 * over the root page with a leaf or internal page.
	 */
	DB_THREAD_LOCK(dbp);
	for (dbc = TAILQ_FIRST(&dbp->active_queue);
	    dbc != NULL; dbc = TAILQ_NEXT(dbc, links)) {
		cp = (CURSOR *)dbc->internal;
		if (cp->pgno == fpgno)
			cp->pgno = tpgno;
	}
	DB_THREAD_UNLOCK(dbp);
}

/*
 * __bam_ca_replace --
 * 	Check if any of the cursors refer to the item we are about to replace.
 *	If so, their flags should be changed from deleted to replaced.
 *
 * PUBLIC: void __bam_ca_replace
 * PUBLIC:    __P((DB *, db_pgno_t, u_int32_t, ca_replace_arg));
 */
void
__bam_ca_replace(dbp, pgno, indx, pass)
	DB *dbp;
	db_pgno_t pgno;
	u_int32_t indx;
	ca_replace_arg pass;
{
	CURSOR *cp;
	DBC *dbc;

	/*
	 * Adjust the cursors.  See the comment in __bam_ca_delete().
	 *
	 * Find any cursors that have logically deleted a record we're about
	 * to overwrite.
	 *
	 * Pass == REPLACE_SETUP:
	 *	Set C_REPLACE_SETUP so we can find the cursors again.
	 *
	 * Pass == REPLACE_SUCCESS:
	 *	Clear C_DELETED and C_REPLACE_SETUP, set C_REPLACE, the
	 *	overwrite was successful.
	 *
	 * Pass == REPLACE_FAILED:
	 *	Clear C_REPLACE_SETUP, the overwrite failed.
	 *
	 * For REPLACE_SUCCESS and REPLACE_FAILED, we reset the indx value
	 * for the cursor as it may have been changed by other cursor update
	 * routines as the item was deleted/inserted.
	 */
	DB_THREAD_LOCK(dbp);
	switch (pass) {
	case REPLACE_SETUP:			/* Setup. */
		for (dbc = TAILQ_FIRST(&dbp->active_queue);
		    dbc != NULL; dbc = TAILQ_NEXT(dbc, links)) {
			cp = (CURSOR *)dbc->internal;
			if ((cp->pgno == pgno && cp->indx == indx) ||
			    (cp->dpgno == pgno && cp->dindx == indx))
				F_SET(cp, C_REPLACE_SETUP);
		}
		break;
	case REPLACE_SUCCESS:			/* Overwrite succeeded. */
		for (dbc = TAILQ_FIRST(&dbp->active_queue);
		    dbc != NULL; dbc = TAILQ_NEXT(dbc, links)) {
			cp = (CURSOR *)dbc->internal;
			if (F_ISSET(cp, C_REPLACE_SETUP)) {
				if (cp->dpgno == pgno)
					cp->dindx = indx;
				if (cp->pgno == pgno)
					cp->indx = indx;
				F_SET(cp, C_REPLACE);
				F_CLR(cp, C_DELETED | C_REPLACE_SETUP);
			}
		}
		break;
	case REPLACE_FAILED:			/* Overwrite failed. */
		for (dbc = TAILQ_FIRST(&dbp->active_queue);
		    dbc != NULL; dbc = TAILQ_NEXT(dbc, links)) {
			cp = (CURSOR *)dbc->internal;
			if (F_ISSET(cp, C_REPLACE_SETUP)) {
				if (cp->dpgno == pgno)
					cp->dindx = indx;
				if (cp->pgno == pgno)
					cp->indx = indx;
				F_CLR(cp, C_REPLACE_SETUP);
			}
		}
		break;
	}
	DB_THREAD_UNLOCK(dbp);
}

/*
 * __bam_ca_split --
 *	Adjust the cursors when splitting a page.
 *
 * PUBLIC: void __bam_ca_split __P((DB *,
 * PUBLIC:    db_pgno_t, db_pgno_t, db_pgno_t, u_int32_t, int));
 */
void
__bam_ca_split(dbp, ppgno, lpgno, rpgno, split_indx, cleft)
	DB *dbp;
	db_pgno_t ppgno, lpgno, rpgno;
	u_int32_t split_indx;
	int cleft;
{
	DBC *dbc;
	CURSOR *cp;

	/* Recno is responsible for its own adjustments. */
	if (dbp->type == DB_RECNO)
		return;

	/*
	 * Adjust the cursors.  See the comment in __bam_ca_delete().
	 *
	 * If splitting the page that a cursor was on, the cursor has to be
	 * adjusted to point to the same record as before the split.  Most
	 * of the time we don't adjust pointers to the left page, because
	 * we're going to copy its contents back over the original page.  If
	 * the cursor is on the right page, it is decremented by the number of
	 * records split to the left page.
	 */
	DB_THREAD_LOCK(dbp);
	for (dbc = TAILQ_FIRST(&dbp->active_queue);
	    dbc != NULL; dbc = TAILQ_NEXT(dbc, links)) {
		cp = (CURSOR *)dbc->internal;
		if (cp->pgno == ppgno)
			if (cp->indx < split_indx) {
				if (cleft)
					cp->pgno = lpgno;
			} else {
				cp->pgno = rpgno;
				cp->indx -= split_indx;
			}
		if (cp->dpgno == ppgno)
			if (cp->dindx < split_indx) {
				if (cleft)
					cp->dpgno = lpgno;
			} else {
				cp->dpgno = rpgno;
				cp->dindx -= split_indx;
			}
	}
	DB_THREAD_UNLOCK(dbp);
}
