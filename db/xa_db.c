/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1998, 1999
 *  Sleepycat Software.  All rights reserved.
 */

#include "db_config.h"

#ifndef lint
static const char sccsid[] = "@(#)xa_db.c  11.4 (Sleepycat) 9/15/99";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>
#endif

#include "db_int.h"
#include "xa.h"
#include "xa_ext.h"

static int CDB___xa_close __P ((DB *, uint32_t));
static int CDB___xa_cursor __P ((DB *, DB_TXN *, DBC **, uint32_t));
static int CDB___xa_del __P ((DB *, DB_TXN *, DBT *, uint32_t));
static int CDB___xa_get __P ((DB *, DB_TXN *, DBT *, DBT *, uint32_t));
static int CDB___xa_put __P ((DB *, DB_TXN *, DBT *, DBT *, uint32_t));

typedef struct __xa_methods
{
  int (*close) __P ((DB *, uint32_t));
  int (*cursor) __P ((DB *, DB_TXN *, DBC **, uint32_t));
  int (*del) __P ((DB *, DB_TXN *, DBT *, uint32_t));
  int (*get) __P ((DB *, DB_TXN *, DBT *, DBT *, uint32_t));
  int (*put) __P ((DB *, DB_TXN *, DBT *, DBT *, uint32_t));
} XA_METHODS;

/*
 * CDB___db_xa_create --
 *  DB XA constructor.
 *
 * PUBLIC: int CDB___db_xa_create __P((DB *));
 */
int
CDB___db_xa_create (dbp)
     DB *dbp;
{
  XA_METHODS *xam;
  int ret;

  /*
   * Interpose XA routines in front of any method that takes a TXN
   * ID as an argument.
   */
  if ((ret = CDB___os_calloc (1, sizeof (XA_METHODS), &xam)) != 0)
    return (ret);

  dbp->xa_internal = xam;
  xam->close = dbp->close;
  xam->cursor = dbp->cursor;
  xam->del = dbp->del;
  xam->get = dbp->get;
  xam->put = dbp->put;
  dbp->close = CDB___xa_close;
  dbp->cursor = CDB___xa_cursor;
  dbp->del = CDB___xa_del;
  dbp->get = CDB___xa_get;
  dbp->put = CDB___xa_put;

  return (0);
}

static int
CDB___xa_cursor (dbp, txn, dbcp, flags)
     DB *dbp;
     DB_TXN *txn;
     DBC **dbcp;
     uint32_t flags;
{
  DB_TXN *t;

  t = txn != NULL && txn == dbp->open_txn ? txn : dbp->dbenv->xa_txn;

  return (((XA_METHODS *) dbp->xa_internal)->cursor (dbp, t, dbcp, flags));
}

static int
CDB___xa_del (dbp, txn, key, flags)
     DB *dbp;
     DB_TXN *txn;
     DBT *key;
     uint32_t flags;
{
  DB_TXN *t;

  t = txn != NULL && txn == dbp->open_txn ? txn : dbp->dbenv->xa_txn;

  return (((XA_METHODS *) dbp->xa_internal)->del (dbp, t, key, flags));
}

static int
CDB___xa_close (dbp, flags)
     DB *dbp;
     uint32_t flags;
{
  int (*real_close) __P ((DB *, uint32_t));

  real_close = ((XA_METHODS *) dbp->xa_internal)->close;

  CDB___os_free (dbp->xa_internal, sizeof (XA_METHODS));
  dbp->xa_internal = NULL;

  return (real_close (dbp, flags));
}

static int
CDB___xa_get (dbp, txn, key, data, flags)
     DB *dbp;
     DB_TXN *txn;
     DBT *key, *data;
     uint32_t flags;
{
  DB_TXN *t;

  t = txn != NULL && txn == dbp->open_txn ? txn : dbp->dbenv->xa_txn;

  return (((XA_METHODS *) dbp->xa_internal)->get (dbp, t, key, data, flags));
}

static int
CDB___xa_put (dbp, txn, key, data, flags)
     DB *dbp;
     DB_TXN *txn;
     DBT *key, *data;
     uint32_t flags;
{
  DB_TXN *t;

  t = txn != NULL && txn == dbp->open_txn ? txn : dbp->dbenv->xa_txn;

  return (((XA_METHODS *) dbp->xa_internal)->put (dbp, t, key, data, flags));
}
