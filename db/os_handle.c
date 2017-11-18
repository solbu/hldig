/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1998, 1999
 *  Sleepycat Software.  All rights reserved.
 */

#include "db_config.h"

#ifndef lint
static const char sccsid[] = "@(#)os_handle.c  11.2 (Sleepycat) 11/12/99";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#include <fcntl.h>
#include <string.h>

#ifndef _MSC_VER /* _WIN32 */
#include <unistd.h>
#endif

#endif

#include "db_int.h"
#include "os_jump.h"

/*
 * CDB___os_openhandle --
 *  Open a file, using POSIX 1003.1 open flags.
 *
 * PUBLIC: int CDB___os_openhandle __P((const char *, int, int, DB_FH *));
 */
int
CDB___os_openhandle(name, flags, mode, fhp)
  const char *name;
  int flags, mode;
  DB_FH *fhp;
{
  int ret, try;

  memset(fhp, 0, sizeof(*fhp));

  for (ret = 0, try = 1; try < 4; ++try) {
    /*
     * !!!
     * Open with full sharing on VMS.
     *
     * We use these flags because they are the ones set by the VMS
     * CRTL mmap() call when it opens a file, and we have to be
     * able to open files that mmap() has previously opened, e.g.,
     * when we're joining already existing DB regions.
     */

#ifdef _MSC_VER /* _WIN32 */
        /* THIS IS A HACK TO REINSTATE BINARY MODE!  FIX ME.. FIND THE PROBLEM*/
        mode |= _O_BINARY;
#endif
        
    fhp->fd = CDB___db_jump.j_open != NULL ?
        CDB___db_jump.j_open(name, flags, mode) :
#ifdef __VMS
        open(name, flags, mode, "shr=get,put,upd,del,upi");
#else
        open(name, flags, mode);
#endif

        /* DEBUGGING */
     /* printf("\n[CDB___os_openhandle] name=[%s] mode=[%#x] fhp->fd=[%d]\n", name, mode, fhp->fd); */
        
    if (fhp->fd == -1) {
      /*
       * If it's a "temporary" error, we retry up to 3 times,
       * waiting up to 12 seconds.  While it's not a problem
       * if we can't open a database, an inability to open a
       * log file is cause for serious dismay.
       */
      ret = CDB___os_get_errno();
      if (ret == ENFILE || ret == EMFILE || ret == ENOSPC) {
        (void)CDB___os_sleep(try * 2, 0);
        continue;
      }
    } else
      F_SET(fhp, DB_FH_VALID);
    break;
  }
  return (ret);
}

/*
 * CDB___os_closehandle --
 *  Close a file.
 *
 * PUBLIC: int CDB___os_closehandle __P((DB_FH *));
 */
int
CDB___os_closehandle(fhp)
  DB_FH *fhp;
{
  int ret;

  /* Don't close file descriptors that were never opened. */
  DB_ASSERT(F_ISSET(fhp, DB_FH_VALID) && fhp->fd != -1);

  ret = CDB___db_jump.j_close != NULL ?
      CDB___db_jump.j_close(fhp->fd) : close(fhp->fd);

  /*
   * Smash the POSIX file descriptor -- it's never tested, but we want
   * to catch any mistakes.
   */
  fhp->fd = -1;
  F_CLR(fhp, DB_FH_VALID);

  return (ret == 0 ? 0 : CDB___os_get_errno());
}
