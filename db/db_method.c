/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1999
 *  Sleepycat Software.  All rights reserved.
 */

#include "db_config.h"

#ifndef lint
static const char sccsid[] = "@(#)db_method.c  11.8 (Sleepycat) 9/22/99";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <string.h>
#endif

#include "db_int.h"
#include "db_page.h"
#include "db_am.h"
#include "btree.h"
#include "hash.h"
#include "qam.h"
#include "xa.h"
#include "xa_ext.h"

static int CDB___db_get_byteswapped __P ((DB *));
static DBTYPE CDB___db_get_type __P ((DB *));
static int CDB___db_init __P ((DB *, uint32_t));
static int CDB___db_set_cachesize __P ((DB *, uint32_t, uint32_t, int));
static int CDB___db_set_dup_compare
__P ((DB *, int (*)(const DBT *, const DBT *)));
static void CDB___db_set_errcall __P ((DB *, void (*)(const char *, char *)));
static void CDB___db_set_errfile __P ((DB *, FILE *));
static void CDB___db_set_feedback __P ((DB *, void (*)(DB *, int, int)));
static int CDB___db_set_flags __P ((DB *, uint32_t));
static int CDB___db_set_lorder __P ((DB *, int));
static int CDB___db_set_malloc __P ((DB *, void *(*)(size_t)));
static int CDB___db_set_pagesize __P ((DB *, uint32_t));
static int CDB___db_set_realloc __P ((DB *, void *(*)(void *, size_t)));
static void CDB___db_set_errpfx __P ((DB *, const char *));
static void CDB___db_set_paniccall __P ((DB *, void (*)(DB_ENV *, int)));
static void CDB___dbh_err __P ((DB *, int, const char *, ...));
static void CDB___dbh_errx __P ((DB *, const char *, ...));

/*
 * CDB_db_create --
 *  DB constructor.
 */
int
CDB_db_create (dbpp, dbenv, flags)
     DB **dbpp;
     DB_ENV *dbenv;
     uint32_t flags;
{
  DB *dbp;
  int ret;

  /* Check for invalid function flags. */
  switch (flags)
  {
  case 0:
    break;
  case DB_XA_CREATE:
    if (dbenv != NULL)
    {
      CDB___db_err (dbenv,
                    "XA applications may not specify an environment to CDB_db_create");
      return (EINVAL);
    }

    /*
     * If it's an XA database, open it within the XA environment,
     * taken from the global list of environments.  (When the XA
     * transaction manager called our xa_start() routine the
     * "current" environment was moved to the start of the list.
     */
    dbenv = TAILQ_FIRST (&DB_GLOBAL (db_envq));
    break;
  default:
    return (CDB___db_ferr (dbenv, "CDB_db_create", 0));
  }

  /* Allocate the DB. */
  if ((ret = CDB___os_calloc (1, sizeof (*dbp), &dbp)) != 0)
    return (ret);
  if ((ret = CDB___db_init (dbp, flags)) != 0)
  {
    CDB___os_free (dbp, sizeof (*dbp));
    return (ret);
  }

  /* If we don't have an environment yet, allocate a local one. */
  if (dbenv == NULL)
  {
    if ((ret = CDB_db_env_create (&dbenv, 0)) != 0)
    {
      CDB___os_free (dbp, sizeof (*dbp));
      return (ret);
    }
    F_SET (dbenv, DB_ENV_DBLOCAL);
  }

  dbp->dbenv = dbenv;

  *dbpp = dbp;
  return (0);
}

/*
 * CDB___db_init --
 *  Initialize a DB structure.
 */
static int
CDB___db_init (dbp, flags)
     DB *dbp;
     uint32_t flags;
{
  int ret;

  dbp->pgsize = DB_DEF_IOSIZE;

  dbp->log_fileid = DB_LOGFILEID_INVALID;

  TAILQ_INIT (&dbp->free_queue);
  TAILQ_INIT (&dbp->active_queue);

  FLD_SET (dbp->am_ok, DB_OK_BTREE | DB_OK_HASH | DB_OK_QUEUE | DB_OK_RECNO);

  dbp->close = CDB___db_close;
  dbp->cursor = CDB___db_cursor;
  dbp->del = NULL;              /* !!! Must be set by access method. */
  dbp->err = CDB___dbh_err;
  dbp->errx = CDB___dbh_errx;
  dbp->fd = CDB___db_fd;
  dbp->get = CDB___db_get;
  dbp->get_byteswapped = CDB___db_get_byteswapped;
  dbp->get_type = CDB___db_get_type;
  dbp->join = CDB___db_join;
  dbp->open = CDB___db_open;
  dbp->put = CDB___db_put;
  dbp->remove = CDB___db_remove;
  dbp->set_cachesize = CDB___db_set_cachesize;
  dbp->set_dup_compare = CDB___db_set_dup_compare;
  dbp->set_errcall = CDB___db_set_errcall;
  dbp->set_errfile = CDB___db_set_errfile;
  dbp->set_errpfx = CDB___db_set_errpfx;
  dbp->set_feedback = CDB___db_set_feedback;
  dbp->set_flags = CDB___db_set_flags;
  dbp->set_lorder = CDB___db_set_lorder;
  dbp->set_malloc = CDB___db_set_malloc;
  dbp->set_pagesize = CDB___db_set_pagesize;
  dbp->set_paniccall = CDB___db_set_paniccall;
  dbp->set_realloc = CDB___db_set_realloc;
  dbp->stat = NULL;             /* !!! Must be set by access method. */
  dbp->sync = CDB___db_sync;
  dbp->upgrade = CDB___db_upgrade;
  /* Access method specific. */
  if ((ret = CDB___bam_db_create (dbp)) != 0)
    return (ret);
  if ((ret = CDB___ham_db_create (dbp)) != 0)
    return (ret);
  if ((ret = CDB___qam_db_create (dbp)) != 0)
    return (ret);

  /*
   * XA specific: must be last, as we replace methods set by the
   * access methods.
   */
  if (LF_ISSET (DB_XA_CREATE) && (ret = CDB___db_xa_create (dbp)) != 0)
    return (ret);

  F_SET (dbp, DB_AM_PGDEF);

  return (0);
}

