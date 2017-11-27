/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 1998, 1999
 *  Sleepycat Software.  All rights reserved.
 */

#include "db_config.h"

#ifndef lint
static const char sccsid[] =
  "@(#)CDB_log_archive.c  11.2 (Sleepycat) 9/16/99";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER                 /* _WIN32 */
#include <direct.h>
#else
#include <unistd.h>
#endif

#endif /* !NO_SYSTEM_INCLUDES */

#include "db_int.h"
#include "db_dispatch.h"
#include "log.h"

static int CDB___absname __P ((char *, char *, char **));
static int CDB___build_data
__P ((DB_ENV *, char *, char ***, void *(*)(size_t)));
static int CDB___cmpfunc __P ((const void *, const void *));
static int CDB___usermem __P ((char ***, void *(*)(size_t)));

/*
 * CDB_log_archive --
 *  Supporting function for db_archive(1).
 */
int
CDB_log_archive (dbenv, listp, flags, db_malloc)
     DB_ENV *dbenv;
     char ***listp;
     u_int32_t flags;
     void *(*db_malloc) __P ((size_t));
{
  DBT rec;
  DB_LOG *dblp;
  DB_LSN stable_lsn;
  u_int32_t fnum;
  int array_size, n, ret;
  char **array, **arrayp, *name, *p, *pref, buf[MAXPATHLEN];

  PANIC_CHECK (dbenv);
  ENV_REQUIRES_CONFIG (dbenv, dbenv->lg_handle, DB_INIT_LOG);

  name = NULL;
  dblp = dbenv->lg_handle;
  COMPQUIET (fnum, 0);

#define  OKFLAGS  (DB_ARCH_ABS | DB_ARCH_DATA | DB_ARCH_LOG)
  if (flags != 0)
  {
    if ((ret = CDB___db_fchk (dbenv, "CDB_log_archive", flags, OKFLAGS)) != 0)
      return (ret);
    if ((ret =
         CDB___db_fcchk (dbenv,
                         "CDB_log_archive", flags, DB_ARCH_DATA,
                         DB_ARCH_LOG)) != 0)
      return (ret);
  }

  /*
   * Get the absolute pathname of the current directory.  It would
   * be nice to get the shortest pathname of the database directory,
   * but that's just not possible.
   *
   * XXX
   * Can't trust getcwd(3) to set a valid errno.  If it doesn't, just
   * guess that we ran out of memory.
   */
  if (LF_ISSET (DB_ARCH_ABS))
  {
    CDB___os_set_errno (0);
    if ((pref = getcwd (buf, sizeof (buf))) == NULL)
    {
      if (CDB___os_get_errno () == 0)
        CDB___os_set_errno (ENOMEM);
      return (CDB___os_get_errno ());
    }
  }
  else
    pref = NULL;

  switch (LF_ISSET (~DB_ARCH_ABS))
  {
  case DB_ARCH_DATA:
    return (CDB___build_data (dbenv, pref, listp, db_malloc));
  case DB_ARCH_LOG:
    memset (&rec, 0, sizeof (rec));
    if (F_ISSET (dbenv, DB_ENV_THREAD))
      F_SET (&rec, DB_DBT_MALLOC);
    if ((ret = CDB_log_get (dbenv, &stable_lsn, &rec, DB_LAST)) != 0)
      return (ret);
    if (F_ISSET (dbenv, DB_ENV_THREAD))
      CDB___os_free (rec.data, rec.size);
    fnum = stable_lsn.file;
    break;
  case 0:
    if ((ret = CDB___log_findckp (dbenv, &stable_lsn)) != 0)
    {
      /*
       * A return of DB_NOTFOUND means that we didn't find
       * any records in the log (so we are not going to be
       * deleting any log files).
       */
      if (ret != DB_NOTFOUND)
        return (ret);
      *listp = NULL;
      return (0);
    }
    /* Remove any log files before the last stable LSN. */
    fnum = stable_lsn.file - 1;
    break;
  }

#define  LIST_INCREMENT  64
  /* Get some initial space. */
  array_size = 10;
  if ((ret =
       CDB___os_malloc (sizeof (char *) * array_size, NULL, &array)) != 0)
    return (ret);
  array[0] = NULL;

  /* Build an array of the file names. */
  for (n = 0; fnum > 0; --fnum)
  {
    if ((ret = CDB___log_name (dblp, fnum, &name, NULL, 0)) != 0)
      goto err;
    if (CDB___os_exists (name, NULL) != 0)
    {
      CDB___os_freestr (name);
      name = NULL;
      break;
    }

    if (n >= array_size - 1)
    {
      array_size += LIST_INCREMENT;
      if ((ret =
           CDB___os_realloc (sizeof (char *) * array_size, NULL,
                             &array)) != 0)
        goto err;
    }

    if (LF_ISSET (DB_ARCH_ABS))
    {
      if ((ret = CDB___absname (pref, name, &array[n])) != 0)
        goto err;
      CDB___os_freestr (name);
    }
    else if ((p = CDB___db_rpath (name)) != NULL)
    {
      if ((ret = CDB___os_strdup (p + 1, &array[n])) != 0)
        goto err;
      CDB___os_freestr (name);
    }
    else
      array[n] = name;

    name = NULL;
    array[++n] = NULL;
  }

  /* If there's nothing to return, we're done. */
  if (n == 0)
  {
    *listp = NULL;
    ret = 0;
    goto err;
  }

  /* Sort the list. */
  qsort (array, (size_t) n, sizeof (char *), CDB___cmpfunc);

  /* Rework the memory. */
  if ((ret = CDB___usermem (&array, db_malloc)) != 0)
    goto err;

  *listp = array;
  return (0);

err:if (array != NULL)
  {
    for (arrayp = array; *arrayp != NULL; ++arrayp)
      CDB___os_freestr (*arrayp);
    CDB___os_free (array, sizeof (char *) * array_size);
  }
  if (name != NULL)
    CDB___os_freestr (name);
  return (ret);
}

