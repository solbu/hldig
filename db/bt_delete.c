/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997, 1998, 1999, 2000
 *	Sleepycat Software.  All rights reserved.
 */
/*
 * Copyright (c) 1990, 1993, 1994, 1995, 1996
 *	Keith Bostic.  All rights reserved.
 */
/*
 * Copyright (c) 1990, 1993, 1994, 1995
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Mike Olson.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "htconfig.h"

#ifndef lint
static const char revid[] = "$Id: bt_delete.c,v 1.1.2.3 2000/09/17 01:35:03 ghutchis Exp $";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <string.h>
#endif

#include "db_int.h"
#include "db_page.h"
#include "db_shash.h"
#include "btree.h"
#include "lock.h"

/*
 * CDB___bam_delete --
 *	Delete the items referenced by a key.
 *
 * PUBLIC: int CDB___bam_delete __P((DB *, DB_TXN *, DBT *, u_int32_t));
 */
int
CDB___bam_delete(dbp, txn, key, flags)
	DB *dbp;
	DB_TXN *txn;
	DBT *key;
	u_int32_t flags;
{
	DBC *dbc;
	DBT lkey;
	DBT data;
	u_int32_t f_init, f_next;
	int ret, t_ret;

	PANIC_CHECK(dbp->dbenv);
	DB_ILLEGAL_BEFORE_OPEN(dbp, "DB->del");

	/* Check for invalid flags. */
	if ((ret =
	    CDB___db_delchk(dbp, key, flags, F_ISSET(dbp, DB_AM_RDONLY))) != 0)
		return (ret);

	/* Allocate a cursor. */
	if ((ret = dbp->cursor(dbp, txn, &dbc, DB_WRITELOCK)) != 0)
		return (ret);

	DEBUG_LWRITE(dbc, txn, "bam_delete", key, NULL, flags);

	/*
	 * Walk a cursor through the key/data pairs, deleting as we go.  Set
	 * the DB_DBT_USERMEM flag, as this might be a threaded application
	 * and the flags checking will catch us.  We don't actually want the
	 * keys or data, so request a partial of length 0.
	 */
	memset(&lkey, 0, sizeof(lkey));
	F_SET(&lkey, DB_DBT_USERMEM | DB_DBT_PARTIAL);
	memset(&data, 0, sizeof(data));
	F_SET(&data, DB_DBT_USERMEM | DB_DBT_PARTIAL);

	/*
	 * If locking (and we haven't already acquired CDB locks), set the
	 * read-modify-write flag.
	 */
	f_init = DB_SET;
	f_next = DB_NEXT_DUP;
	if (STD_LOCKING(dbc)) {
		f_init |= DB_RMW;
		f_next |= DB_RMW;
	}

	/* Walk through the set of key/data pairs, deleting as we go. */
	if ((ret = dbc->c_get(dbc, key, &data, f_init)) != 0)
		goto err;
	for (;;) {
		if ((ret = dbc->c_del(dbc, 0)) != 0)
			goto err;
		if ((ret = dbc->c_get(dbc, &lkey, &data, f_next)) != 0) {
			if (ret == DB_NOTFOUND) {
				ret = 0;
				break;
			}
			goto err;
		}
	}

err:	/* Discard the cursor. */
	if ((t_ret = dbc->c_close(dbc)) != 0 && ret == 0)
		ret = t_ret;

	return (ret);
}

/*
 * CDB___bam_ditem --
 *	Delete one or more entries from a page.
 *
 * PUBLIC: int CDB___bam_ditem __P((DBC *, PAGE *, u_int32_t));
 */
