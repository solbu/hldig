/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997, 1998, 1999
 *	Sleepycat Software.  All rights reserved.
 */
#include "db_config.h"

#ifndef lint
static const char sccsid[] = "@(#)mp_alloc.c	11.3 (Sleepycat) 9/29/99";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>
#include <errno.h>
#endif

#include "db_int.h"
#include "db_shash.h"
#include "mp.h"

#if 0
#  define DEBUG
#endif

/*
 * CDB___memp_clean_page  allocates memory from the memory pool.  If mfp
 * is non-NULL, its page size specifies the amount of memory to allocate.
 * This memory is essentially a disk cache, and it seems pages can be freed
 * at will (after writing to disk, if the page is dirty).
 *
 * On-the-fly compression (CDB___memp_cmpr...) causes problems when writing
 * dirty pages.  CDB___memp_cmpr_alloc_chain  calls  CDB___memp_alloc,
 * potentially causing infinite recursion.  This should eventually be fixed
 * in  CDB___memp_cmpr_alloc_chain but a hack has been put here for now.
 * An explicit count of the recursion level is kept.  In the outer call,
 * any page can be swapped out, whether dirty or not.  During a recursive call,
 * only clean pages are swapped out.  To ensure that there are sufficient
 * clean pages,  CDB___memp_clean_page  is called whenever the number of
 * dirt pages exceed  CDB___mp_dirty_level  times the number of clean pages.
 */

static int CDB___memp_clean_page __P((DB_MPOOL *, REGINFO *));

/* Factor by which count of dirty pages must exceed clean pages before */
/* cache is forced to flush */
int CDB___mp_dirty_level = 3000;

/*
 * CDB___memp_alloc --
 *	Allocate some space in the mpool region.
 *
 * PUBLIC: int CDB___memp_alloc __P((DB_MPOOL *,
 * PUBLIC:     REGINFO *, MPOOLFILE *, size_t, roff_t *, void *));
 */