/*
 * CDB___build_data --
 *  Build a list of datafiles for return.
 */
static int
CDB___build_data (dbenv, pref, listp, db_malloc)
     DB_ENV *dbenv;
     char *pref, ***listp;
     void *(*db_malloc) __P ((size_t));
{
  DBT rec;
  DB_LOG *dblp;
  DB_LSN lsn;
  __log_register_args *argp;
  u_int32_t rectype;
  int array_size, last, n, nxt, ret;
  char **array, **arrayp, *p, *real_name;

  dblp = dbenv->lg_handle;

  /* Get some initial space. */
  array_size = 10;
  if ((ret =
       CDB___os_malloc (sizeof (char *) * array_size, NULL, &array)) != 0)
    return (ret);
  array[0] = NULL;

  memset (&rec, 0, sizeof (rec));
  if (F_ISSET (dbenv, DB_ENV_THREAD))
    F_SET (&rec, DB_DBT_MALLOC);
  for (n = 0, ret = CDB_log_get (dbenv, &lsn, &rec, DB_FIRST);
       ret == 0; ret = CDB_log_get (dbenv, &lsn, &rec, DB_NEXT))
  {
    if (rec.size < sizeof (rectype))
    {
      ret = EINVAL;
      CDB___db_err (dbenv, "CDB_log_archive: bad log record");
      goto lg_free;
    }

    memcpy (&rectype, rec.data, sizeof (rectype));
    if (rectype != DB_log_register)
    {
      if (F_ISSET (dbenv, DB_ENV_THREAD))
      {
        CDB___os_free (rec.data, rec.size);
        rec.data = NULL;
      }
      continue;
    }
    if ((ret = CDB___log_register_read (rec.data, &argp)) != 0)
    {
      ret = EINVAL;
      CDB___db_err (dbenv, "CDB_log_archive: unable to read log record");
      goto lg_free;
    }

    if (n >= array_size - 1)
    {
      array_size += LIST_INCREMENT;
      if ((ret = CDB___os_realloc (sizeof (char *) * array_size,
                                   NULL, &array)) != 0)
        goto lg_free;
    }

    if ((ret = CDB___os_strdup (argp->name.data, &array[n])) != 0)
    {
    lg_free:if (F_ISSET (&rec, DB_DBT_MALLOC) && rec.data != NULL)
        CDB___os_free (rec.data, rec.size);
      goto err1;
    }

    array[++n] = NULL;
    CDB___os_free (argp, 0);

    if (F_ISSET (dbenv, DB_ENV_THREAD))
    {
      CDB___os_free (rec.data, rec.size);
      rec.data = NULL;
    }
  }

  /* If there's nothing to return, we're done. */
  if (n == 0)
  {
    ret = 0;
    *listp = NULL;
    goto err1;
  }

  /* Sort the list. */
  qsort (array, (size_t) n, sizeof (char *), CDB___cmpfunc);

  /*
   * Build the real pathnames, discarding nonexistent files and
   * duplicates.
   */
  for (last = nxt = 0; nxt < n;)
  {
    /*
     * Discard duplicates.  Last is the next slot we're going
     * to return to the user, nxt is the next slot that we're
     * going to consider.
     */
    if (last != nxt)
    {
      array[last] = array[nxt];
      array[nxt] = NULL;
    }
    for (++nxt; nxt < n && strcmp (array[last], array[nxt]) == 0; ++nxt)
    {
      CDB___os_freestr (array[nxt]);
      array[nxt] = NULL;
    }

    /* Get the real name. */
    if ((ret = CDB___db_appname (dbenv,
                                 DB_APP_DATA, NULL, array[last], 0, NULL,
                                 &real_name)) != 0)
      goto err2;

    /* If the file doesn't exist, ignore it. */
    if (CDB___os_exists (real_name, NULL) != 0)
    {
      CDB___os_freestr (real_name);
      CDB___os_freestr (array[last]);
      array[last] = NULL;
      continue;
    }

    /* Rework the name as requested by the user. */
    CDB___os_freestr (array[last]);
    array[last] = NULL;
    if (pref != NULL)
    {
      ret = CDB___absname (pref, real_name, &array[last]);
      CDB___os_freestr (real_name);
      if (ret != 0)
        goto err2;
    }
    else if ((p = CDB___db_rpath (real_name)) != NULL)
    {
      ret = CDB___os_strdup (p + 1, &array[last]);
      CDB___os_freestr (real_name);
      if (ret != 0)
        goto err2;
    }
    else
      array[last] = real_name;
    ++last;
  }

  /* NULL-terminate the list. */
  array[last] = NULL;

  /* Rework the memory. */
  if ((ret = CDB___usermem (&array, db_malloc)) != 0)
    goto err1;

  *listp = array;
  return (0);

err2:                          /*
                                 * XXX
                                 * We've possibly inserted NULLs into the array list, so clean up a
                                 * bit so that the other error processing works.
                                 */
  if (array != NULL)
    for (; nxt < n; ++nxt)
      CDB___os_freestr (array[nxt]);
  /* FALLTHROUGH */

err1:if (array != NULL)
  {
    for (arrayp = array; *arrayp != NULL; ++arrayp)
      CDB___os_freestr (*arrayp);
    CDB___os_free (array, array_size * sizeof (char *));
  }
  return (ret);
}