/*
 * CDB___dbh_am_chk --
 *  Error if an unreasonable method is called.
 *
 * PUBLIC: int CDB___dbh_am_chk __P((DB *, uint32_t));
 */
int
CDB___dbh_am_chk (dbp, flags)
     DB *dbp;
     uint32_t flags;
{
  /*
   * We start out allowing any access methods to be called, and as the
   * application calls the methods the options become restricted.  The
   * idea is to quit as soon as an illegal method combination is called.
   */
  if ((LF_ISSET (DB_OK_BTREE) && FLD_ISSET (dbp->am_ok, DB_OK_BTREE)) ||
      (LF_ISSET (DB_OK_HASH) && FLD_ISSET (dbp->am_ok, DB_OK_HASH)) ||
      (LF_ISSET (DB_OK_QUEUE) && FLD_ISSET (dbp->am_ok, DB_OK_QUEUE)) ||
      (LF_ISSET (DB_OK_RECNO) && FLD_ISSET (dbp->am_ok, DB_OK_RECNO)))
  {
    FLD_CLR (dbp->am_ok, ~flags);
    return (0);
  }

  CDB___db_err (dbp->dbenv,
                "call implies an access method which is inconsistent with previous calls");
  return (EINVAL);
}

/*
 * CDB___dbh_err --
 *  Error message, including the standard error string.
 */
static void
#if defined(__STDC__) || defined(_MSC_VER)      /* WIN32 */
CDB___dbh_err (DB * dbp, int error, const char *fmt, ...)
#else
CDB___dbh_err (dbp, error, fmt, va_alist)
     DB *dbp;
     int error;
     const char *fmt;
     va_dcl
#endif
{
  va_list ap;

#if defined(__STDC__) || defined(_MSC_VER)      /* WIN32 */
  va_start (ap, fmt);
#else
  va_start (ap);
#endif
  CDB___db_real_err (dbp->dbenv, error, 1, 1, fmt, ap);

  va_end (ap);
}

/*
 * CDB___dbh_errx --
 *  Error message.
 */
static void
#if defined(__STDC__) || defined(_MSC_VER)      /* WIN32 */
CDB___dbh_errx (DB * dbp, const char *fmt, ...)
#else
CDB___dbh_errx (dbp, fmt, va_alist)
     DB *dbp;
     const char *fmt;
     va_dcl
#endif
{
  va_list ap;

#if defined(__STDC__) || defined(_MSC_VER)      /* WIN32 */
  va_start (ap, fmt);
#else
  va_start (ap);
#endif
  CDB___db_real_err (dbp->dbenv, 0, 0, 1, fmt, ap);

  va_end (ap);
}

/*
 * CDB___db_get_byteswapped --
 *  Return if database requires byte swapping.
 */
static int
CDB___db_get_byteswapped (dbp)
     DB *dbp;
{
  DB_ILLEGAL_BEFORE_OPEN (dbp, "get_byteswapped");

  return (F_ISSET (dbp, DB_AM_SWAP) ? 1 : 0);
}

/*
 * CDB___db_get_type --
 *  Return type of underlying database.
 */
static DBTYPE
CDB___db_get_type (dbp)
     DB *dbp;
{
  DB_ILLEGAL_BEFORE_OPEN (dbp, "get_type");

  return (dbp->type);
}

/*
 * CDB___db_set_cachesize --
 *  Set underlying cache size.
 */
static int
CDB___db_set_cachesize (dbp, cache_gbytes, cache_bytes, ncache)
     DB *dbp;
     uint32_t cache_gbytes, cache_bytes;
     int ncache;
{
  DB_ILLEGAL_IN_ENV (dbp, "set_cachesize");
  DB_ILLEGAL_AFTER_OPEN (dbp, "set_cachesize");

  return (dbp->dbenv->
          set_cachesize (dbp->dbenv, cache_gbytes, cache_bytes, ncache));
}