int
CDB___bam_ditem(dbc, h, indx)
	DBC *dbc;
	PAGE *h;
	u_int32_t indx;
{
	BINTERNAL *bi;
	BKEYDATA *bk;
	DB *dbp;
	u_int32_t nbytes;
	int ret;

	dbp = dbc->dbp;

	switch (TYPE(h)) {
	case P_IBTREE:
		bi = GET_BINTERNAL(h, indx);
		switch (B_TYPE(bi->type)) {
		case B_DUPLICATE:
		case B_KEYDATA:
			nbytes = BINTERNAL_SIZE(bi->len);
			break;
		case B_OVERFLOW:
			nbytes = BINTERNAL_SIZE(bi->len);
			if ((ret =
			    CDB___db_doff(dbc, ((BOVERFLOW *)bi->data)->pgno)) != 0)
				return (ret);
			break;
		default:
			return (CDB___db_pgfmt(dbp, PGNO(h)));
		}
		break;
	case P_IRECNO:
		nbytes = RINTERNAL_SIZE;
		break;
	case P_LBTREE:
		/*
		 * If it's a duplicate key, discard the index and don't touch
		 * the actual page item.
		 *
		 * !!!
		 * This works because no data item can have an index matching
		 * any other index so even if the data item is in a key "slot",
		 * it won't match any other index.
		 */
		if ((indx % 2) == 0) {
			/*
			 * Check for a duplicate after us on the page.  NOTE:
			 * we have to delete the key item before deleting the
			 * data item, otherwise the "indx + P_INDX" calculation
			 * won't work!
			 */
			if (indx + P_INDX < (u_int32_t)NUM_ENT(h) &&
			    h->inp[indx] == h->inp[indx + P_INDX])
				return (CDB___bam_adjindx(dbc,
				    h, indx, indx + O_INDX, 0));
			/*
			 * Check for a duplicate before us on the page.  It
			 * doesn't matter if we delete the key item before or
			 * after the data item for the purposes of this one.
			 */
			if (indx > 0 && h->inp[indx] == h->inp[indx - P_INDX])
				return (CDB___bam_adjindx(dbc,
				    h, indx, indx - P_INDX, 0));
		}
		/* FALLTHROUGH */
	case P_LDUP:
	case P_LRECNO:
		bk = GET_BKEYDATA(h, indx);
		switch (B_TYPE(bk->type)) {
		case B_DUPLICATE:
			nbytes = BOVERFLOW_SIZE;
			break;
		case B_OVERFLOW:
			nbytes = BOVERFLOW_SIZE;
			if ((ret = CDB___db_doff(
			    dbc, (GET_BOVERFLOW(h, indx))->pgno)) != 0)
				return (ret);
			break;
		case B_KEYDATA:
			nbytes = BKEYDATA_SIZE(bk->len);
			break;
		default:
			return (CDB___db_pgfmt(dbp, PGNO(h)));
		}
		break;
	default:
		return (CDB___db_pgfmt(dbp, PGNO(h)));
	}

	/* Delete the item and mark the page dirty. */
	if ((ret = CDB___db_ditem(dbc, h, indx, nbytes)) != 0)
		return (ret);
	if ((ret = CDB_memp_fset(dbp->mpf, h, DB_MPOOL_DIRTY)) != 0)
		return (ret);

	return (0);
}

/*
 * CDB___bam_adjindx --
 *	Adjust an index on the page.
 *
 * PUBLIC: int CDB___bam_adjindx __P((DBC *, PAGE *, u_int32_t, u_int32_t, int));
 */
int
CDB___bam_adjindx(dbc, h, indx, indx_copy, is_insert)
	DBC *dbc;
	PAGE *h;
	u_int32_t indx, indx_copy;
	int is_insert;
{
	DB *dbp;
	db_indx_t copy;
	int ret;

	dbp = dbc->dbp;

	/* Log the change. */
	if (DB_LOGGING(dbc) &&
	    (ret = CDB___bam_adj_log(dbp->dbenv, dbc->txn, &LSN(h),
	    0, dbp->log_fileid, PGNO(h), &LSN(h), indx, indx_copy,
	    (u_int32_t)is_insert)) != 0)
		return (ret);

	/* Shuffle the indices and mark the page dirty. */
	if (is_insert) {
		copy = h->inp[indx_copy];
		if (indx != NUM_ENT(h))
			memmove(&h->inp[indx + O_INDX], &h->inp[indx],
			    sizeof(db_indx_t) * (NUM_ENT(h) - indx));
		h->inp[indx] = copy;
		++NUM_ENT(h);
	} else {
		--NUM_ENT(h);
		if (indx != NUM_ENT(h))
			memmove(&h->inp[indx], &h->inp[indx + O_INDX],
			    sizeof(db_indx_t) * (NUM_ENT(h) - indx));
	}
	if ((ret = CDB_memp_fset(dbp->mpf, h, DB_MPOOL_DIRTY)) != 0)
		return (ret);

	return (0);
}

