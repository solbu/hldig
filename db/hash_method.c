/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1999
 *  Sleepycat Software.  All rights reserved.
 */

#include "db_config.h"

#ifndef lint
static const char sccsid[] = "@(#)hash_method.c  11.3 (Sleepycat) 9/29/99";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>
#endif

#include "db_int.h"
#include "db_page.h"
#include "hash.h"

static int CDB___ham_set_h_ffactor __P ((DB *, uint32_t));
static int CDB___ham_set_h_hash
__P ((DB *, uint32_t (*)(const void *, uint32_t)));
static int CDB___ham_set_h_nelem __P ((DB *, uint32_t));

/*
 * CDB___ham_db_create --
 *  Hash specific initialization of the DB structure.
 *
 * PUBLIC: int CDB___ham_db_create __P((DB *));
 */
int
CDB___ham_db_create (dbp)
     DB *dbp;
{
  HASH *hashp;
  int ret;

  if ((ret = CDB___os_malloc (sizeof (HASH), NULL, &dbp->h_internal)) != 0)
    return (ret);

  hashp = dbp->h_internal;

  hashp->h_nelem = 0;           /* Defaults. */
  hashp->h_ffactor = 0;
  hashp->h_hash = NULL;

  dbp->set_h_ffactor = CDB___ham_set_h_ffactor;
  dbp->set_h_hash = CDB___ham_set_h_hash;
  dbp->set_h_nelem = CDB___ham_set_h_nelem;

  return (0);
}

/*
 * PUBLIC: int CDB___ham_db_close __P((DB *));
 */
int
CDB___ham_db_close (dbp)
     DB *dbp;
{
  if (dbp->h_internal == NULL)
    return (0);
  CDB___os_free (dbp->h_internal, sizeof (HASH));
  dbp->h_internal = NULL;
  return (0);
}

/*
 * CDB___ham_set_h_ffactor --
 *  Set the fill factor.
 */
static int
CDB___ham_set_h_ffactor (dbp, h_ffactor)
     DB *dbp;
     uint32_t h_ffactor;
{
  HASH *hashp;

  DB_ILLEGAL_AFTER_OPEN (dbp, "set_h_ffactor");
  DB_ILLEGAL_METHOD (dbp, DB_OK_HASH);

  hashp = dbp->h_internal;
  hashp->h_ffactor = h_ffactor;
  return (0);
}

/*
 * CDB___ham_set_h_hash --
 *  Set the hash function.
 */
static int
CDB___ham_set_h_hash (dbp, func)
     DB *dbp;
uint32_t (*func) __P ((const void *, uint32_t));
{
  HASH *hashp;

  DB_ILLEGAL_AFTER_OPEN (dbp, "set_h_hash");
  DB_ILLEGAL_METHOD (dbp, DB_OK_HASH);

  hashp = dbp->h_internal;
  hashp->h_hash = func;
  return (0);
}

/*
 * CDB___ham_set_h_nelem --
 *  Set the table size.
 */
static int
CDB___ham_set_h_nelem (dbp, h_nelem)
     DB *dbp;
     uint32_t h_nelem;
{
  HASH *hashp;

  DB_ILLEGAL_AFTER_OPEN (dbp, "set_h_nelem");
  DB_ILLEGAL_METHOD (dbp, DB_OK_HASH);

  hashp = dbp->h_internal;
  hashp->h_nelem = h_nelem;
  return (0);
}
