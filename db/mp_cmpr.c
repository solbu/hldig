/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1999, 2000
 *  Loic Dachary.  All rights reserved.
 *
 * Overview of the code (by Lachlan Andrew, lha@users.sourceforge.net):
 *
 * This code compresses pages on-the-fly, either using a built-in algorithm,
 * or using the  zlib  library.  The compressed page is stored in pages of
 * size  CMPR_MULTIPLY(db_io->pagesize)  -- a fixed multiple of the true
 * page size,  db_io->pagesize.  If the compressed page requires multiple
 * pages, extra pages are allocated at the end of the file, and "chained"
 * on to the original page.  The chain is specified as an array in the first
 * page (not a linked list).  If a subsequent write of the page requires
 * a shorter chain, the spare pages are recorded as "free" and listed in
 * the weak-compression database (with suffix given by DB_CMPR_SUFFIX).
 *
 * When writing a compressed page, extra memory may need to be allocated if
 * chaining occurs.  This can cause recursive calls to  CDB___memp_alloc(),
 * since the latter may write dirty cache pages to satisfy the request.
 * There is currently an explicit check for recursive calls, both in
 * CDB___memp_alloc()  and  CDB___memp_cmpr_write(), but a more elegant
 * solution would be nice.
 *
 * There also seems to be an issue with the memory allocation for the chain
 * array.  The small allocations seem to cause fragmentation in the memory
 * pool (seen as very many small clean blocks, which don't go away).
 * 
 *
 * TODO:
 *   Keith Bostic says:
 *   The only change I'd probably think about is if
 *   we should merge the call to CDB___memp_pg and CDB___memp_cmpr -- kind
 *   of a stack of page modification routines, that sits on top of
 *   CDB___os_io.  That's a bigger change, but it's probably cleaner
 *   in the long-run.
 * 
 * Pending questions:
 *
 *  The CMPR structure contains binary data. Should we store them in network order ?
 *  How is this related to DB_AM_SWAP ?
 *
 *  The calls to cmpr_open/cmpr_close in memp_pgread/pgwrite are probably not
 *  at the right place / most logical place in the function. I have troubles
 *  finding out where to put them. They work, that's not the problem. Just don't
 *  know if they should be before or after.  
 *
 *  When opening weakcmpr DB, the DB_THREAD flag is not available. Should it be
 *  set if the main DB has this flag set ?
 *
 *  In CDB___memp_cmpr, niop is always multiplied by compression factor for page 0. 
 *  I see no problems with this but it's a bit awkward.
 *
 *  In CDB___memp_cmpr_page, the page built fills some fields of the PAGE structure
 *  others are set to 0. I'm not 100% sure this is enough. It should only impact
 *  utilities that read pages by incrementing pgno. Only stat does this an it's
 *  enough for it. I've not found any other context where these fake pages are 
 *  used.
 * 
 */

#include "db_config.h"

#ifndef lint
static const char sccsid[] = "@(#)mp_cmpr.c  1.1 (Senga) 01/08/99";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#endif

#include "db_int.h"
#include "db_page.h"
#include "shqueue.h"
#include "db_shash.h"
#include "mp.h"
#include "db_page.h"
#include "common_ext.h"

#ifdef DEBUG
#include "WordMonitor.h"
#endif /* DEBUG */

#if 0
#define DEBUG_CMPR 1
#endif
#if 0
#define DEBUG_CMPR_ALLOC 1
#endif

#ifdef HAVE_LIBZ
#include "zlib.h"
#endif /* HAVE_LIBZ */
static int memp_cmpr_zlib_level = -1;

/*
 * Helpers declarations.
 */
static int CDB___memp_cmpr_page
__P ((DB_MPOOLFILE *, CMPR *, DB_IO *, ssize_t *));

/*
 * Maximum chain length
 * Cast to signed, as  -1  used as a flag, which compares bigger on some systems
 */
#define CMPR_MAX  (int)(dbenv->mp_cmpr_info->max_npages)

#define CMPR_MULTIPLY(n) ((n) << (dbenv->mp_cmpr_info->coefficient))
#define CMPR_DIVIDE(n)   ((n) >> (dbenv->mp_cmpr_info->coefficient))

#ifdef HAVE_LIBZ
static DB_CMPR_INFO default_cmpr_info = {
  CDB___memp_cmpr_deflate,
  CDB___memp_cmpr_inflate,
  3,                            /* reduce page size by factor of 1<<3 = 8 */
  9,                            /* allow 9 reduced pages, in case "compression" expands data */
  6,                            /* zlib compression level */
  NULL
};
#else /* HAVE_LIBZ */
static DB_CMPR_INFO default_cmpr_info = {
  0,
  0,
  0,
  0,
  0,
  NULL
};
#endif /* HAVE_LIBZ */

/*
 * Entry point. Functionaly equivalent to CDB___os_io.
 * Compress/uncompress pages before returning them or writing them to disk.
 */

/*
 * CDB___memp_cmpr --
 *  Transparent compression read/write
 *
 * PUBLIC: int CDB___memp_cmpr __P((DB_MPOOLFILE *, BH *, DB_IO *, int, ssize_t *));
 */
int
CDB___memp_cmpr (dbmfp, bhp, db_io, flag, niop)
     DB_MPOOLFILE *dbmfp;
     BH *bhp;
     DB_IO *db_io;
     int flag;
     ssize_t *niop;
{
  size_t orig_pagesize = db_io->pagesize;
  db_pgno_t orig_pgno = db_io->pgno;
  size_t orig_bytes = db_io->bytes;
  DB_ENV *dbenv = dbmfp->dbmp->dbenv;
  DB_CMPR_INFO *cmpr_info = dbenv->mp_cmpr_info;
  int ret = 0;

  db_io->pagesize = CMPR_DIVIDE (db_io->pagesize);
  db_io->bytes = CMPR_DIVIDE (db_io->bytes);

#ifdef HAVE_LIBZ
  if (memp_cmpr_zlib_level == -1)
  {
    memp_cmpr_zlib_level = cmpr_info->zlib_flags;
    if (memp_cmpr_zlib_level == -1)
      memp_cmpr_zlib_level = Z_DEFAULT_COMPRESSION;
  }
#endif

  /*
   * Page 0 is a special case. It contains the metadata information (at most 512 bytes)
   * and must not be compressed because it is read with CDB___os_read and not CDB___os_io.
   */
  switch (flag)
  {
  case DB_IO_READ:
    if (db_io->pgno == 0)
    {
      ret = CDB___os_io (db_io, DB_IO_READ, niop);
      *niop = CMPR_MULTIPLY (*niop);
    }
    else
      ret = CDB___memp_cmpr_read (dbmfp, bhp, db_io, niop);
    break;
  case DB_IO_WRITE:
    if (db_io->pgno == 0)
    {
      ret = CDB___os_io (db_io, DB_IO_WRITE, niop);
      *niop = CMPR_MULTIPLY (*niop);
    }
    else
      ret = CDB___memp_cmpr_write (dbmfp, bhp, db_io, niop);
    break;
  }

  db_io->pgno = orig_pgno;
  db_io->pagesize = orig_pagesize;
  db_io->bytes = orig_bytes;

  return ret;
}

/*
 * CDB___memp_cmpr_read --
 *  Transparent compression read
 *
 * PUBLIC: int CDB___memp_cmpr_read __P((DB_MPOOLFILE *, BH *, DB_IO *, ssize_t *));
 */
int
CDB___memp_cmpr_read (dbmfp, bhp, db_io, niop)
     DB_MPOOLFILE *dbmfp;
     BH *bhp;
     DB_IO *db_io;
     ssize_t *niop;
{
  CMPR cmpr;
  int ret;
  int chain = 0;
  u_int8_t *buffcmpr = 0;
  int buffcmpr_length = 0;
  int chain_length = 0;
  db_pgno_t first_pgno = db_io->pgno;
  DB_ENV *dbenv = dbmfp->dbmp->dbenv;
  DB_CMPR_INFO *cmpr_info = dbenv->mp_cmpr_info;
  /*
   * By default the compression does not use too much space,
   * hence the chain is empty.
   */
  F_CLR (bhp, BH_CMPR);

  /*
   * Read first page (if no overflow, this is the only one)
   */
  ret = CDB___os_io (db_io, DB_IO_READ, niop);

  /*
   * An error or partial read on the first page means that we're not
   * going anywhere.
   */
  if (ret || *niop < db_io->pagesize)
    goto err;

  /*
   * Read the cmpr header from page.
   */
  memcpy (&cmpr, db_io->buf, sizeof (CMPR));

  /*
   * If not at the beginning of compressed page chain, build
   * a fake page.
   */
  if (F_ISSET (&cmpr, DB_CMPR_FREE) || F_ISSET (&cmpr, DB_CMPR_INTERNAL))
  {
    ret = CDB___memp_cmpr_page (dbmfp, &cmpr, db_io, niop);
    goto err;
  }

  /*
   * Sanity check. Happens if file corrupted.
   */
  if (!F_ISSET (&cmpr, DB_CMPR_FIRST))
  {
    CDB___db_err (dbmfp->dbmp->dbenv,
                  "CDB___memp_cmpr_read: expected DB_CMPR_FIRST flag set at pgno = %ld",
                  db_io->pgno);
    ret = CDB___db_panic (dbmfp->dbmp->dbenv, EINVAL);
    goto err;
  }

  if ((ret =
       CDB___os_malloc (db_io->pagesize * CMPR_MAX, NULL, &buffcmpr)) != 0)
    goto err;

  do
  {
    /*
     * Read the first part of the compressed data from page.
     */
    memcpy (buffcmpr + buffcmpr_length, DB_CMPR_DATA (db_io),
            DB_CMPR_PAGESIZE (db_io));
    buffcmpr_length += DB_CMPR_PAGESIZE (db_io);

    /*
     * Flag must only contain FIRST|INTERNAL and/or CHAIN. If other bits are
     * set, the data is corrupted. Removing the FIRST|INTERNAL bits and checking
     * the CHAIN bit with == instead of F_ISSET verify this.
     */
    F_CLR (&cmpr, DB_CMPR_FIRST | DB_CMPR_INTERNAL);
    chain = cmpr.flags;

    if (chain == DB_CMPR_CHAIN)
    {
      /*
       * Overflow Case. Continue reading data from extra pages.
       */

      chain_length++;
      if (chain_length >= CMPR_MAX)
      {
        CDB___db_err (dbmfp->dbmp->dbenv,
                      "CDB___memp_cmpr_read: compression chain too long at pgno = %ld",
                      db_io->pgno);
        ret = CDB___db_panic (dbmfp->dbmp->dbenv, EINVAL);
        goto err;
      }

      if (cmpr.next == 0)
      {
        CDB___db_err (dbmfp->dbmp->dbenv,
                      "CDB___memp_cmpr_read: cmpr.next is null at pgno = %ld",
                      chain, db_io->pgno);
        ret = CDB___db_panic (dbmfp->dbmp->dbenv, EINVAL);
        goto err;
      }
      /*
       * Keep the chain in buffer header.
       * Freed when  bhp  freed in  CDB___memp_bhfree().
       */
      CDB___memp_cmpr_alloc_chain (dbmfp->dbmp, bhp, BH_CMPR_POOL);

      bhp->chain[chain_length - 1] = cmpr.next;
      db_io->pgno = cmpr.next;
      /*
       * Read data from extra page.
       */
      if ((ret = CDB___os_io (db_io, DB_IO_READ, niop)) != 0 ||
          *niop != db_io->pagesize)
      {
        ret = EIO;
        goto err;
      }
      /*
       * Read the cmpr header from this extra page
       */
      memcpy (&cmpr, db_io->buf, sizeof (CMPR));
    }
    else if (chain != 0)
    {
      CDB___db_err (dbmfp->dbmp->dbenv,
                    "CDB___memp_cmpr_read: unexpected compression flag value 0x%x at pgno = %ld",
                    chain, db_io->pgno);
      ret = CDB___db_panic (dbmfp->dbmp->dbenv, ret);
      goto err;
    }
    else if (cmpr.next != 0)
    {
      CDB___db_err (dbmfp->dbmp->dbenv,
                    "CDB___memp_cmpr_read: cmpr.next is not null at pgno = %ld",
                    chain, db_io->pgno);
      ret = CDB___db_panic (dbmfp->dbmp->dbenv, ret);
      goto err;
    }
  }
  while (chain);

  /*
   * We gathered all the compressed data in buffcmpr, inflate it.
   */

  if (cmpr_info->zlib_flags != 0)
    ret =
      CDB___memp_cmpr_inflate (buffcmpr, buffcmpr_length, db_io->buf,
                               CMPR_MULTIPLY (db_io->pagesize),
                               cmpr_info->user_data);
  else
    ret =
      (*cmpr_info->uncompress) (buffcmpr, buffcmpr_length, db_io->buf,
                                CMPR_MULTIPLY (db_io->pagesize),
                                cmpr_info->user_data);

  if (ret != 0)
  {
    CDB___db_err (dbmfp->dbmp->dbenv,
                  "CDB___memp_cmpr_read: unable to uncompress page at pgno = %ld",
                  first_pgno);
    ret = CDB___db_panic (dbmfp->dbmp->dbenv, ret);
    goto err;
  }
#ifdef DEBUG
  {
    int ratio =
      buffcmpr_length >
      0 ? (CMPR_MULTIPLY (db_io->pagesize) / buffcmpr_length) : 0;
    if (ratio > 10)
      ratio = 10;
    word_monitor_add (WORD_MONITOR_COMPRESS_01 + ratio, 1);
  }
#endif /* DEBUG */

  *niop = CMPR_MULTIPLY (db_io->pagesize);

err:
#ifdef DEBUG_CMPR
  if (chain_length > 0)
  {
    int i;
    fprintf (stderr,
             "CDB___memp_cmpr_read:: chain_length (number of overflow pages):%2d\n",
             chain_length);
    fprintf (stderr, "CDB___memp_cmpr_read:: chain ");
    for (i = 0; i < chain_length; i++)
      fprintf (stderr, "%d, ", bhp->chain[i]);
    fprintf (stderr, "\n");
  }
#endif
  if (buffcmpr)
    CDB___os_free (buffcmpr, 0);
  return ret;
}

/*
 * CDB___memp_cmpr_write --
 *  Transparent compression write
 *
 * PUBLIC: int CDB___memp_cmpr_write __P((DB_MPOOLFILE *, BH *, DB_IO *, ssize_t *));
 */
int
CDB___memp_cmpr_write (dbmfp, bhp, db_io, niop)
     DB_MPOOLFILE *dbmfp;
     BH *bhp;
     DB_IO *db_io;
     ssize_t *niop;
{
  CMPR cmpr;
  int chain_length = 0;
  int first_nonreused_chain_pos = 0;
  int ret;
  u_int8_t *buffcmpr = 0;
  u_int8_t *buffp;
  unsigned int buffcmpr_length;
  u_int8_t *orig_buff = db_io->buf;
  DB_ENV *dbenv = dbmfp->dbmp->dbenv;
  DB_CMPR_INFO *cmpr_info = dbenv->mp_cmpr_info;

  if ((ret =
       CDB___os_malloc (CMPR_MULTIPLY (db_io->bytes), NULL,
                        &db_io->buf)) != 0)
    goto err;


  if (cmpr_info->zlib_flags != 0)
    ret =
      CDB___memp_cmpr_deflate (orig_buff, CMPR_MULTIPLY (db_io->pagesize),
                               &buffcmpr, &buffcmpr_length,
                               cmpr_info->user_data);
  else
    ret =
      (*cmpr_info->compress) (orig_buff, CMPR_MULTIPLY (db_io->pagesize),
                              &buffcmpr, &buffcmpr_length,
                              cmpr_info->user_data);

  if (ret != 0)
  {
    CDB___db_err (dbmfp->dbmp->dbenv,
                  "CDB___memp_cmpr_write: unable to compress page at pgno = %ld",
                  db_io->pgno);
    ret = CDB___db_panic (dbmfp->dbmp->dbenv, ret);
    goto err;
  }
#ifdef DEBUG
  {
    int ratio =
      buffcmpr_length >
      0 ? (CMPR_MULTIPLY (db_io->pagesize) / buffcmpr_length) : 0;
    if (ratio > 10)
      ratio = 10;
    word_monitor_add (WORD_MONITOR_COMPRESS_01 + ratio, 1);
  }
#endif /* DEBUG */

  /*
   * This can never happen.
   */
  if (buffcmpr_length > DB_CMPR_PAGESIZE (db_io) * CMPR_MAX)
  {
    CDB___db_err (dbmfp->dbmp->dbenv,
                  "CDB___memp_cmpr_write: compressed data is too big at pgno = %ld",
                  db_io->pgno);
    ret = CDB___db_panic (dbmfp->dbmp->dbenv, EINVAL);
    goto err;
  }

  buffp = buffcmpr;
  cmpr.flags = DB_CMPR_FIRST;
  cmpr.next = 0;

  /* write pages until the whole compressed data is written */
  do
  {
    unsigned int length = buffcmpr_length - (buffp - buffcmpr);
    unsigned int copy_length =
      length > DB_CMPR_PAGESIZE (db_io) ? DB_CMPR_PAGESIZE (db_io) : length;
    /*
     * We handle serious compression stuff only if we need to.
     * overflow! the compressed buffer is too big -> get extra page
     */
    if (length > copy_length)
    {
      if (dbmfp->dbmp->recursion_level >= 2)
      {
        fprintf (stderr, "CDB___memp_cmpr_write: Wanted %d > %d bytes\n",
                 length, copy_length);
        fprintf (stderr, "Reducing  wordlist_cache_dirty_level  may help.\n");
        ret = EBUSY;
        goto err;
      }
      chain_length++;
      if (chain_length >= CMPR_MAX)
      {
        CDB___db_err (dbmfp->dbmp->dbenv,
                      "CDB___memp_cmpr_write: chain_length overflow");
        ret = CDB___db_panic (dbmfp->dbmp->dbenv, EINVAL);
        goto err;
      }
      F_SET (&cmpr, DB_CMPR_CHAIN);
      if ((ret =
           CDB___memp_cmpr_alloc (dbmfp, &cmpr.next, bhp,
                                  &first_nonreused_chain_pos)) != 0)
        goto err;
      CDB___memp_cmpr_alloc_chain (dbmfp->dbmp, bhp, BH_CMPR_OS);
      bhp->chain[chain_length - 1] = cmpr.next;
    }
    /* write in the cmpr header */
    memcpy (db_io->buf, &cmpr, DB_CMPR_OVERHEAD);
    /* write in what's left of the compressed buffer (and that also fits in) */
    memcpy (db_io->buf + DB_CMPR_OVERHEAD, buffp, copy_length);
    buffp += copy_length;
    /* actual output  */
    if ((ret = CDB___os_io (db_io, DB_IO_WRITE, niop)) != 0 ||
        *niop != db_io->pagesize)
    {
      ret = EIO;
      goto err;
    }
    db_io->pgno = cmpr.next;
    cmpr.flags = DB_CMPR_INTERNAL;
    cmpr.next = 0;
  }
  while ((unsigned int) (buffp - buffcmpr) < buffcmpr_length);

#ifdef DEBUG_CMPR
  fprintf (stderr,
           "CDB___memp_cmpr_write:: chain_length (number of overflow pages):%2d\n",
           chain_length);
  if (chain_length > 0)
  {
    int i;
    fprintf (stderr, "CDB___memp_cmpr_write:: chain ");
    for (i = 0; i < chain_length; i++)
      fprintf (stderr, "%d, ", bhp->chain[i]);
    fprintf (stderr, "\n");
  }
#endif
  /*
   * If the chain was not completely reused, free the remaining pages (the page compression
   * rate is better).
   */
  if (F_ISSET (bhp, BH_CMPR) && first_nonreused_chain_pos >= 0)
  {
    int i;
    CMPR cmpr;
    cmpr.flags = DB_CMPR_FREE;
    cmpr.next = 0;
    memcpy (db_io->buf, &cmpr, sizeof (CMPR));
    for (i = first_nonreused_chain_pos; i < (CMPR_MAX - 1) && bhp->chain[i];
         i++)
    {
      if ((ret = CDB___memp_cmpr_free (dbmfp, bhp->chain[i])) != 0)
        goto err;
      /*
       * Mark the page as free for recovery.
       */
      db_io->pgno = bhp->chain[i];
      if ((ret = CDB___os_io (db_io, DB_IO_WRITE, niop)) != 0 ||
          *niop != db_io->pagesize)
      {
        ret = EIO;
        goto err;
      }
      bhp->chain[i] = 0;
    }
  }

  CDB___memp_cmpr_free_chain (dbmfp->dbmp, bhp);

  /*
   * In case of success, always pretend that we exactly wrote the
   * all bytes of the original pagesize.
   */
  *niop = CMPR_MULTIPLY (db_io->pagesize);

err:
  CDB___os_free (db_io->buf, 0);
  db_io->buf = orig_buff;
  if (buffcmpr)
    CDB___os_free (buffcmpr, 0);

  return ret;
}

/*
 * Helpers
 */

/*
 * CDB___memp_cmpr_page --
 *  Build a fake page. This function is a CDB___memp_cmpr_read helper.
 *
 */
static int
CDB___memp_cmpr_page (dbmfp, cmpr, db_io, niop)
     DB_MPOOLFILE *dbmfp;
     CMPR *cmpr;
     DB_IO *db_io;
     ssize_t *niop;
{
  DB_ENV *dbenv = dbmfp->dbmp->dbenv;
  int ret = 0;
  PAGE page;

  memset ((char *) &page, '\0', sizeof (PAGE));

  page.pgno = db_io->pgno;
  page.type = F_ISSET (cmpr, DB_CMPR_FREE) ? P_CMPR_FREE : P_CMPR_INTERNAL;

  /*
   * Sanity check
   */
  if (db_io->pagesize < sizeof (PAGE))
  {
    ret = ENOMEM;
    goto err;
  }

  memcpy (db_io->buf, (char *) &page, sizeof (PAGE));

  *niop = CMPR_MULTIPLY (db_io->pagesize);

err:

  return ret;
}

/*
 * CDB___memp_cmpr_inflate --
 *  Decompress buffer
 *
 * PUBLIC: int CDB___memp_cmpr_inflate __P((const u_int8_t *, int, u_int8_t *, int, void *));
 */
int
CDB___memp_cmpr_inflate (inbuff, inbuff_length, outbuff, outbuff_length,
                         user_data)
     const u_int8_t *inbuff;
     int inbuff_length;
     u_int8_t *outbuff;
     int outbuff_length;
     void *user_data;
{
#ifdef HAVE_LIBZ
  int ret = 0;
  z_stream c_stream;

  c_stream.zalloc = (alloc_func) 0;
  c_stream.zfree = (free_func) 0;
  c_stream.opaque = (voidpf) 0;
  c_stream.next_in = (Bytef *) inbuff;
  c_stream.avail_in = inbuff_length;
  c_stream.next_out = outbuff;
  c_stream.avail_out = outbuff_length;

  if (inflateInit (&c_stream) != Z_OK ||
      inflate (&c_stream, Z_FINISH) != Z_STREAM_END ||
      inflateEnd (&c_stream) != Z_OK)
    ret = EIO;

  /*
   * The uncompressed data must *exactly* fill outbuff_length.
   */
  if (c_stream.avail_out != 0)
    ret = EIO;

  return ret;
#else /* HAVE_LIBZ */
  return EINVAL;
#endif /* HAVE_LIBZ */
}



/*
 * CDB___memp_cmpr_deflate --
 *  Compress buffer
 *
 * PUBLIC: int CDB___memp_cmpr_deflate __P((const u_int8_t *, int, u_int8_t **, int*, void *));
 */
int
CDB___memp_cmpr_deflate (inbuff, inbuff_length, outbuffp, outbuff_lengthp,
                         user_data)
     const u_int8_t *inbuff;
     int inbuff_length;
     u_int8_t **outbuffp;
     int *outbuff_lengthp;
     void *user_data;
{
#ifdef HAVE_LIBZ
  int ret = 0;
  int r;
  int off = 0;
  int freesp = 0;
  z_stream c_stream;
  u_int8_t *outbuff;

  /*
   * Z_FINISH can be used immediately after deflateInit if all the compression
   * is to be done in a single step. In this case, avail_out must be at least
   * 0.1% larger than avail_in plus 12 bytes.  If deflate does not return
   * Z_STREAM_END, then it must be called again as described above.
   *
   * !!!
   * In order to avoid division by 1000, divide by 512 (2^9) using shift.
   * That is, make the buffer 0.2% larger. 
   */
  int outbuff_length = inbuff_length + (inbuff_length >> 9) + 12;

  *outbuffp = 0;
  *outbuff_lengthp = 0;

  if (CDB___os_malloc (outbuff_length, NULL, &outbuff) != 0)
  {
    ret = ENOMEM;
    goto err;
  }

  /*
   * Clear possible garbage in the page
   */
  {
    PAGE *pg = (PAGE *) inbuff;
    switch (TYPE (pg))
    {
    case P_IBTREE:
    case P_LBTREE:
      off = LOFFSET (pg);
      freesp = P_FREESPACE (pg);
      memset ((char *) (inbuff + off), 0, freesp);
      break;
    }
  }

  c_stream.zalloc = (alloc_func) 0;
  c_stream.zfree = (free_func) 0;
  c_stream.opaque = (voidpf) 0;

  if (deflateInit (&c_stream, memp_cmpr_zlib_level) != Z_OK)
  {
    ret = EIO;
    goto err;
  }

  c_stream.next_in = (Bytef *) inbuff;
  c_stream.avail_in = inbuff_length;
  c_stream.next_out = outbuff;
  c_stream.avail_out = outbuff_length;

  while ((r = deflate (&c_stream, Z_FINISH)) != Z_STREAM_END && r == Z_OK)
    ;

  if (r != Z_STREAM_END)
    ret = EIO;

  if (deflateEnd (&c_stream) != Z_OK)
    ret = EIO;

  if (ret == 0)
  {
    *outbuffp = outbuff;
    *outbuff_lengthp = outbuff_length - c_stream.avail_out;
  }
  else
  {
    CDB___os_free (outbuff, outbuff_length);
  }
#ifdef DEBUG_CMPR
  fprintf (stderr, "CDB___memp_cmpr_deflate:: compress %d bytes to %d \n",
           inbuff_length, *outbuff_lengthp);
#endif

err:
  return ret;
#else /* HAVE_LIBZ */
  return EINVAL;
#endif /* HAVE_LIBZ */
}



/*
 * CDB___memp_cmpr_info_valid --
 *  Compute compressed page size
 */
static int
CDB___memp_cmpr_info_valid (dbenv, cmpr_info)
     DB_ENV *dbenv;
     DB_CMPR_INFO *cmpr_info;
{
  int ret = 0;
  if (!cmpr_info)
  {
    CDB___db_err (dbenv, "CDB___memp_cmpr_info_valid: cmpr_info == NULL");
    ret = CDB___db_panic (dbenv, EINVAL);
    goto err;
  }

  if (!cmpr_info->compress)
  {
    CDB___db_err (dbenv, "CDB___memp_cmpr_info_valid: compress == NULL!");
    ret = CDB___db_panic (dbenv, EINVAL);
    goto err;
  }

  if (!cmpr_info->uncompress)
  {
    CDB___db_err (dbenv, "CDB___memp_cmpr_info_valid: uncompress == NULL!");
    ret = CDB___db_panic (dbenv, EINVAL);
    goto err;
  }

  if (cmpr_info->coefficient == 0 || cmpr_info->coefficient > 5)
  {
    CDB___db_err (dbenv,
                  "CDB___memp_cmpr_info_valid:  coefficient should be > 0 and < 5 coefficient=%d ",
                  cmpr_info->coefficient);
    ret = CDB___db_panic (dbenv, EINVAL);
    goto err;
  }

  if (cmpr_info->max_npages == 0 || cmpr_info->max_npages > 128)
  {
    CDB___db_err (dbenv,
                  "CDB___memp_cmpr_info_valid:  max_npages should be > 0 and < 128 max_npages=%d ",
                  cmpr_info->max_npages);
    ret = CDB___db_panic (dbenv, EINVAL);
    goto err;
  }
err:
  return ret;
}

/*
 * __memp_cmpr_pagesize --
 *  Compute compressed page size
 *
 * PUBLIC: u_int8_t CDB___memp_cmpr_coefficient __P((DB_ENV *dbenv));
 */
u_int8_t
CDB___memp_cmpr_coefficient (dbenv)
     DB_ENV *dbenv;
{
  u_int8_t ret = 0;

  if (!dbenv || !dbenv->mp_cmpr_info)
  {
    ret = default_cmpr_info.coefficient;
  }
  else
  {
    CDB___memp_cmpr_info_valid (dbenv, dbenv->mp_cmpr_info);
    ret = dbenv->mp_cmpr_info->coefficient;
  }

  return (ret);
}

/*
 * Initialisation of page compression
 */

/*
 * CDB___memp_cmpr_open --
 *  Open the db that contains the free compression pages.
 *
 * PUBLIC: int CDB___memp_cmpr_open __P((const char *, DB_ENV *, CMPR_CONTEXT *));
 */
int
CDB___memp_cmpr_open (dbenv, path, flags, mode, cmpr_context)
     DB_ENV *dbenv;
     const char *path;
     int flags;
     int mode;
     CMPR_CONTEXT *cmpr_context;
{
  int ret;
  char *tmp = 0;
  int tmp_length = strlen (path) + strlen (DB_CMPR_SUFFIX) + 1;

  /*
   * Management of pages containing data when the compression does not achieve
   * the expected compression ratio.
   */
  {
    DB *dbp;
    if ((ret = CDB___os_malloc (tmp_length, NULL, &tmp)) != 0)
      goto err;
    sprintf (tmp, "%s%s", path, DB_CMPR_SUFFIX);

    /* Use *standalone* database, to prevent recursion when writing pages */
    /* from the cache, shared with other members of the environment */
    if (CDB_db_create (&dbp, NULL, 0) != 0)
      goto err;

    cmpr_context->weakcmpr = dbp;

    (dbp->set_flags) (dbp, DB_RECNUM);

    LF_CLR (DB_COMPRESS);
    if (!LF_ISSET (DB_RDONLY))
      LF_SET (DB_CREATE);
    if ((ret = (dbp->open) (dbp, tmp, NULL, DB_BTREE, flags, mode)) != 0)
      goto err;
  }

  /*
   * Initialisation of cmpr_context
   */
  if (!dbenv->mp_cmpr_info)
  {
    if (default_cmpr_info.compress == 0)
    {
      CDB___db_err (dbenv,
                    "CDB___memp_cmpr_open: zlib compression not available, re-compile --with-zlib=DIR");
      ret = CDB___db_panic (dbenv, EINVAL);
      goto err;
    }
    dbenv->mp_cmpr_info = &default_cmpr_info;
  }
  /*
   * Check if cmpr_info is sane
   */
  if ((ret = CDB___memp_cmpr_info_valid (dbenv, dbenv->mp_cmpr_info)))
    goto err;

err:
  if (tmp)
    CDB___os_free (tmp, tmp_length);
  return ret;
}

/*
 * CDB___memp_cmpr_close --
 *  Close the db that contains the free compression pages.
 *
 * PUBLIC: int CDB___memp_cmpr_close __P((CMPR_CONTEXT *));
 */
int
CDB___memp_cmpr_close (cmpr_context)
     CMPR_CONTEXT *cmpr_context;
{
  int ret = 0;

  if (cmpr_context->weakcmpr == 0)
  {
    ret = EINVAL;
    goto err;
  }

  if ((ret = cmpr_context->weakcmpr->close (cmpr_context->weakcmpr, 0)) != 0)
    goto err;
  cmpr_context->weakcmpr = 0;

err:
  return ret;
}

/*
 * CDB___memp_cmpr_alloc --
 *  Get a new free page to store weak compression data.
 *
 * PUBLIC: int CDB___memp_cmpr_alloc __P((DB_MPOOLFILE *, db_pgno_t *, BH *, int *));
 */
int
CDB___memp_cmpr_alloc (dbmfp, pgnop, bhp, first_nonreused_chain_posp)
     DB_MPOOLFILE *dbmfp;
     db_pgno_t *pgnop;
     BH *bhp;
     int *first_nonreused_chain_posp;
{
  DB_ENV *dbenv = dbmfp->dbmp->dbenv;
  int ret = 0;

#ifdef DEBUG_CMPR
  fprintf (stderr,
           "CDB___memp_cmpr_alloc:: bhp:%8x bhp->chain:%8x  first_nonreused_chain_posp:%2d\n",
           bhp, bhp->chain, *first_nonreused_chain_posp);
#endif
  if (F_ISSET (bhp, BH_CMPR) && bhp->chain == NULL)
  {
    CDB___db_err (dbenv,
                  "CDB___memp_cmpr_alloc: BH_CMPR set and bhp->chain == NULL");
    ret = CDB___db_panic (dbenv, EINVAL);
    goto err;
  }

  if ((*first_nonreused_chain_posp) >= (CMPR_MAX - 1))
  {
    CDB___db_err (dbenv,
                  "CDB___memp_cmpr_alloc: first_nonreused_chain_pos >= (CMPR_MAX - 1)");
    ret = CDB___db_panic (dbenv, EINVAL);
    goto err;
  }

  /*
   * If possible reuse an existing chain.
   */
  if ((*first_nonreused_chain_posp) >= 0 && F_ISSET (bhp, BH_CMPR)
      && bhp->chain[*first_nonreused_chain_posp])
  {
    *pgnop = bhp->chain[*first_nonreused_chain_posp];
    (*first_nonreused_chain_posp)++;
#ifdef DEBUG_CMPR
    fprintf (stderr, "CDB___memp_cmpr_alloc:: reusing page in chain \n");
#endif
  }
  else
  {
    DB *db = dbmfp->cmpr_context.weakcmpr;
    DBT key;
    DBT data;
    db_recno_t recno = 1;

    /* all pages in bhp->chain are now reused */
    (*first_nonreused_chain_posp) = -1;
#ifdef DEBUG_CMPR
    fprintf (stderr,
             "CDB___memp_cmpr_alloc:: no more reusable pages in chain\n");
#endif

    if (db == 0)
    {
      CDB___db_err (dbenv,
                    "CDB___memp_cmpr_alloc: dbmfp->cmpr_context.weakcmpr is null");
      ret = CDB___db_panic (dbenv, EINVAL);
      goto err;
    }

    /*
     * If the existing chain is too short, pop a free page from
     * the free pages database.
     */
    memset (&key, '\0', sizeof (DBT));
    memset (&data, '\0', sizeof (DBT));

    key.data = &recno;
    key.size = sizeof (recno);

    if ((ret = db->get (db, NULL, &key, &data, DB_SET_RECNO)) != 0)
    {
      /*
       * If the free list is empty, create a new page.
       */
#ifdef DEBUG_CMPR
      fprintf (stderr,
               "CDB___memp_cmpr_alloc:: weakcmpr free page pool empty, allocating\n");
#endif
      if (ret == DB_NOTFOUND)
      {
        DB_MPOOL *dbmp = dbmfp->dbmp;
        ret = 0;
        R_LOCK (dbenv, &dbmp->reginfo);
        ++dbmfp->mfp->last_pgno;
#ifdef DEBUG
        word_monitor_set (WORD_MONITOR_PGNO, dbmfp->mfp->last_pgno);
#endif /* DEBUG */
        *pgnop = dbmfp->mfp->last_pgno;
        R_UNLOCK (dbenv, &dbmp->reginfo);
        ret = 0;
      }
      else
      {
        CDB___db_err (dbenv,
                      "CDB___memp_cmpr_alloc: unexpected error from weakcmpr base");
        ret = CDB___db_panic (dbenv, ret);
      }
    }
    else
    {
      if (key.size != sizeof (db_pgno_t))
      {
        CDB___db_err (dbenv,
                      "CDB___memp_cmpr_alloc: unexpected key size from weakcmpr base (%d instead of %d)",
                      key.size, sizeof (db_pgno_t));
        ret = CDB___db_panic (dbenv, ret);
      }
      else
      {
        memcpy ((char *) pgnop, (char *) key.data, key.size);
        if ((ret = db->del (db, NULL, &key, 0)) != 0)
        {
          CDB___db_err (dbenv,
                        "CDB___memp_cmpr_alloc: del error, got pgno %d",
                        *pgnop);
          ret = CDB___db_panic (dbenv, ret);
        }
        if (*pgnop == 0)
        {
          CDB___db_err (dbenv, "CDB___memp_cmpr_alloc: unexpected pgno == 0");
          ret = CDB___db_panic (dbenv, ret);
        }
      }
#ifdef DEBUG_CMPR
      fprintf (stderr,
               "CDB___memp_cmpr_alloc:: reuse free page %d from weakcmpr\n",
               *pgnop);
#endif
    }
  }

err:
  return ret;
}

/*
 * CDB___memp_cmpr_free --
 *  Release a page used to store weak compression data.
 *
 * PUBLIC: int CDB___memp_cmpr_free __P((DB_MPOOLFILE *, db_pgno_t));
 */
int
CDB___memp_cmpr_free (dbmfp, pgno)
     DB_MPOOLFILE *dbmfp;
     db_pgno_t pgno;
{
  int ret = 0;
  DB_ENV *dbenv = dbmfp->dbmp->dbenv;
  DB *db = dbmfp->cmpr_context.weakcmpr;
  DBT key;
  DBT data;

#ifdef DEBUG_CMPR
  fprintf (stderr,
           "CDB___memp_cmpr_free::  freeing page (inserting into weakcmpr):%3d \n",
           pgno);
#endif
  if (db == 0)
  {
    CDB___db_err (dbenv,
                  "CDB___memp_cmpr_free: dbmfp->cmpr_context.weakcmpr is null");
    ret = CDB___db_panic (dbenv, EINVAL);
    goto err;
  }

  memset (&key, '\0', sizeof (DBT));
  memset (&data, '\0', sizeof (DBT));

  key.data = &pgno;
  key.size = sizeof (db_pgno_t);

  data.data = " ";
  data.size = 1;

  if ((ret = db->put (db, 0, &key, &data, DB_NOOVERWRITE)) != 0)
  {
    CDB___db_err (dbenv, "CDB___memp_cmpr_free: put failed for pgno = %d",
                  pgno);
    ret = CDB___db_panic (dbenv, ret);
    goto err;
  }

err:
  return ret;
}


/*
 * CDB___memp_cmpr_alloc_chain --
 *  Allocate chain entry in BH
 *
 * PUBLIC: int CDB___memp_cmpr_alloc_chain __P((DB_MPOOL *, BH *));
 */

int
CDB___memp_cmpr_alloc_chain (dbmp, bhp, alloc_type)
     DB_MPOOL *dbmp;
     BH *bhp;
     int alloc_type;
{
  DB_ENV *dbenv = dbmp->dbenv;
  int ret = 0;
  if (!bhp->chain)
  {
    int alloc_ret;
    int alloc_length = sizeof (db_pgno_t) * (CMPR_MAX - 1);
    switch (alloc_type)
    {
    case BH_CMPR_POOL:
      {
        MPOOL *mp = dbmp->reginfo.primary;
        int n_cache = NCACHE (mp, bhp->pgno);
        alloc_ret =
          CDB___memp_alloc (dbmp, &dbmp->c_reginfo[n_cache], NULL,
                            alloc_length, NULL, (void *) (&bhp->chain));
        F_SET (bhp, BH_CMPR_POOL);
      }
      break;
    case BH_CMPR_OS:
      alloc_ret = CDB___os_malloc (alloc_length, NULL, &bhp->chain);
      F_SET (bhp, BH_CMPR_OS);
      break;
    default:
      CDB___db_err (dbenv,
                    "CDB___memp_cmpr_alloc_chain: unknown alloc type :%d",
                    alloc_type);
      ret = CDB___db_panic (dbenv, EINVAL);
      goto err;
      break;
    }

    if (alloc_ret)
    {
      CDB___db_err (dbenv,
                    "CDB___memp_cmpr_alloc_chain: memp_alloc %d bytes failed:%d",
                    alloc_length, alloc_ret);
      ret = CDB___db_panic (dbenv, EINVAL);
      goto err;
    }
    memset ((void *) bhp->chain, 0, alloc_length);
#if defined(DEBUG_CMPR) || defined(DEBUG_CMPR_ALLOC)
    fprintf (stderr,
             "CDB___memp_cmpr_alloc_chain:: allocate chain in %s :%8x\n",
             (alloc_type == BH_CMPR_OS ? "malloc" : "shalloc"), bhp->chain);
#endif
  }
  else
  {
#ifdef DEBUG_CMPR
    fprintf (stderr, "CDB___memp_cmpr_alloc_chain:: existing chain:%8x\n",
             bhp->chain);
#endif
  }
  F_SET (bhp, BH_CMPR);
err:
  return ret;
}

/*
 * CDB___memp_cmpr_free_chain --
 *  Free chain entry in BH
 *
 * PUBLIC: int CDB___memp_cmpr_free_chain __P((DB_MPOOL *, BH *));
 */

int
CDB___memp_cmpr_free_chain (dbmp, bhp)
     DB_MPOOL *dbmp;
     BH *bhp;
{
  DB_ENV *dbenv = dbmp->dbenv;

  if (F_ISSET (bhp, BH_CMPR))
  {
    if (bhp->chain)
    {
      int alloc_length = sizeof (db_pgno_t) * (CMPR_MAX - 1);
      int alloc_type = bhp->flags & (BH_CMPR_POOL | BH_CMPR_OS);
      switch (alloc_type)
      {
      case BH_CMPR_POOL:
        {
          MPOOL *mp = dbmp->reginfo.primary;
          int n_cache = NCACHE (mp, bhp->pgno);
          CDB___db_shalloc_free (dbmp->c_reginfo[n_cache].addr, bhp->chain);
        }
        break;
      case BH_CMPR_OS:
        CDB___os_free (bhp->chain, alloc_length);
        break;
      default:
        CDB___db_err (dbenv,
                      "CDB___memp_cmpr_free_chain: unknown alloc type :%d",
                      alloc_type);
        return CDB___db_panic (dbenv, EINVAL);
        break;
      }
#if defined(DEBUG_CMPR) || defined(DEBUG_CMPR_ALLOC)
      fprintf (stderr, "CDB___memp_cmpr_free_chain:: free chain in %s :%8x\n",
               (alloc_type == BH_CMPR_OS ? "malloc" : "shalloc"), bhp->chain);
#endif
      bhp->chain = NULL;
    }
    else
    {
      CDB___db_err (dbenv,
                    "CDB___memp_cmpr_free_chain: BH_CMPR set but null bhp->chain");
      return CDB___db_panic (dbenv, EINVAL);
    }
  }
  else if (bhp->chain)
  {
    CDB___db_err (dbenv,
                  "CDB___memp_cmpr_free_chain: BH_CMPR not set but bhp->chain not null");
    return CDB___db_panic (dbenv, EINVAL);
  }

  F_CLR (bhp, BH_CMPR | BH_CMPR_OS | BH_CMPR_POOL);

  return 0;
}
