/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 1997, 1998, 1999, 2000
 *  Sleepycat Software.  All rights reserved.
 */

#include "hlconfig.h"

#ifndef lint
static const char revid[] =
  "$Id: db_upg.c,v 1.2 2002/02/02 18:18:05 ghutchis Exp $";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <string.h>
#endif

#include "db_int.h"
#include "db_page.h"
#include "db_swap.h"
#include "btree.h"
#include "hash.h"
#include "qam.h"

static int (*const func_31_list[P_PAGETYPE_MAX])
__P ((DB *, char *, uint32_t, DB_FH *, PAGE *, int *)) =
{
  NULL,                         /* P_INVALID */
    NULL,                       /* __P_DUPLICATE */
    CDB___ham_31_hash,          /* P_HASH */
    NULL,                       /* P_IBTREE */
    NULL,                       /* P_IRECNO */
    CDB___bam_31_lbtree,        /* P_LBTREE */
    NULL,                       /* P_LRECNO */
    NULL,                       /* P_OVERFLOW */
    CDB___ham_31_hashmeta,      /* P_HASHMETA */
    CDB___bam_31_btreemeta,     /* P_BTREEMETA */
};

static int __db_page_pass __P ((DB *, char *, uint32_t, int (*const[])
                                (DB *, char *, uint32_t, DB_FH *, PAGE *,
                                 int *), DB_FH *));

/*
 * CDB___db_upgrade --
 *  Upgrade an existing database.
 *
 * PUBLIC: int CDB___db_upgrade __P((DB *, const char *, uint32_t));
 */
int
CDB___db_upgrade (dbp, fname, flags)
     DB *dbp;
     const char *fname;
     uint32_t flags;
{
  DB_ENV *dbenv;
  DB_FH fh;
  size_t n;
  int ret, t_ret;
  uint8_t mbuf[256];
  char *real_name;

  dbenv = dbp->dbenv;

  /* Validate arguments. */
  if ((ret = CDB___db_fchk (dbenv, "DB->upgrade", flags, DB_DUPSORT)) != 0)
    return (ret);

  /* Get the real backing file name. */
  if ((ret = CDB___db_appname (dbenv,
                               DB_APP_DATA, NULL, fname, 0, NULL,
                               &real_name)) != 0)
    return (ret);

  /* Open the file. */
  if ((ret = CDB___os_open (dbenv, real_name, 0, 0, &fh)) != 0)
  {
    CDB___db_err (dbenv, "%s: %s", real_name, CDB_db_strerror (ret));
    return (ret);
  }

  /* Initialize the feedback. */
  if (dbp->db_feedback != NULL)
    dbp->db_feedback (dbp, DB_UPGRADE, 0);

  /*
   * Read the metadata page.  We read 256 bytes, which is larger than
   * any access method's metadata page and smaller than any disk sector.
   */
  if ((ret = CDB___os_read (dbenv, &fh, mbuf, sizeof (mbuf), &n)) != 0)
    goto err;

  switch (((DBMETA *) mbuf)->magic)
  {
  case DB_BTREEMAGIC:
    switch (((DBMETA *) mbuf)->version)
    {
    case 6:
      /*
       * Before V7 not all pages had page types, so we do the
       * single meta-data page by hand.
       */
      if ((ret = CDB___bam_30_btreemeta (dbp, real_name, mbuf)) != 0)
        goto err;
      if ((ret = CDB___os_seek (dbenv, &fh, 0, 0, 0, 0, DB_OS_SEEK_SET)) != 0)
        goto err;
      if ((ret = CDB___os_write (dbenv, &fh, mbuf, 256, &n)) != 0)
        goto err;
      /* FALLTHROUGH */
    case 7:
      /*
       * We need the page size to do more.  Rip it out of
       * the meta-data page.
       */
      memcpy (&dbp->pgsize, mbuf + 20, sizeof (uint32_t));

      if ((ret =
           __db_page_pass (dbp, real_name, flags, func_31_list, &fh)) != 0)
        goto err;
      /* FALLTHROUGH */
    case 8:
      break;
    default:
      CDB___db_err (dbenv, "%s: unsupported btree version: %lu",
                    real_name, (u_long) ((DBMETA *) mbuf)->version);
      ret = DB_OLD_VERSION;
      goto err;
    }
    break;
  case DB_HASHMAGIC:
    switch (((DBMETA *) mbuf)->version)
    {
    case 4:
    case 5:
      /*
       * Before V6 not all pages had page types, so we do the
       * single meta-data page by hand.
       */
      if ((ret = CDB___ham_30_hashmeta (dbp, real_name, mbuf)) != 0)
        goto err;
      if ((ret = CDB___os_seek (dbenv, &fh, 0, 0, 0, 0, DB_OS_SEEK_SET)) != 0)
        goto err;
      if ((ret = CDB___os_write (dbenv, &fh, mbuf, 256, &n)) != 0)
        goto err;
      /* FALLTHROUGH */
    case 6:
      /*
       * We need the page size to do more.  Rip it out of
       * the meta-data page.
       */
      memcpy (&dbp->pgsize, mbuf + 20, sizeof (uint32_t));

      if ((ret =
           __db_page_pass (dbp, real_name, flags, func_31_list, &fh)) != 0)
        goto err;
      /* FALLTHROUGH */
    case 7:
      break;
    default:
      CDB___db_err (dbenv, "%s: unsupported hash version: %lu",
                    real_name, (u_long) ((DBMETA *) mbuf)->version);
      ret = DB_OLD_VERSION;
      goto err;
    }
    break;
  case DB_QAMMAGIC:
    switch (((DBMETA *) mbuf)->version)
    {
    case 1:
      /*
       * If we're in a Queue database, the only page that
       * needs upgrading is the meta-database page, don't
       * bother with a full pass.
       */
      if ((ret = CDB___qam_31_qammeta (dbp, real_name, mbuf)) != 0)
        return (ret);
      if ((ret = CDB___os_seek (dbenv, &fh, 0, 0, 0, 0, DB_OS_SEEK_SET)) != 0)
        goto err;
      if ((ret = CDB___os_write (dbenv, &fh, mbuf, 256, &n)) != 0)
        goto err;
      /* FALLTHROUGH */
    case 2:
      break;
    default:
      CDB___db_err (dbenv, "%s: unsupported queue version: %lu",
                    real_name, (u_long) ((DBMETA *) mbuf)->version);
      ret = DB_OLD_VERSION;
      goto err;
    }
    break;
  default:
    M_32_SWAP (((DBMETA *) mbuf)->magic);
    switch (((DBMETA *) mbuf)->magic)
    {
    case DB_BTREEMAGIC:
    case DB_HASHMAGIC:
    case DB_QAMMAGIC:
      CDB___db_err (dbenv,
                    "%s: DB->upgrade only supported on native byte-order systems",
                    real_name);
      break;
    default:
      CDB___db_err (dbenv, "%s: unrecognized file type", real_name);
      break;
    }
    ret = EINVAL;
    goto err;
  }

  ret = CDB___os_fsync (dbenv, &fh);

err:if ((t_ret = CDB___os_closehandle (&fh)) != 0 && ret == 0)
    ret = t_ret;
  CDB___os_freestr (real_name);

  /* We're done. */
  if (dbp->db_feedback != NULL)
    dbp->db_feedback (dbp, DB_UPGRADE, 100);

  return (ret);
}

