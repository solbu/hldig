/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 1998, 1999
 *  Sleepycat Software.  All rights reserved.
 */

#include "db_config.h"

#ifndef lint
static const char sccsid[] = "@(#)os_unlink.c  11.1 (Sleepycat) 7/25/99";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#ifndef _MSC_VER /* _WIN32 */
#include <unistd.h>
#endif
#endif

#include "db_int.h"
#include "os_jump.h"

/*
 * CDB___os_unlink --
 *  Remove a file.
 *
 * PUBLIC: int CDB___os_unlink __P((const char *));
 */
int
CDB___os_unlink(path)
  const char *path;
{
  int ret;

  ret = CDB___db_jump.j_unlink != NULL ?
      CDB___db_jump.j_unlink(path) : unlink(path);
  return (ret == -1 ? CDB___os_get_errno() : 0);
}