/*
 * CDB___bam_dpages --
 *	Delete a set of locked pages.
 *
 * PUBLIC: int CDB___bam_dpages __P((DBC *, EPG *));
 */
int
CDB___bam_dpages(dbc, stack_epg)
	DBC *dbc;
	EPG *stack_epg;
{
	BTREE_CURSOR *cp;
	DB *dbp;
	DBT a, b;
	DB_LOCK c_lock, p_lock;
	EPG *epg;
	PAGE *child, *parent;
	db_indx_t nitems;
	db_pgno_t pgno, root_pgno;
	db_recno_t rcnt;
	int done, ret, t_ret;

	dbp = dbc->dbp;
	cp = (BTREE_CURSOR *)dbc->internal;

	/*
	 * We have the entire stack of deletable pages locked.
	 *
	 * Btree calls us with a pointer to the beginning of a stack, where
	 * the first page in the stack is to have a single item deleted, and
	 * the rest of the pages are to be removed.
	 *
	 * Recno calls us with a pointer into the middle of the stack, where
	 * the referenced page is to have a single item deleted, and pages
	 * after the stack reference are to be removed.
	 *
	 * First, discard any pages that we don't care about.
	 */
	ret = 0;
	for (epg = cp->sp; epg < stack_epg; ++epg) {
		if ((t_ret =
		    CDB_memp_fput(dbp->mpf, epg->page, 0)) != 0 && ret == 0)
			ret = t_ret;
		(void)__TLPUT(dbc, epg->lock);
	}
	if (ret != 0)
		goto err;

	/*
	 * !!!
	 * There is an interesting deadlock situation here.  We have to relink
	 * the leaf page chain around the leaf page being deleted.  Consider
	 * a cursor walking through the leaf pages, that has the previous page
	 * read-locked and is waiting on a lock for the page we're deleting.
	 * It will deadlock here.  Before we unlink the subtree, we relink the
	 * leaf page chain.
	 */
	if ((ret = CDB___db_relink(dbc, DB_REM_PAGE, cp->csp->page, NULL, 1)) != 0)
		goto err;

	/*
	 * Delete the last item that references the underlying pages that are
	 * to be deleted, and adjust cursors that reference that page.  Then,
	 * save that page's page number and item count and release it.  If
	 * the application isn't retaining locks because it's running without
	 * transactions, this lets the rest of the tree get back to business
	 * immediately.
	 */
	if ((ret = CDB___bam_ditem(dbc, epg->page, epg->indx)) != 0)
		goto err;
	CDB___bam_ca_di(dbp, PGNO(epg->page), epg->indx, -1);

	pgno = PGNO(epg->page);
	nitems = NUM_ENT(epg->page);

	if ((ret = CDB_memp_fput(dbp->mpf, epg->page, 0)) != 0)
		goto err_inc;
	(void)__TLPUT(dbc, epg->lock);

	/* Free the rest of the pages in the stack. */
	while (++epg <= cp->csp) {
		/*
		 * Delete page entries so they will be restored as part of
		 * recovery.  We don't need to do cursor adjustment here as
		 * the pages are being emptied by definition and so cannot
		 * be referenced by a cursor.
		 */
		if (NUM_ENT(epg->page) != 0) {
			DB_ASSERT(NUM_ENT(epg->page) == 1);

			if ((ret = CDB___bam_ditem(dbc, epg->page, epg->indx)) != 0)
				goto err;
		}

		if ((ret = CDB___db_free(dbc, epg->page)) != 0)
			goto err_inc;
		(void)__TLPUT(dbc, epg->lock);
	}

	if (0) {
err_inc:	++epg;
err:		for (; epg <= cp->csp; ++epg) {
			(void)CDB_memp_fput(dbp->mpf, epg->page, 0);
			(void)__TLPUT(dbc, epg->lock);
		}
		BT_STK_CLR(cp);
		return (ret);
	}
	BT_STK_CLR(cp);

	/*
	 * If we just deleted the next-to-last item from the root page, the
	 * tree can collapse one or more levels.  While there remains only a
	 * single item on the root page, write lock the last page referenced
	 * by the root page and copy it over the root page.  If we can't get a
	 * write lock, that's okay, the tree just stays deeper than we'd like.
	 */
	root_pgno = cp->root;
	if (pgno != root_pgno || nitems != 1)
		return (0);

	for (done = 0; !done;) {
		/* Initialize. */
		parent = child = NULL;
		p_lock.off = c_lock.off = LOCK_INVALID;

		/* Lock the root. */
		pgno = root_pgno;
		if ((ret =
		    CDB___db_lget(dbc, 0, pgno, DB_LOCK_WRITE, 0, &p_lock)) != 0)
			goto stop;
		if ((ret = CDB_memp_fget(dbp->mpf, &pgno, 0, &parent)) != 0)
			goto stop;

		if (NUM_ENT(parent) != 1)
			goto stop;

		switch (TYPE(parent)) {
		case P_IBTREE:
			pgno = GET_BINTERNAL(parent, 0)->pgno;
			break;
		case P_IRECNO:
			pgno = GET_RINTERNAL(parent, 0)->pgno;
			break;
		default:
			goto stop;
		}

		/* Lock the child page. */
		if ((ret =
		    CDB___db_lget(dbc, 0, pgno, DB_LOCK_WRITE, 0, &c_lock)) != 0)
			goto stop;
		if ((ret = CDB_memp_fget(dbp->mpf, &pgno, 0, &child)) != 0)
			goto stop;

		/* Log the change. */
		if (DB_LOGGING(dbc)) {
			memset(&a, 0, sizeof(a));
			a.data = child;
			a.size = dbp->pgsize;
			memset(&b, 0, sizeof(b));
			b.data = P_ENTRY(parent, 0);
			b.size = BINTERNAL_SIZE(((BINTERNAL *)b.data)->len);
			CDB___bam_rsplit_log(dbp->dbenv, dbc->txn,
			   &child->lsn, 0, dbp->log_fileid, PGNO(child), &a,
			   PGNO(parent), RE_NREC(parent), &b, &parent->lsn);
		}

		/*
		 * Make the switch.
		 *
		 * One fixup -- internal pages below the top level do not store
		 * a record count, so we have to preserve it if we're not
		 * converting to a leaf page.  Note also that we are about to
		 * overwrite the parent page, including its LSN.  This is OK
		 * because the log message we wrote describing this update
		 * stores its LSN on the child page.  When the child is copied
		 * onto the parent, the correct LSN is copied into place.
		 */
		COMPQUIET(rcnt, 0);
		if (F_ISSET(cp, C_RECNUM) && LEVEL(child) > LEAFLEVEL)
			rcnt = RE_NREC(parent);
		memcpy(parent, child, dbp->pgsize);
		PGNO(parent) = root_pgno;
		if (F_ISSET(cp, C_RECNUM) && LEVEL(child) > LEAFLEVEL)
			RE_NREC_SET(parent, rcnt);

		/* Mark the pages dirty. */
		CDB_memp_fset(dbp->mpf, parent, DB_MPOOL_DIRTY);
		CDB_memp_fset(dbp->mpf, child, DB_MPOOL_DIRTY);

		/* Adjust the cursors. */
		CDB___bam_ca_rsplit(dbp, PGNO(child), root_pgno);

		/*
		 * Free the page copied onto the root page and discard its
		 * lock.  (The call to CDB___db_free() discards our reference
		 * to the page.)
		 */
		(void)CDB___db_free(dbc, child);
		child = NULL;

		if (0) {
stop:			done = 1;
		}
		if (p_lock.off != LOCK_INVALID)
			(void)__TLPUT(dbc, p_lock);
		if (parent != NULL)
			CDB_memp_fput(dbp->mpf, parent, 0);
		if (c_lock.off != LOCK_INVALID)
			(void)__TLPUT(dbc, c_lock);
		if (child != NULL)
			CDB_memp_fput(dbp->mpf, child, 0);
	}

	return (0);
}