/*
 * CDB___db_set_dup_compare --
 *  Set duplicate comparison routine.
 */
static int
CDB___db_set_dup_compare (dbp, func)
     DB *dbp;
     int (*func) __P ((const DBT *, const DBT *));
{
  DB_ILLEGAL_AFTER_OPEN (dbp, "dup_compare");
  DB_ILLEGAL_METHOD (dbp, DB_OK_BTREE | DB_OK_HASH);

  dbp->dup_compare = func;

  return (0);
}

static void
CDB___db_set_errcall (dbp, errcall)
     DB *dbp;
     void (*errcall) __P ((const char *, char *));
{
  dbp->dbenv->set_errcall (dbp->dbenv, errcall);
}

static void
CDB___db_set_errfile (dbp, errfile)
     DB *dbp;
     FILE *errfile;
{
  dbp->dbenv->set_errfile (dbp->dbenv, errfile);
}

static void
CDB___db_set_errpfx (dbp, errpfx)
     DB *dbp;
     const char *errpfx;
{
  dbp->dbenv->set_errpfx (dbp->dbenv, errpfx);
}

static void
CDB___db_set_feedback (dbp, feedback)
     DB *dbp;
     void (*feedback) __P ((DB *, int, int));
{
  dbp->db_feedback = feedback;
}

static int
CDB___db_set_flags (dbp, flags)
     DB *dbp;
     uint32_t flags;
{
  int ret;

  /*
   * !!!
   * The hash access method only takes two flags: DB_DUP and DB_DUPSORT.
   * The Btree access method uses them for the same purposes, and so we
   * resolve them there.
   *
   * The queue access method takes no flags.
   */
  if ((ret = CDB___bam_set_flags (dbp, &flags)) != 0)
    return (ret);
  if ((ret = CDB___ram_set_flags (dbp, &flags)) != 0)
    return (ret);

  return (flags == 0 ? 0 : CDB___db_ferr (dbp->dbenv, "DB->set_flags", 0));
}

static int
CDB___db_set_lorder (dbp, db_lorder)
     DB *dbp;
     int db_lorder;
{
  int ret;

  DB_ILLEGAL_AFTER_OPEN (dbp, "set_lorder");

  /* Flag if the specified byte order requires swapping. */
  switch (ret = CDB___db_byteorder (dbp->dbenv, db_lorder))
  {
  case 0:
    F_CLR (dbp, DB_AM_SWAP);
    break;
  case DB_SWAPBYTES:
    F_SET (dbp, DB_AM_SWAP);
    break;
  default:
    return (ret);
    /* NOTREACHED */
  }
  return (0);
}

static int
CDB___db_set_malloc (dbp, func)
     DB *dbp;
     void *(*func) __P ((size_t));
{
  DB_ILLEGAL_AFTER_OPEN (dbp, "set_malloc");

  dbp->db_malloc = func;
  return (0);
}

static int
CDB___db_set_pagesize (dbp, db_pagesize)
     DB *dbp;
     uint32_t db_pagesize;
{
  DB_ILLEGAL_AFTER_OPEN (dbp, "set_pagesize");

  if (db_pagesize < DB_MIN_PGSIZE)
  {
    CDB___db_err (dbp->dbenv, "page sizes may not be smaller than %lu",
                  (u_long) DB_MIN_PGSIZE);
    return (EINVAL);
  }
  if (db_pagesize > DB_MAX_PGSIZE)
  {
    CDB___db_err (dbp->dbenv, "page sizes may not be larger than %lu",
                  (u_long) DB_MAX_PGSIZE);
    return (EINVAL);
  }

  /*
   * We don't want anything that's not a power-of-2, as we rely on that
   * for alignment of various types on the pages.
   */
  if ((uint32_t) 1 << CDB___db_log2 (db_pagesize) != db_pagesize)
  {
    CDB___db_err (dbp->dbenv, "page sizes must be a power-of-2");
    return (EINVAL);
  }

  /*
   * XXX
   * Should we be checking for a page size that's not a multiple of 512,
   * so that we never try and write less than a disk sector?
   */

  F_CLR (dbp, DB_AM_PGDEF);
  dbp->pgsize = db_pagesize;

  return (0);
}

static int
CDB___db_set_realloc (dbp, func)
     DB *dbp;
     void *(*func) __P ((void *, size_t));
{
  DB_ILLEGAL_AFTER_OPEN (dbp, "set_realloc");

  dbp->db_realloc = func;
  return (0);
}

static void
CDB___db_set_paniccall (dbp, paniccall)
     DB *dbp;
     void (*paniccall) __P ((DB_ENV *, int));
{
  dbp->dbenv->set_paniccall (dbp->dbenv, paniccall);
}