int
CDB___memp_alloc(dbmp, memreg, mfp, len, offsetp, retp)
	DB_MPOOL *dbmp;
	REGINFO *memreg;
	MPOOLFILE *mfp;
	size_t len;
	roff_t *offsetp;
	void *retp;
{
	BH *bhp, *nbhp;
	MCACHE *mc;
	MPOOL *mp;
	MPOOLFILE *bh_mfp;
	u_int32_t failed_writes, pages_reviewed;
	size_t total;
	int nomore, restart, ret, wrote;
	void *p;


	dbmp->recursion_level++;
	if (dbmp->recursion_level > 2)	/* bhwrite (used by us) calls us. */
	{				/* Disallow deeper recursions. */
	    fprintf (stderr,
		"Can't allocate %lu bytes: Recursive call (mfp %p)\n",
		(u_long)len, mfp);
#ifdef DEBUG
	    {
	    static int flag = 1;
	    if (flag)
	    {
		flag = 0;
		/* Obtain a backtrace and print it to stderr. */
		{
		  void *array[100];
		  size_t size;
		  char **strings;
		  size_t i;

		  size = backtrace (array, 100);
		  strings = backtrace_symbols (array, size);

		  printf ("Obtained %zd stack frames.\n", size);

		  for (i = 0; i < size; i++)
		     fprintf (stderr, "%s\n", strings[i]);

		  free (strings);
		}
	    }
	    }
#endif
	    ret = ENOMEM;
	    goto err;
	}

	mp = dbmp->reginfo.primary;

	/* This next statement is dodgy.  The memory allocation in which */
	/* memreg->primary is located is not (quite) large enough. */
	/* The same void pointer is used in  CDB___memp_open() to point to */
	/* an MPOOL, rather than an MCACHE as it does here. */
	mc = memreg->primary;
	failed_writes = 0;

#ifdef DEBUG
#  ifndef HEAVY_DEBUG
	if (dbmp->recursion_level > 1)
#  endif
	    fprintf(stderr,"Depth %d D %d C %d\n", dbmp->recursion_level,
			mc->stat.st_page_dirty, mc->stat.st_page_clean);
#endif

	/*
	 * If we're allocating a buffer, and the one we're discarding is the
	 * same size, we don't want to waste the time to re-integrate it into
	 * the shared memory free list.  If the DB_MPOOLFILE argument isn't
	 * NULL, we'll compare the underlying page sizes of the two buffers
	 * before free-ing and re-allocating buffers.
	 */
	if (mfp != NULL)
		len = (sizeof(BH) - sizeof(u_int8_t)) + mfp->stat.st_pagesize;

	nomore = 0;
alloc:	if ((ret = CDB___db_shalloc(memreg->addr, len, MUTEX_ALIGN, &p)) == 0) {
		if (offsetp != NULL)
			*offsetp = R_OFFSET(memreg, p);
		*(void **)retp = p;
#ifdef HEAVY_DEBUG
		fprintf(stderr,"D %d C %d\n",
			mc->stat.st_page_dirty, mc->stat.st_page_clean);
#endif
		if (mc->stat.st_page_dirty/CDB___mp_dirty_level > mc->stat.st_page_clean) {
		    /* if too many pages dirty, write some out */
		    /* since "write" with compression on can itself */
		    /* need to allocate memory... */
		    CDB___memp_clean_page(dbmp, memreg);
		}
		ret = 0;
		goto err;
	}
	if (nomore) {
	    CDB___db_err(dbmp->dbenv,
		"Unable to allocate %lu bytes from mpool shared region: %s\n",
		(u_long)len, CDB_db_strerror(ret));
	    if (CDB___mp_dirty_level > 1)
	        fprintf(stderr,
	            "Try setting  wordlist_cache_dirty_level=%d  in configuration file\n",
	            CDB___mp_dirty_level > 40 ? 40 : CDB___mp_dirty_level/2);
	    goto err;
	}

retry:	/* Find a buffer we can flush; LRU, skipping unwritable dirty pages */
	restart = 0;
	total = 0;
	pages_reviewed = 0;
#ifdef DEBUG
	if (dbmp->recursion_level > 1)
	    fprintf(stderr,"Starting loop\n");
#endif
	for (bhp =
	    SH_TAILQ_FIRST(&mc->bhq, __bh); bhp != NULL; bhp = nbhp) {
		nbhp = SH_TAILQ_NEXT(bhp, q, __bh);

		++pages_reviewed;
#ifdef DEBUG
		if (dbmp->recursion_level > 1)
		    fprintf(stderr,"Reviewed %d D %d C %d T %d\n",
			pages_reviewed,
			mc->stat.st_page_dirty, mc->stat.st_page_clean, total);
#endif

		/* Ignore pinned or locked (I/O in progress) buffers. */
		if (bhp->ref != 0 || F_ISSET(bhp, BH_LOCKED))
			continue;

		/* Find the associated MPOOLFILE. */
		bh_mfp = R_ADDR(&dbmp->reginfo, bhp->mf_offset);

		/* Write the page if it's dirty. */
		if (F_ISSET(bhp, BH_DIRTY)) {
		        /* If called from within  CDB___memp_cmpr_write()... */
		        if (dbmp->recursion_level > 1)
			    continue;

			++bhp->ref;
			ret = CDB___memp_bhwrite(dbmp,
			    bh_mfp, bhp, &restart, &wrote);
			--bhp->ref;

			if (ret != 0) {
				/*
				 * Count the number of writes that have
				 * failed.  If the number of writes that
				 * have failed, total, plus the number
				 * of pages we've reviewed on this pass
				 * equals the number of buffers there
				 * currently are, we've most likely
				 * run out of buffers that are going to
				 * succeed, and it's time to fail.
				 * (We chuck failing buffers to the
				 * end of the list.) [#0637]
				 */
				failed_writes++;
#ifdef DEBUG
				fprintf(stderr,"D %d C %d R %d F %d\n",
					mc->stat.st_page_dirty,
					mc->stat.st_page_clean,
					pages_reviewed, failed_writes);
#endif
				if (failed_writes + pages_reviewed >=
					mc->stat.st_page_dirty +
					mc->stat.st_page_clean)
				    goto err;

				/*
				 * Otherwise, relocate this buffer
				 * to the end of the LRU queue
				 * so we're less likely to encounter
				 * it again, and try again.
				 */
				SH_TAILQ_REMOVE(&mc->bhq, bhp, q, __bh);
				SH_TAILQ_INSERT_TAIL(&mc->bhq, bhp, q);
#ifdef DEBUG
				fprintf(stderr,"Retrying...\n");
#endif
				goto retry;
			}

			/*
			 * Another process may have acquired this buffer and
			 * incremented the ref count after we wrote it.
			 */
			if (bhp->ref != 0)
				goto retry;

			/*
			 * If we wrote the page, continue and free the buffer.
			 * We don't have to rewalk the list to acquire the
			 * buffer because it was never available for any other
			 * process to modify it.
			 *
			 * If we didn't write the page, but we discarded and
			 * reacquired the region lock, restart the list walk.
			 *
			 * If we neither wrote the buffer nor discarded the
			 * region lock, continue down the buffer list.
			 */
			if (wrote)
				++mc->stat.st_rw_evict;
			else {
				if (restart)
				{
#ifdef DEBUG
				        fprintf(stderr,"Restarting\n");
#endif
					goto retry;
				}
				continue;
			}
		} else
			++mc->stat.st_ro_evict;

		/*
		 * Check to see if the buffer is the size we're looking for.
		 * If it is, simply reuse it.
		 */
		if (mfp != NULL &&
		    mfp->stat.st_pagesize == bh_mfp->stat.st_pagesize) {
			CDB___memp_bhfree(dbmp, bhp, 0);

			if (offsetp != NULL)
				*offsetp = R_OFFSET(memreg, bhp);
			*(void **)retp = bhp;

#ifdef HEAVY_DEBUG
			fprintf(stderr,"d %d c %d\n",
				mc->stat.st_page_dirty, mc->stat.st_page_clean);
#endif
			if (mc->stat.st_page_dirty/CDB___mp_dirty_level > mc->stat.st_page_clean) {
			    /* if too many pages dirty, write some out */
			    /* since "write" with compression on can itself */
			    /* need to allocate memory... */
			    CDB___memp_clean_page(dbmp, memreg);
			}

			ret = 0;
			goto err;
		}

		/* Note how much space we've freed, and free the buffer. */
		total += CDB___db_shsizeof(bhp);
		CDB___memp_bhfree(dbmp, bhp, 1);

#ifdef DEBUG
		if (dbmp->recursion_level > 1)
		    fprintf(stderr,"Free %d  restart %d\n", total, restart);
#endif

		/*
		 * Retry as soon as we've freed up sufficient space.  If we
		 * have to coalesce of memory to satisfy the request, don't
		 * try until it's likely (possible?) that we'll succeed.
		 */
		if (total >= 3 * len)
			goto alloc;

		/* Restart the walk if we discarded the region lock. */
		if (restart)
			goto retry;
	}
	nomore = 1;
	goto alloc;
err:
	dbmp->recursion_level--;
	return (ret);
}


