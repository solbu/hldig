/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997, 1998, 1999
 *  Sleepycat Software.  All rights reserved.
 */

#include "db_config.h"

#ifndef lint
static const char sccsid[] = "@(#)txn_region.c  11.4 (Sleepycat) 9/20/99";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

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

#include <string.h>
#endif

#include "db_int.h"
#include "db_page.h"
#include "txn.h"
#include "db_am.h"

static int CDB___txn_init __P ((DB_ENV *, DB_TXNMGR *));
static int CDB___txn_set_tx_max __P ((DB_ENV *, uint32_t));
static int CDB___txn_set_tx_recover
__P ((DB_ENV *, int (*)(DB_ENV *, DBT *, DB_LSN *, int, void *)));

/*
 * CDB___txn_dbenv_create --
 *  Transaction specific initialization of the DB_ENV structure.
 *
 * PUBLIC: void CDB___txn_dbenv_create __P((DB_ENV *));
 */
void
CDB___txn_dbenv_create (dbenv)
     DB_ENV *dbenv;
{
  dbenv->tx_max = DEF_MAX_TXNS;

  dbenv->set_tx_max = CDB___txn_set_tx_max;
  dbenv->set_tx_recover = CDB___txn_set_tx_recover;
}

/*
 * CDB___txn_set_tx_max --
 *  Set the size of the transaction table.
 */
static int
CDB___txn_set_tx_max (dbenv, tx_max)
     DB_ENV *dbenv;
     uint32_t tx_max;
{
  ENV_ILLEGAL_AFTER_OPEN (dbenv, "set_tx_max");

  dbenv->tx_max = tx_max;
  return (0);
}

/*
 * CDB___txn_set_tx_recover --
 *  Set the transaction abort recover function.
 */
static int
CDB___txn_set_tx_recover (dbenv, tx_recover)
     DB_ENV *dbenv;
     int (*tx_recover) __P ((DB_ENV *, DBT *, DB_LSN *, int, void *));
{
  dbenv->tx_recover = tx_recover;
  return (0);
}

/*
 * CDB___txn_open --
 *  Open a transaction region.
 *
 * PUBLIC: int CDB___txn_open __P((DB_ENV *));
 */
int
CDB___txn_open (dbenv)
     DB_ENV *dbenv;
{
  DB_TXNMGR *tmgrp;
  int ret;

  /* Create/initialize the transaction manager structure. */
  if ((ret = CDB___os_calloc (1, sizeof (DB_TXNMGR), &tmgrp)) != 0)
    return (ret);
  TAILQ_INIT (&tmgrp->txn_chain);
  tmgrp->dbenv = dbenv;
  tmgrp->recover =
    dbenv->tx_recover == NULL ? CDB___db_dispatch : dbenv->tx_recover;

  /* Join/create the txn region. */
  tmgrp->reginfo.id = REG_ID_TXN;
  tmgrp->reginfo.mode = dbenv->db_mode;
  if (F_ISSET (dbenv, DB_ENV_CREATE))
    F_SET (&tmgrp->reginfo, REGION_CREATE_OK);
  if ((ret = CDB___db_r_attach (dbenv,
                                &tmgrp->reginfo,
                                TXN_REGION_SIZE (dbenv->tx_max))) != 0)
    goto err;

  /* If we created the region, initialize it. */
  if (F_ISSET (&tmgrp->reginfo, REGION_CREATE))
    if ((ret = CDB___txn_init (dbenv, tmgrp)) != 0)
      goto err;

  /* Set the local addresses. */
  tmgrp->reginfo.primary =
    R_ADDR (&tmgrp->reginfo, tmgrp->reginfo.rp->primary);

  R_UNLOCK (dbenv, &tmgrp->reginfo);

  /* Acquire a mutex to protect the active TXN list. */
  if (F_ISSET (dbenv, DB_ENV_THREAD))
  {
    if ((ret =
         CDB___db_mutex_alloc (dbenv, &tmgrp->reginfo, &tmgrp->mutexp)) != 0)
      goto detach;
    if ((ret = __db_mutex_init (dbenv, tmgrp->mutexp, 0, MUTEX_THREAD)) != 0)
      goto detach;
  }

  dbenv->tx_handle = tmgrp;
  return (0);

err:if (tmgrp->reginfo.addr != NULL)
  {
    if (F_ISSET (&tmgrp->reginfo, REGION_CREATE))
      F_SET (tmgrp->reginfo.rp, REG_DEAD);
    R_UNLOCK (dbenv, &tmgrp->reginfo);

  detach:(void) CDB___db_r_detach (dbenv, &tmgrp->reginfo, 0);
  }
  CDB___os_free (tmgrp, sizeof (*tmgrp));
  return (ret);
}

/*
 * CDB___txn_init --
 *  Initialize a transaction region in shared memory.
 */
static int
CDB___txn_init (dbenv, tmgrp)
     DB_ENV *dbenv;
     DB_TXNMGR *tmgrp;
{
  DB_TXNREGION *region;
  int ret;

  if ((ret = CDB___db_shalloc (tmgrp->reginfo.addr,
                               sizeof (DB_TXNREGION), 0,
                               &tmgrp->reginfo.primary)) != 0)
    return (ret);
  tmgrp->reginfo.rp->primary =
    R_OFFSET (&tmgrp->reginfo, tmgrp->reginfo.primary);
  region = tmgrp->reginfo.primary;
  memset (region, 0, sizeof (*region));

  region->maxtxns = dbenv->tx_max;
  region->last_txnid = TXN_MINIMUM;
  ZERO_LSN (region->pending_ckp);
  ZERO_LSN (region->last_ckp);
  region->time_ckp = time (NULL);

  /*
   * XXX
   * If we ever do more types of locking and logging, this changes.
   */
  region->logtype = 0;
  region->locktype = 0;
  region->naborts = 0;
  region->ncommits = 0;
  region->nbegins = 0;
  region->nactive = 0;
  region->maxnactive = 0;

  SH_TAILQ_INIT (&region->active_txn);

  return (0);
}