/*
 * __db_page_pass --
 *  Walk the pages of the database, upgrading whatever needs it.
 */
static int
__db_page_pass (dbp, real_name, flags, fl, fhp)
     DB *dbp;
     char *real_name;
     uint32_t flags;
     int (*const fl[P_PAGETYPE_MAX])
  __P ((DB *, char *, uint32_t, DB_FH *, PAGE *, int *));
     DB_FH *fhp;
{
  DB_ENV *dbenv;
  PAGE *page;
  db_pgno_t i, pgno_last;
  size_t n;
  int dirty, ret;

  dbenv = dbp->dbenv;

  /* Determine the last page of the file. */
  if ((ret = CDB___db_lastpgno (dbp, real_name, fhp, &pgno_last)) != 0)
    return (ret);

  /* Allocate memory for a single page. */
  if ((ret = CDB___os_malloc (dbenv, dbp->pgsize, NULL, &page)) != 0)
    return (ret);

  /* Walk the file, calling the underlying conversion functions. */
  for (i = 0; i < pgno_last; ++i)
  {
    if (dbp->db_feedback != NULL)
      dbp->db_feedback (dbp, DB_UPGRADE, (i * 100) / pgno_last);
    if ((ret = CDB___os_seek (dbenv,
                              fhp, dbp->pgsize, i, 0, 0,
                              DB_OS_SEEK_SET)) != 0)
      break;
    if ((ret = CDB___os_read (dbenv, fhp, page, dbp->pgsize, &n)) != 0)
      break;
    dirty = 0;
    if (fl[TYPE (page)] != NULL && (ret = fl[TYPE (page)]
                                    (dbp, real_name, flags, fhp, page,
                                     &dirty)) != 0)
      break;
    if (dirty)
    {
      if ((ret = CDB___os_seek (dbenv,
                                fhp, dbp->pgsize, i, 0, 0,
                                DB_OS_SEEK_SET)) != 0)
        break;
      if ((ret = CDB___os_write (dbenv, fhp, page, dbp->pgsize, &n)) != 0)
        break;
    }
  }

  CDB___os_free (page, dbp->pgsize);
  return (ret);
}

/*
 * CDB___db_lastpgno --
 *  Return the current last page number of the file.
 *
 * PUBLIC: int CDB___db_lastpgno __P((DB *, char *, DB_FH *, db_pgno_t *));
 */
int
CDB___db_lastpgno (dbp, real_name, fhp, pgno_lastp)
     DB *dbp;
     char *real_name;
     DB_FH *fhp;
     db_pgno_t *pgno_lastp;
{
  DB_ENV *dbenv;
  db_pgno_t pgno_last;
  uint32_t mbytes, bytes;
  int ret;

  dbenv = dbp->dbenv;

  if ((ret = CDB___os_ioinfo (dbenv,
                              real_name, fhp, &mbytes, &bytes, NULL)) != 0)
  {
    CDB___db_err (dbenv, "%s: %s", real_name, CDB_db_strerror (ret));
    return (ret);
  }

  /* Page sizes have to be a power-of-two. */
  if (bytes % dbp->pgsize != 0)
  {
    CDB___db_err (dbenv,
                  "%s: file size not a multiple of the pagesize", real_name);
    return (EINVAL);
  }
  pgno_last = mbytes * (MEGABYTE / dbp->pgsize);
  pgno_last += bytes / dbp->pgsize;

  *pgno_lastp = pgno_last;
  return (0);
}
