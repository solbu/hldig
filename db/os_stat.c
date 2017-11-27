/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 1998, 1999
 *  Sleepycat Software.  All rights reserved.
 */

#include "db_config.h"

#ifndef lint
static const char sccsid[] = "@(#)os_stat.c  11.1 (Sleepycat) 7/25/99";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include "db_int.h"
#include "os_jump.h"

/*
 * CDB___os_exists --
 *  Return if the file exists.
 *
 * PUBLIC: int CDB___os_exists __P((const char *, int *));
 */
int
CDB___os_exists (path, isdirp)
     const char *path;
     int *isdirp;
{
  struct stat sb;

  if (CDB___db_jump.j_exists != NULL)
    return (CDB___db_jump.j_exists (path, isdirp));

  if (stat (path, &sb) != 0)
    return (CDB___os_get_errno ());

#if !defined(S_ISDIR) || defined(STAT_MACROS_BROKEN)
#if defined(_WIN32) || defined(WIN16)
#define  S_ISDIR(m)  (_S_IFDIR & (m))
#else
#define  S_ISDIR(m)  (((m) & 0170000) == 0040000)
#endif
#endif
  if (isdirp != NULL)
    *isdirp = S_ISDIR (sb.st_mode);

  return (0);
}

/*
 * CDB___os_ioinfo --
 *  Return file size and I/O size; abstracted to make it easier
 *  to replace.
 *
 * PUBLIC: int CDB___os_ioinfo __P((const char *,
 * PUBLIC:    DB_FH *, u_int32_t *, u_int32_t *, u_int32_t *));
 */
int
CDB___os_ioinfo (path, fhp, mbytesp, bytesp, iosizep)
     const char *path;
     DB_FH *fhp;
     u_int32_t *mbytesp, *bytesp, *iosizep;
{
  struct stat sb;

  if (CDB___db_jump.j_ioinfo != NULL)
    return (CDB___db_jump.j_ioinfo (path, fhp->fd, mbytesp, bytesp, iosizep));

  if (fstat (fhp->fd, &sb) == -1)
    return (CDB___os_get_errno ());

  /* Return the size of the file. */
  if (mbytesp != NULL)
    *mbytesp = sb.st_size / MEGABYTE;
  if (bytesp != NULL)
    *bytesp = sb.st_size % MEGABYTE;

  /*
   * Return the underlying filesystem blocksize, if available.
   *
   * XXX
   * Check for a 0 size -- the HP MPE/iX architecture has st_blksize,
   * but it's always 0.
   */
#ifdef HAVE_ST_BLKSIZE
  if (iosizep != NULL && (*iosizep = sb.st_blksize) == 0)
    *iosizep = DB_DEF_IOSIZE;
#else
  if (iosizep != NULL)
    *iosizep = DB_DEF_IOSIZE;
#endif
  return (0);
}