/*
 * CDB___txn_close --
 *  Close a transaction region.
 *
 * PUBLIC: int CDB___txn_close __P((DB_ENV *));
 */
int
CDB___txn_close (dbenv)
     DB_ENV *dbenv;
{
  DB_TXN *txnp;
  DB_TXNMGR *tmgrp;
  int ret, t_ret;

  ret = 0;
  tmgrp = dbenv->tx_handle;

  /*
   * This function can only be called once per process (i.e., not
   * once per thread), so no synchronization is required.
   *
   * We would like to abort any running transactions, but the caller
   * is doing something wrong by calling close with active
   * transactions.  It's quite likely that this will fail because
   * recovery won't find open files.  If this happens, the right
   * solution is DB_RUNRECOVERY.  So, convert any failure messages
   * to that.
   */
  while ((txnp =
          TAILQ_FIRST (&tmgrp->txn_chain)) != TAILQ_END (&tmgrp->txn_chain))
    if ((t_ret = CDB_txn_abort (txnp)) != 0)
    {
      CDB___db_err (dbenv,
                    "Unable to abort transaction 0x%x: %s\n",
                    txnp->txnid, CDB_db_strerror (t_ret));
      CDB___txn_end (txnp, 0);
      if (ret == 0)
        ret = t_ret == 0 ? 0 : DB_RUNRECOVERY;
    }

  /* Flush the log. */
  if (F_ISSET (dbenv, DB_ENV_LOGGING) &&
      (t_ret = CDB_log_flush (dbenv, NULL)) != 0 && ret == 0)
    ret = t_ret;

  /* Discard the per-thread lock. */
  if (tmgrp->mutexp != NULL)
    CDB___db_mutex_free (dbenv, &tmgrp->reginfo, tmgrp->mutexp);

  /* Detach from the region. */
  if ((t_ret = CDB___db_r_detach (dbenv, &tmgrp->reginfo, 0)) != 0
      && ret == 0)
    ret = t_ret;

  CDB___os_free (tmgrp, sizeof (*tmgrp));
  return (ret);
}

int
CDB_txn_stat (dbenv, statp, db_malloc)
     DB_ENV *dbenv;
     DB_TXN_STAT **statp;
     void *(*db_malloc) __P ((size_t));
{
  DB_TXNMGR *mgr;
  DB_TXNREGION *region;
  DB_TXN_STAT *stats;
  TXN_DETAIL *txnp;
  size_t nbytes;
  uint32_t nactive, ndx;
  int ret, slop;

  PANIC_CHECK (dbenv);
  ENV_REQUIRES_CONFIG (dbenv, dbenv->tx_handle, DB_INIT_TXN);

  *statp = NULL;

  slop = 200;
  mgr = dbenv->tx_handle;
  region = mgr->reginfo.primary;

retry:R_LOCK (dbenv, &mgr->reginfo);
  nactive = region->nactive;
  R_UNLOCK (dbenv, &mgr->reginfo);

  /*
   * Allocate extra active structures to handle any transactions that
   * are created while we have the region unlocked.
   */
  nbytes = sizeof (DB_TXN_STAT) + sizeof (DB_TXN_ACTIVE) * (nactive + slop);
  if ((ret = CDB___os_malloc (nbytes, db_malloc, &stats)) != 0)
    return (ret);

  R_LOCK (dbenv, &mgr->reginfo);
  stats->st_last_txnid = region->last_txnid;
  stats->st_last_ckp = region->last_ckp;
  stats->st_maxtxns = region->maxtxns;
  stats->st_naborts = region->naborts;
  stats->st_nbegins = region->nbegins;
  stats->st_ncommits = region->ncommits;
  stats->st_pending_ckp = region->pending_ckp;
  stats->st_time_ckp = region->time_ckp;
  stats->st_nactive = region->nactive;
  if (stats->st_nactive > nactive + 200)
  {
    R_UNLOCK (dbenv, &mgr->reginfo);
    slop *= 2;
    goto retry;
  }
  stats->st_maxnactive = region->maxnactive;
  stats->st_txnarray = (DB_TXN_ACTIVE *) & stats[1];

  ndx = 0;
  for (txnp = SH_TAILQ_FIRST (&region->active_txn, __txn_detail);
       txnp != NULL; txnp = SH_TAILQ_NEXT (txnp, links, __txn_detail))
  {
    stats->st_txnarray[ndx].txnid = txnp->txnid;
    if (txnp->parent == INVALID_ROFF)
      stats->st_txnarray[ndx].parentid = TXN_INVALID_ID;
    else
      stats->st_txnarray[ndx].parentid =
        ((TXN_DETAIL *) R_ADDR (&mgr->reginfo, txnp->parent))->txnid;
    stats->st_txnarray[ndx].lsn = txnp->begin_lsn;
    ndx++;

    if (ndx >= stats->st_nactive)
      break;
  }

  stats->st_region_wait = mgr->reginfo.rp->mutex.mutex_set_wait;
  stats->st_region_nowait = mgr->reginfo.rp->mutex.mutex_set_nowait;
  stats->st_regsize = mgr->reginfo.rp->size;

  R_UNLOCK (dbenv, &mgr->reginfo);

  *statp = stats;
  return (0);
}
