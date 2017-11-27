/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 1998, 1999
 *  Sleepycat Software.  All rights reserved.
 */

#define _XOPEN_SOURCE 500
#include <sys/types.h>
#include <unistd.h>
#ifndef u_long
typedef __u_long u_long;
#endif

#include "db_config.h"

#ifndef lint
static const char sccsid[] = "@(#)os_rw.c  11.2 (Sleepycat) 9/20/99";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#ifndef _MSC_VER                /* _WIN32 */
#include <unistd.h>
#endif
#endif

#include "db_int.h"
#include "os_jump.h"

/*
 * CDB___os_io --
 *  Do an I/O.
 *
 * PUBLIC: int CDB___os_io __P((DB_IO *, int, ssize_t *));
 */
int
CDB___os_io (db_iop, op, niop)
     DB_IO *db_iop;
     int op;
     ssize_t *niop;
{
  int ret;

  /* HACK to debug where the O_BINARY mode of the file gets fouled up */
  /*
     printf("\n[CDB___os_io]");
     printf("DB_IO_READ:[%d], DB_IO_WRITE:[%d]\n", (DB_IO_READ==op?1:0), (DB_IO_WRITE==op?1:0));
     printf("[CDB___os_io]");
     #ifdef HAVE_PREAD
     printf("using pread/pwrite\n");
     #else
     printf("using CDB___os_read & CDB___os_write\n");
     #endif
     printf("[CDB___os_io]");
     printf("FD=[%d], BYTES=[%d], PAGESIZE=[%d]\n", db_iop->fhp->fd, db_iop->bytes,db_iop->pagesize);
   */

#ifdef HAVE_PREAD
  switch (op)
  {
  case DB_IO_READ:
    if (CDB___db_jump.j_read != NULL)
      goto slow;
    *niop = pread (db_iop->fhp->fd, db_iop->buf,
                   db_iop->bytes, (off_t) db_iop->pgno * db_iop->pagesize);
    break;
  case DB_IO_WRITE:
    if (CDB___db_jump.j_write != NULL)
      goto slow;
    *niop = pwrite (db_iop->fhp->fd, db_iop->buf,
                    db_iop->bytes, (off_t) db_iop->pgno * db_iop->pagesize);
    break;
  }
  if (*niop == (ssize_t) db_iop->bytes)
    return (0);
slow:
#endif
  MUTEX_THREAD_LOCK (db_iop->mutexp);

  if ((ret = CDB___os_seek (db_iop->fhp,
                            db_iop->pagesize, db_iop->pgno, 0, 0,
                            DB_OS_SEEK_SET)) != 0)
    goto err;
  switch (op)
  {
  case DB_IO_READ:
    ret = CDB___os_read (db_iop->fhp, db_iop->buf, db_iop->bytes, niop);
    break;
  case DB_IO_WRITE:
    ret = CDB___os_write (db_iop->fhp, db_iop->buf, db_iop->bytes, niop);
    break;
  }

err:MUTEX_THREAD_UNLOCK (db_iop->mutexp);

  return (ret);

}

/*
 * CDB___os_read --
 *  Read from a file handle.
 *
 * PUBLIC: int CDB___os_read __P((DB_FH *, void *, size_t, ssize_t *));
 */
int
CDB___os_read (fhp, addr, len, nrp)
     DB_FH *fhp;
     void *addr;
     size_t len;
     ssize_t *nrp;
{
  size_t offset;
  ssize_t nr;
  u_int8_t *taddr;


  /* HACK to debug where the O_BINARY mode of the file gets fouled up */
  /*
     printf("\n[CDB___os_read] fhp->fd=[%d], len=[%d]\n", fhp->fd, len);
     printf("[CDB___os_read] CDB___db_jump.j_read==NULL ?[%d]\n",
     ( CDB___db_jump.j_read == NULL ?1:0) );  
     printf("[CDB___os_read] lseek(fhp->fd, 0, SEEK_CUR)=[%d]\n", lseek(fhp->fd, 0, SEEK_CUR));
     printf("[CDB___os_read] current mode=[%#x]\n", setmode(fhp->fd, 0x8000));
   */

  for (taddr = addr, offset = 0; offset < len; taddr += nr, offset += nr)
  {
    if ((nr = CDB___db_jump.j_read != NULL ?
         CDB___db_jump.j_read (fhp->fd, taddr, len - offset) :
         read (fhp->fd, taddr, len - offset)) < 0)
      return (CDB___os_get_errno ());
    if (nr == 0)
      break;
  }
  *nrp = taddr - (u_int8_t *) addr;
  return (0);
}

/*
 * CDB___os_write --
 *  Write to a file handle.
 *
 * PUBLIC: int CDB___os_write __P((DB_FH *, void *, size_t, ssize_t *));
 */
int
CDB___os_write (fhp, addr, len, nwp)
     DB_FH *fhp;
     void *addr;
     size_t len;
     ssize_t *nwp;
{
  size_t offset;
  ssize_t nw;
  u_int8_t *taddr;

  /* HACK to debug where the O_BINARY mode of the file gets fouled up */
  /*
     printf("\n[CDB___os_write] fhp->fd=[%d], len=[%d]\n", fhp->fd, len);
     printf("[CDB___os_write] CDB___db_jump.j_write==NULL ?[%d]\n",
     ( CDB___db_jump.j_write == NULL ?1:0) );  
     printf("[CDB___os_write] lseek(fhp->fd, 0, SEEK_CUR)=[%d]\n", lseek(fhp->fd, 0, SEEK_CUR));
     printf("[CDB___os_write] current mode=[%#x]\n", setmode(fhp->fd, 0x8000));
   */

  for (taddr = addr, offset = 0; offset < len; taddr += nw, offset += nw)
  {
    if ((nw = CDB___db_jump.j_write != NULL ?
         CDB___db_jump.j_write (fhp->fd, taddr, len - offset) :
         write (fhp->fd, taddr, len - offset)) < 0)
      return (CDB___os_get_errno ());

    /* HACK to debug where the O_BINARY mode of the file gets fouled up */
    /* printf("** %d bytes written, wanted to write %d\n", nw, len-offset); */
  }
  *nwp = len;

  /* HACK to debug where the O_BINARY mode of the file gets fouled up */
  /* printf("[AFTER CDB___os_write] lseek(fhp->fd, 0, SEEK_CUR)=[%d]\n", lseek(fhp->fd, 0, SEEK_CUR)); */

  return (0);
}