/*
 * CDB___absname --
 *  Return an absolute path name for the file.
 */
static int
CDB___absname (pref, name, newnamep)
     char *pref, *name, **newnamep;
{
  size_t l_pref, l_name;
  int isabspath, ret;
  char *newname;

  l_name = strlen (name);
  isabspath = CDB___os_abspath (name);
  l_pref = isabspath ? 0 : strlen (pref);

  /* Malloc space for concatenating the two. */
  if ((ret = CDB___os_malloc (l_pref + l_name + 2, NULL, &newname)) != 0)
    return (ret);
  *newnamep = newname;

  /* Build the name.  If `name' is an absolute path, ignore any prefix. */
  if (!isabspath)
  {
    memcpy (newname, pref, l_pref);
    if (strchr (PATH_SEPARATOR, newname[l_pref - 1]) == NULL)
      newname[l_pref++] = PATH_SEPARATOR[0];
  }
  memcpy (newname + l_pref, name, l_name + 1);

  return (0);
}

/*
 * CDB___usermem --
 *  Create a single chunk of memory that holds the returned information.
 *  If the user has their own malloc routine, use it.
 */
static int
CDB___usermem (listp, db_malloc)
     char ***listp;
     void *(*db_malloc) __P ((size_t));
{
  size_t len;
  int ret;
  char **array, **arrayp, **orig, *strp;

  /* Find out how much space we need. */
  for (len = 0, orig = *listp; *orig != NULL; ++orig)
    len += sizeof (char *) + strlen (*orig) + 1;
  len += sizeof (char *);

  /* Allocate it and set up the pointers. */
  if ((ret = CDB___os_malloc (len, db_malloc, &array)) != 0)
    return (ret);

  strp = (char *) (array + (orig - *listp) + 1);

  /* Copy the original information into the new memory. */
  for (orig = *listp, arrayp = array; *orig != NULL; ++orig, ++arrayp)
  {
    len = strlen (*orig);
    memcpy (strp, *orig, len + 1);
    *arrayp = strp;
    strp += len + 1;

    CDB___os_freestr (*orig);
  }

  /* NULL-terminate the list. */
  *arrayp = NULL;

  CDB___os_free (*listp, 0);
  *listp = array;

  return (0);
}

static int
CDB___cmpfunc (p1, p2)
     const void *p1, *p2;
{
  return (strcmp (*((char *const *) p1), *((char *const *) p2)));
}