/*
 * CDB___memp_clean_page --
 *	Find dirty pages in MPOOL region and flush them.  Doesn't flush all,
 *	but ensures sufficient clean pages that compressed writes will not
 *	be stranded if the compression process needs to allocate memory.
 *
 *	I think  CDB_memp_trickle()  was intended for this purpose, but it
 *	seems not to be in use.  See also  CDB_memp_sync().
 *
 * PRIVATE: int CDB___memp_clean_page __P((DB_MPOOL *, REGINFO *, MPOOLFILE *));
 */
int
CDB___memp_clean_page(dbmp, memreg)
	DB_MPOOL *dbmp;
	REGINFO *memreg;
{
	BH *bhp, *nbhp;
	MCACHE *mc;
	MPOOLFILE *bh_mfp;
	int restart, cleaned, wrote;

	mc = memreg->primary;
	cleaned = 0;
#ifdef DEBUG
	fprintf(stderr,"Trying to clean %d pages (%d already clean)\n",
		mc->stat.st_page_dirty, mc->stat.st_page_clean);
#endif

	for (bhp = SH_TAILQ_FIRST(&mc->bhq, __bh); bhp != NULL; bhp = nbhp) {

	    nbhp = SH_TAILQ_NEXT(bhp, q, __bh);

	    /* Ignore pinned or locked (I/O in progress) buffers. */
	    if (bhp->ref != 0 || F_ISSET(bhp, BH_LOCKED))
		    continue;

	    /* Find the associated MPOOLFILE. */
	    bh_mfp = R_ADDR(&dbmp->reginfo, bhp->mf_offset);

	    /* Only write the page if it's dirty. */
	    if (!F_ISSET(bhp, BH_DIRTY))
		continue;

	    ++bhp->ref;		/* are these needed? I just copied from above!*/
	    if (!CDB___memp_bhwrite(dbmp, bh_mfp, bhp, &restart, &wrote))
		cleaned++;
	    --bhp->ref;
    }
#ifdef DEBUG
	fprintf(stderr,"Cleaned %d\n", cleaned);
#endif
    return (0);
}
