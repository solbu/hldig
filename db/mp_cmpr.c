/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1999, 2000
 *	Loic Dachary.  All rights reserved.
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
 *  In CDB___memp_cmpr, niop is always multiplied by compression factor for page 0. 
 *  I see no problems with this but it's a bit awkward.
 *
 *  In __memp_cmpr_page, the page built fills some fields of the PAGE structure
 *  others are set to 0. I'm not 100% sure this is enough. It should only impact
 *  utilities that read pages by incrementing pgno. Only stat does this an it's
 *  enough for it. I've not found any other context where these fake pages are 
 *  used.
 * 
 */

#include "config.h"

#ifndef lint
static const char sccsid[] = "@(#)mp_cmpr.c	1.1 (Senga) 01/08/99";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
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

/*
 * Helpers declarations.
 */
static int __memp_cmpr_page __P((DB_MPOOLFILE *, CMPR *, DB_IO *, ssize_t *));

/*
 * Maximum chain length
 */
#define CMPR_MAX	(dbenv->mp_cmpr_info->max_npages)

#define CMPR_MULTIPLY(n) ((n) << (dbenv->mp_cmpr_info->coefficient))
#define CMPR_DIVIDE(n)   ((n) >> (dbenv->mp_cmpr_info->coefficient))

#ifdef HAVE_LIBZ
static DB_CMPR_INFO default_cmpr_info = {
    CDB___memp_cmpr_deflate,    
    CDB___memp_cmpr_inflate,    
    1,
    3,
    NULL
};
#else /* HAVE_LIBZ */
static DB_CMPR_INFO default_cmpr_info = {
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
 *	Transparent compression read/write
 *
 * PUBLIC: int CDB___memp_cmpr __P((DB_MPOOLFILE *, BH *, DB_IO *, int, ssize_t *));
 */
int
CDB___memp_cmpr(dbmfp, bhp, db_io, flag, niop)
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
  int ret = 0;

  db_io->pagesize = CMPR_DIVIDE(db_io->pagesize);
  db_io->bytes = CMPR_DIVIDE(db_io->bytes);

  /*
   * Page 0 is a special case. It contains the metadata information
   * (at most 256 bytes) and must not be compressed because it is read
   * with CDB___os_read and not CDB___os_io. This read is done before
   * any memory pool structure is initialized, we therefore have no
   * chance to trap it anywhere but here. 
   */
  switch (flag) {
  case DB_IO_READ:
    if(db_io->pgno == PGNO_BASE_MD) {
      ret = CDB___os_io(dbenv, db_io, DB_IO_READ, niop);
      *niop = CMPR_MULTIPLY(*niop);
    } else 
      ret = CDB___memp_cmpr_read(dbmfp, bhp, db_io, niop);
    break;
  case DB_IO_WRITE:
    if(db_io->pgno == PGNO_BASE_MD) {
      /*
       * Write a copy of the DBMETA information at 
       * 256, 512, 1024, 2048, 4096, ... up to the actually required page size.
       * This ensure that the DBMETA information will be found without knowing
       * the actual page size used in the file. 
       * !! Assume that PGNO_BASE_MD == 1
       */
      size_t required = db_io->pagesize;
      size_t orig_bytes = db_io->bytes;
      db_io->bytes = DBMETASIZE;
      for(db_io->pagesize = DBMETASIZE; db_io->pagesize < required; db_io->pagesize <<= 1) {
	ret = CDB___os_io(dbenv, db_io, DB_IO_WRITE, niop);
	if(ret != 0 || *niop != DBMETASIZE)
	  break;
      }
      db_io->bytes = orig_bytes;
      db_io->pagesize = required;
      if(ret == 0)
	ret = CDB___os_io(dbenv, db_io, DB_IO_WRITE, niop);
      *niop = CMPR_MULTIPLY(*niop);
    } else
      ret = CDB___memp_cmpr_write(dbmfp, bhp, db_io, niop);
    break;
  }

  db_io->pgno = orig_pgno;
  db_io->pagesize = orig_pagesize;
  db_io->bytes = orig_bytes;

  return ret;
}

/*
 * CDB___memp_cmpr_read_meta --
 *	Transparent compression read page containing meta header
 */
int
CDB___memp_cmpr_read_meta(dbenv, fhp, buff, buff_length, nrp)
     DB_ENV *dbenv;
     DB_FH *fhp;
     void *buff;
     size_t buff_length;
     ssize_t *nrp;
{
  CMPR cmpr;
  int ret;
  int i;

  if((ret = CDB___os_read(dbenv, fhp, buff, buff_length, nrp)) != 0)
    goto err;

  if(*nrp != buff_length)
    goto err;
  
  /*
   * Read the cmpr header from page.
   */
  memcpy(&cmpr, buff, sizeof(CMPR));

  /*
   * If not at the beginning of compressed page chain, build
   * a fake page.
   */
  if(F_ISSET(&cmpr, DB_CMPR_FREE) || F_ISSET(&cmpr, DB_CMPR_INTERNAL)) {
    ret = CDB___db_panic(dbenv, EINVAL);
    goto err;
  }

  for(i = 0; i < buff_length - (DB_CMPR_OVERHEAD + 1); i++)
    ((char*)buff)[i] = ((char*)buff)[i + (DB_CMPR_OVERHEAD + 1)];

 err:
  return ret;
}

/*
 * CDB___memp_cmpr_read --
 *	Transparent compression read
 *
 * PUBLIC: int CDB___memp_cmpr_read __P((DB_MPOOLFILE *, BH *, DB_IO *, ssize_t *));
 */
int
CDB___memp_cmpr_read(dbmfp, bhp, db_io, niop)
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
  F_CLR(bhp, BH_CMPR);

  /*
   * Read first page (if no overflow, this is the only one)
   */
  ret = CDB___os_io(dbenv, db_io, DB_IO_READ, niop);

  /*
   * An error or partial read on the first page means that we're not
   * going anywhere.
   */
  if(ret || *niop < db_io->pagesize)
    goto err;

  /*
   * Read the cmpr header from page.
   */
  memcpy(&cmpr, db_io->buf, sizeof(CMPR));

  /*
   * If not at the beginning of compressed page chain, build
   * a fake page.
   */
  if(F_ISSET(&cmpr, DB_CMPR_FREE) || F_ISSET(&cmpr, DB_CMPR_INTERNAL)) {
    ret = __memp_cmpr_page(dbmfp, &cmpr, db_io, niop);
    goto err;
  }

  /*
   * Sanity check. Happens if file corrupted.
   */
  if(!F_ISSET(&cmpr, DB_CMPR_FIRST)) {
    CDB___db_err(dbmfp->dbmp->dbenv, "CDB___memp_cmpr_read: expected DB_CMPR_FIRST flag set at pgno = %ld", db_io->pgno);
    ret = CDB___db_panic(dbmfp->dbmp->dbenv, EINVAL);
    goto err;
  }
  
  if((ret = CDB___os_malloc(dbenv, db_io->pagesize * CMPR_MAX, NULL, &buffcmpr)) != 0)
    goto err;

  do {
    /*
     * Read the first part of the compressed data from page.
     */
    memcpy(buffcmpr + buffcmpr_length, DB_CMPR_DATA(db_io), DB_CMPR_PAGESIZE(db_io));
    buffcmpr_length += DB_CMPR_PAGESIZE(db_io);
    
    /*
     * Flag must only contain FIRST|INTERNAL and/or CHAIN. If other bits are
     * set, the data is corrupted. Removing the FIRST|INTERNAL bits and checking
     * the CHAIN bit with == instead of F_ISSET verify this.
     */
    F_CLR(&cmpr, DB_CMPR_FIRST | DB_CMPR_INTERNAL);
    chain = cmpr.flags;

    if(chain == DB_CMPR_CHAIN) {
      /*
       * Overflow Case. Continue reading data from extra pages.
       */

      chain_length++;
      if(chain_length >= CMPR_MAX) {
	CDB___db_err(dbmfp->dbmp->dbenv, "CDB___memp_cmpr_read: compression chain too long at pgno = %ld", db_io->pgno);
	ret = CDB___db_panic(dbmfp->dbmp->dbenv, EINVAL);
	goto err;
      }

      if(cmpr.next == 0) {
	CDB___db_err(dbmfp->dbmp->dbenv, "CDB___memp_cmpr_read: cmpr.next is null at pgno = %ld", chain, db_io->pgno);
	ret = CDB___db_panic(dbmfp->dbmp->dbenv, EINVAL);
	goto err;
      }
      /*
       * Keep the chain in buffer header.
       */
      CDB___memp_cmpr_alloc_chain(dbmfp->dbmp, bhp, BH_CMPR_POOL);

      bhp->chain[chain_length - 1] = cmpr.next;
      db_io->pgno = cmpr.next;
      /*
       * Read data from extra page.
       */
      if((ret = CDB___os_io(dbenv, db_io, DB_IO_READ, niop)) != 0 ||
	 *niop != db_io->pagesize) {
	ret = EIO;
	goto err;
      }
      /*
       * Read the cmpr header from this extra page
       */
      memcpy(&cmpr, db_io->buf, sizeof(CMPR));
    } else if(chain != 0) {
      CDB___db_err(dbmfp->dbmp->dbenv, "CDB___memp_cmpr_read: unexpected compression flag value 0x%x at pgno = %ld", chain, db_io->pgno);
      ret = CDB___db_panic(dbmfp->dbmp->dbenv, ret);
      goto err;
    } else if(cmpr.next != 0) {
      CDB___db_err(dbmfp->dbmp->dbenv, "CDB___memp_cmpr_read: cmpr.next is not null at pgno = %ld", chain, db_io->pgno);
      ret = CDB___db_panic(dbmfp->dbmp->dbenv, ret);
      goto err;
    }
  } while(chain);
  
  /*
   * We gathered all the compressed data in buffcmpr, inflate it.
   */
  {
    switch((*buffcmpr) & TYPE_MASK) {
    case P_HASHMETA:
    case P_BTREEMETA:
    case P_QAMMETA:
    case P_INVALID:
      memcpy(db_io->buf, buffcmpr + sizeof(char), 255);
      break;
    default:
      if((ret = (*cmpr_info->uncompress)(dbenv, buffcmpr, buffcmpr_length, db_io->buf, CMPR_MULTIPLY(db_io->pagesize), cmpr_info->user_data)) != 0) {
	CDB___db_err(dbmfp->dbmp->dbenv, "CDB___memp_cmpr_read: unable to uncompress page at pgno = %ld", first_pgno);
	ret = CDB___db_panic(dbmfp->dbmp->dbenv, ret);
	goto err;
      }
      break;
    }
  }
#ifdef DEBUG
  {
    int ratio = buffcmpr_length > 0 ? (CMPR_MULTIPLY(db_io->pagesize) / buffcmpr_length) : 0;
    if(ratio > 10) ratio = 10;
    word_monitor_add(DB_MONITOR(dbenv), WORD_MONITOR_COMPRESS_01 + ratio, 1);
  }
#endif /* DEBUG */

  *niop = CMPR_MULTIPLY(db_io->pagesize);

 err:
#ifdef DEBUG_CMPR
  if(chain_length > 0) {
    int i;
    fprintf(stderr,"CDB___memp_cmpr_read:: chain_length (number of overflow pages):%2d\n",chain_length);
    fprintf(stderr,"CDB___memp_cmpr_read:: chain ");
    for(i = 0; i < chain_length; i++)
      fprintf(stderr, "%d, ", bhp->chain[i]);
    fprintf(stderr, "\n");
  }
#endif
  if(buffcmpr) CDB___os_free(buffcmpr, 0);
  return ret;
}

/*
 * CDB___memp_cmpr_write --
 *	Transparent compression write
 *
 * PUBLIC: int CDB___memp_cmpr_write __P((DB_MPOOLFILE *, BH *, DB_IO *, ssize_t *));
 */
int
CDB___memp_cmpr_write(dbmfp, bhp, db_io, niop)
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
  int buffcmpr_length;
  u_int8_t *orig_buff = db_io->buf;
  DB_ENV *dbenv = dbmfp->dbmp->dbenv;
  DB_CMPR_INFO *cmpr_info = dbenv->mp_cmpr_info;

  if((ret = CDB___os_malloc(dbenv, CMPR_MULTIPLY(db_io->bytes), NULL, &db_io->buf)) != 0)
    goto err;

  /*
   * Call the compression function, except for META pages (at most 256 bytes)
   */
  {
    PAGE* pp = (PAGE*)orig_buff;
    switch(TYPE(pp)) {
    case P_HASHMETA:
    case P_BTREEMETA:
    case P_QAMMETA:
    case P_INVALID:
      /*
       * Compressed meta is type byte + 255 bytes (the largest META info
       * is smaller than 255 bytes).
       */
      buffcmpr_length = 256;
      if ((ret = CDB___os_malloc(dbenv, buffcmpr_length, NULL, &buffcmpr)) != 0)
	goto err;
      buffcmpr[0] = TYPE_TAGS(pp);
      memcpy(buffcmpr + 1, orig_buff, 255);
      break;
    default:
      if((ret = (*cmpr_info->compress)(dbenv, orig_buff, CMPR_MULTIPLY(db_io->pagesize), &buffcmpr, &buffcmpr_length, cmpr_info->user_data)) != 0) {
	CDB___db_err(dbmfp->dbmp->dbenv, "CDB___memp_cmpr_write: unable to compress page at pgno = %ld", db_io->pgno);
	ret = CDB___db_panic(dbmfp->dbmp->dbenv, ret);
	goto err;
      }
    }
  }
#ifdef DEBUG
  {
    int ratio = buffcmpr_length > 0 ? (CMPR_MULTIPLY(db_io->pagesize) / buffcmpr_length) : 0;
    if(ratio > 10) ratio = 10;
    word_monitor_add(DB_MONITOR(dbenv), WORD_MONITOR_COMPRESS_01 + ratio, 1);
  }
#endif /* DEBUG */

  /*
   * This can never happen.
   */
  if(buffcmpr_length > DB_CMPR_PAGESIZE(db_io) * CMPR_MAX) {
    CDB___db_err(dbmfp->dbmp->dbenv, "CDB___memp_cmpr_write: compressed data is too big at pgno = %ld", db_io->pgno);
    ret = CDB___db_panic(dbmfp->dbmp->dbenv, EINVAL);
    goto err;
  }

  buffp = buffcmpr;
  cmpr.flags = DB_CMPR_FIRST;
  cmpr.next = 0;

  /* write pages until the whole compressed data is written */ 
  do {
    int length = buffcmpr_length - (buffp - buffcmpr);
    int copy_length = length > DB_CMPR_PAGESIZE(db_io) ? DB_CMPR_PAGESIZE(db_io) : length;
    /*
     * We handle serious compression stuff only if we need to.
     * overflow! the compressed buffer is too big -> get extra page
     */
    if(length > copy_length) {
      chain_length++;
      if(chain_length >= CMPR_MAX) {
	  CDB___db_err(dbmfp->dbmp->dbenv, "CDB___memp_cmpr_write: chain_length overflow");
	  ret = CDB___db_panic(dbmfp->dbmp->dbenv, EINVAL);
	  goto err;
      }
      F_SET(&cmpr, DB_CMPR_CHAIN);
      if((ret = CDB___memp_cmpr_alloc(dbmfp, &cmpr.next, db_io->pagesize, bhp, &first_nonreused_chain_pos)) != 0)
	goto err;
      CDB___memp_cmpr_alloc_chain(dbmfp->dbmp, bhp, BH_CMPR_OS);
      bhp->chain[chain_length - 1] = cmpr.next;
    }
    /* write in the cmpr header */
    memcpy(db_io->buf, &cmpr, DB_CMPR_OVERHEAD);
    /* write in what's left of the compressed buffer (and that also fits in) */
    memcpy(db_io->buf + DB_CMPR_OVERHEAD, buffp, copy_length);
    buffp += copy_length;
    /* actual output  */
    if((ret = CDB___os_io(dbenv, db_io, DB_IO_WRITE, niop)) != 0 ||
       *niop != db_io->pagesize) {
      ret = EIO;
      goto err;
    }
    db_io->pgno = cmpr.next;
    cmpr.flags = DB_CMPR_INTERNAL;
    cmpr.next = 0;
  } while(buffp - buffcmpr < buffcmpr_length);

#ifdef DEBUG_CMPR
  fprintf(stderr,"CDB___memp_cmpr_write:: chain_length (number of overflow pages):%2d\n",chain_length);
  if(chain_length > 0) {
    int i;
    fprintf(stderr,"CDB___memp_cmpr_write:: chain ");
    for(i = 0; i < chain_length; i++)
      fprintf(stderr, "%d, ", bhp->chain[i]);
    fprintf(stderr, "\n");
  }
#endif
  /*
   * If the chain was not completely reused, free the remaining pages (the page compression
   * rate is better).
   */
  if(F_ISSET(bhp, BH_CMPR) && first_nonreused_chain_pos >= 0) {
    int i;
    for(i = first_nonreused_chain_pos; i < (CMPR_MAX - 1) && bhp->chain[i]; i++) {
      if((ret = CDB___memp_cmpr_free(dbmfp, bhp->chain[i], db_io->pagesize)) != 0)
	goto err;
      bhp->chain[i] = 0;
    }
  }
  
  CDB___memp_cmpr_free_chain(dbmfp->dbmp, bhp);

  /*
   * In case of success, always pretend that we exactly wrote the
   * all bytes of the original pagesize.
   */
  *niop = CMPR_MULTIPLY(db_io->pagesize);

 err:
  CDB___os_free(db_io->buf, 0);
  db_io->buf = orig_buff;
  if(buffcmpr) CDB___os_free(buffcmpr, 0);

  return ret;
}

/*
 * Helpers
 */

/*
 * __memp_cmpr_page --
 *	Build a fake page. This function is a CDB___memp_cmpr_read helper.
 *
 */
static int
__memp_cmpr_page(dbmfp, cmpr, db_io, niop)
     DB_MPOOLFILE *dbmfp;    
     CMPR *cmpr;
     DB_IO *db_io;
     ssize_t *niop;
{
  DB_ENV *dbenv = dbmfp->dbmp->dbenv;
  int ret = 0;
  PAGE page;

  memset((char*)&page, '\0', sizeof(PAGE));

  page.pgno = db_io->pgno;
  page.type = F_ISSET(cmpr, DB_CMPR_FREE) ? P_CMPR_FREE : P_CMPR_INTERNAL;

  /*
   * Sanity check
   */
  if(db_io->pagesize < sizeof(PAGE)) {
    ret = ENOMEM;
    goto err;
  }
  
  memcpy(db_io->buf, (char*)&page, sizeof(PAGE));

  *niop = CMPR_MULTIPLY(db_io->pagesize);

 err:

  return ret;
}

#ifdef HAVE_LIBZ
#include "zlib.h"
#endif /* HAVE_LIBZ */

/*
 * CDB___memp_cmpr_inflate --
 *	Decompress buffer
 *
 * PUBLIC: int CDB___memp_cmpr_inflate __P((const u_int8_t *, int, u_int8_t *, int, void *));
 */
int
CDB___memp_cmpr_inflate(dbenv, inbuff, inbuff_length, outbuff, outbuff_length, user_data)
     DB_ENV *dbenv;
     const u_int8_t* inbuff;
     int inbuff_length;
     u_int8_t* outbuff;
     int outbuff_length;
     void *user_data;
{
#ifdef HAVE_LIBZ
  int ret = 0;
  z_stream c_stream;

  c_stream.zalloc=(alloc_func)0;
  c_stream.zfree=(free_func)0;
  c_stream.opaque=(voidpf)0;
  c_stream.next_in = (Bytef*)inbuff;
  c_stream.avail_in = inbuff_length;
  c_stream.next_out = outbuff;
  c_stream.avail_out = outbuff_length;

  if(inflateInit(&c_stream) != Z_OK ||
     inflate(&c_stream, Z_FINISH) != Z_STREAM_END ||
     inflateEnd(&c_stream) != Z_OK)
    ret = EIO;

  /*
   * The uncompressed data must *exactly* fill outbuff_length.
   */
  if(c_stream.avail_out != 0)
    ret = EIO;

  return ret;
#else /* HAVE_LIBZ */
  return EINVAL;
#endif /* HAVE_LIBZ */
}



/*
 * CDB___memp_cmpr_deflate --
 *	Compress buffer
 *
 * PUBLIC: int CDB___memp_cmpr_deflate __P((const u_int8_t *, int, u_int8_t **, int*, void *));
 */
int
CDB___memp_cmpr_deflate(dbenv, inbuff, inbuff_length, outbuffp, outbuff_lengthp, user_data)
     DB_ENV* dbenv;
     const u_int8_t* inbuff;
     int inbuff_length;
     u_int8_t** outbuffp;
     int* outbuff_lengthp;
     void *user_data;
{
#ifdef HAVE_LIBZ
  int ret = 0;
  int r;
  z_stream c_stream;
  u_int8_t* outbuff;

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

  if(CDB___os_malloc(dbenv, outbuff_length, NULL, &outbuff) != 0) {
    ret = ENOMEM;
    goto err;
  }
  
  /*
   * Clear possible garbage in the page
   */
  {
    PAGE* pg = (PAGE*)inbuff;
    switch(TYPE(pg)) {
    case P_IBTREE:
    case P_LBTREE:
      memset((char*)(inbuff + LOFFSET(pg)), '\0', P_FREESPACE(pg));
      break;
    }
  }

  c_stream.zalloc=(alloc_func)0;
  c_stream.zfree=(free_func)0;
  c_stream.opaque=(voidpf)0;

  if(deflateInit(&c_stream, Z_DEFAULT_COMPRESSION) != Z_OK) {
    ret = EIO;
    goto err;
  }

  c_stream.next_in = (Bytef*)inbuff;
  c_stream.avail_in = inbuff_length;
  c_stream.next_out = outbuff;
  c_stream.avail_out = outbuff_length;

  while((r = deflate(&c_stream, Z_FINISH)) != Z_STREAM_END && r == Z_OK)
    ;

  if(r != Z_STREAM_END)
    ret = EIO;
  
  if(deflateEnd(&c_stream) != Z_OK)
    ret = EIO;

  if(ret == 0) {
    *outbuffp = outbuff;
    *outbuff_lengthp = outbuff_length - c_stream.avail_out;
  } else {
    CDB___os_free(outbuff, outbuff_length);
  }
#ifdef DEBUG_CMPR
  fprintf(stderr,"CDB___memp_cmpr_deflate:: compress %d bytes to %d \n", inbuff_length, *outbuff_lengthp);
#endif
  
 err:
  return ret;
#else /* HAVE_LIBZ */
  return EINVAL;
#endif /* HAVE_LIBZ */
}



/*
 * __memp_cmpr_info_valid --
 *	Compute compressed page size
 */
static int
__memp_cmpr_info_valid(dbenv,cmpr_info)
    DB_ENV *dbenv;
    DB_CMPR_INFO *cmpr_info;
{
    int ret = 0;
    if(!cmpr_info              ) {
	CDB___db_err(dbenv, "__memp_cmpr_info_valid: cmpr_info == NULL");
	ret = CDB___db_panic(dbenv, EINVAL);
	goto err;
    }

    if(!cmpr_info->compress   ) {
	CDB___db_err(dbenv, "__memp_cmpr_info_valid: compress == NULL!");
	ret = CDB___db_panic(dbenv, EINVAL);
	goto err;
    }

    if(!cmpr_info->uncompress   ) {
	CDB___db_err(dbenv, "__memp_cmpr_info_valid: uncompress == NULL!");
	ret = CDB___db_panic(dbenv, EINVAL);
	goto err;
    }

    if(cmpr_info->coefficient == 0 ||  cmpr_info->coefficient > 5  ) {
	CDB___db_err(dbenv, "__memp_cmpr_info_valid:  coefficient should be > 0 and < 5 coefficient=%d ", cmpr_info->coefficient);
	ret = CDB___db_panic(dbenv, EINVAL);
	goto err;
    }

    if(cmpr_info->max_npages == 0 ||  cmpr_info->max_npages > 128  ) {
	CDB___db_err(dbenv, "__memp_cmpr_info_valid:  max_npages should be > 0 and < 128 max_npages=%d ", cmpr_info->max_npages);
	ret = CDB___db_panic(dbenv, EINVAL);
	goto err;
    }
err:
    return ret;
}

/*
 * __memp_cmpr_pagesize --
 *	Compute compressed page size
 *
 * PUBLIC: u_int8_t CDB___memp_cmpr_coefficient __P((DB_ENV *dbenv));
 */
u_int8_t
CDB___memp_cmpr_coefficient(dbenv)
    DB_ENV *dbenv;
{
    u_int8_t ret = 0;

    if(!dbenv || !dbenv->mp_cmpr_info) {
	ret = default_cmpr_info.coefficient;
    } else {
	__memp_cmpr_info_valid(dbenv, dbenv->mp_cmpr_info);
	ret = dbenv->mp_cmpr_info->coefficient;
    }

    return (ret);
}
/*
 * Initialisation of page compression
 */

#define CMPR_META_NORMAL	0x01
#define CMPR_META_COMPRESSED	0x02

typedef struct _cmprmeta {
  u_int32_t	magic;		/* 00-03: Magic number. */
  db_pgno_t	free;		/* 04-07: First free page. */
} CMPRMETA;

int
CDB___memp_cmpr_create(dbenv, fhp, pgsize, flags)
     DB_ENV *dbenv;
     DB_FH *fhp;
     size_t pgsize;
     int flags;
{
  int ret;
  int count = 0;
  CMPRMETA meta;
  char* buffer;

  if((ret = CDB___os_malloc(dbenv, pgsize, NULL, &buffer)) != 0) {
      CDB___db_err(dbenv, "CDB___memp_cmpr_create: os_malloc %d bytes failed:%d", pgsize, ret);
      ret = CDB___db_panic(dbenv, EINVAL);
      return ret;
  }

  meta.magic = flags == MP_CMPR ? CMPR_META_COMPRESSED : CMPR_META_NORMAL;
  meta.free = PGNO_INVALID;

  if((ret = CDB___os_seek(dbenv, fhp, 0, 0, 0, 0, DB_OS_SEEK_SET)) != 0) {
    CDB___db_err(dbenv, "CDB___memp_cmpr_create: seek to 0 error");
    return CDB___db_panic(dbenv, ret);
  }
  memcpy(buffer, (char*)&meta, sizeof(CMPRMETA));
  if((ret = CDB___os_write(dbenv, fhp, buffer, pgsize, &count)) < 0) {
    CDB___db_err(dbenv, "CDB___memp_cmpr_create: write error at 0");
    return CDB___db_panic(dbenv, ret);
  }
  if(count != pgsize) {
    CDB___db_err(dbenv, "CDB___memp_cmpr_create: write error %d bytes instead of %d bytes", count, pgsize);
    return CDB___db_panic(dbenv, EINVAL);
  }
  CDB___os_free(buffer, pgsize);

  return ret;
}

/*
 * CDB___memp_cmpr_open --
 *	Cache the meta information about compression, initialize dbenv info.
 *
 * PUBLIC: int CDB___memp_cmpr_open __P((DB_ENV *, MPOOLFILE *, const char *));
 */
int
CDB___memp_cmpr_open(dbenv, mfp, path)
     DB_ENV *dbenv;
     MPOOLFILE *mfp;
     const char *path;
{
  int ret;
  /*
   * Read compression meta information
   */
  DB_FH fh;
  ssize_t count;
  CMPRMETA meta;

  if((ret = CDB___os_open(dbenv, path, DB_OSO_RDONLY, 0, &fh)) != 0) {
    CDB___db_err(dbenv, "CDB___memp_cmpr_open: cannot open %s readonly", path);
    return CDB___db_panic(dbenv, ret);
  }

  if((ret = CDB___os_read(dbenv, &fh, (void*)&meta, sizeof(CMPRMETA), &count)) != 0) {
    CDB___db_err(dbenv, "CDB___memp_cmpr_open: cannot read page 0");
    ret = CDB___db_panic(dbenv, ret);
    goto err;
  }

  if(count != sizeof(CMPRMETA)) {
    CDB___db_err(dbenv, "CDB___memp_cmpr_open: read error %d bytes instead of %d bytes", count, sizeof(CMPRMETA));
    ret = CDB___db_panic(dbenv, EINVAL);
    goto err;
  }

  if(meta.magic == CMPR_META_COMPRESSED) {
    mfp->flags |= MP_CMPR;
    mfp->cmpr_free = meta.free;

    /*
     * Initialisation of cmpr_context
     */
    if(!dbenv->mp_cmpr_info) {
      if(default_cmpr_info.compress == 0) {
	CDB___db_err(dbenv, "CDB___memp_cmpr_open: zlib compression not available, re-compile --with-zlib=DIR");
	ret = CDB___db_panic(dbenv, EINVAL);
	goto err;
      }
      dbenv->mp_cmpr_info = &default_cmpr_info;
    }
    /*
     * Check if cmpr_info is sane
     */
    if((ret = __memp_cmpr_info_valid(dbenv, dbenv->mp_cmpr_info)))
      goto err;
  }

 err:
  CDB___os_closehandle(&fh);

  return ret;
}

/*
 * CDB___memp_cmpr_close --
 *	This is not really a close but a sync. It is called more than
 *      once per file, specifically when opening subdatabases. The
 *      file handle will be used afterwards, most of the time.
 *
 * PUBLIC: int CDB___memp_cmpr_close __P((DB_ENV *, DB_MPOOLFILE *));
 */
int
CDB___memp_cmpr_close(dbenv, dbmfp)
     DB_ENV *dbenv;
     DB_MPOOLFILE *dbmfp;
{
  /*
   * If handle is READ/WRITE
   */
  if(dbmfp->flags & MP_UPGRADE) {
    MPOOLFILE *mfp = dbmfp->mfp;
    DB_FH *fhp = &dbmfp->fh;
    size_t count = 0;
    int ret;

    CMPRMETA meta;
    memset((char*)&meta, '\0', sizeof(CMPRMETA));

    meta.magic = mfp->flags & MP_CMPR ? CMPR_META_COMPRESSED : CMPR_META_NORMAL;
    if(mfp->flags & MP_CMPR)
      meta.free = mfp->cmpr_free;

    if((ret = CDB___os_seek(dbenv, fhp, 0, 0, 0, 0, DB_OS_SEEK_SET)) != 0) {
      CDB___db_err(dbenv, "CDB___memp_cmpr_close: seek to 0 error");
      return CDB___db_panic(dbenv, ret);
    }
  
    if((ret = CDB___os_write(dbenv, fhp, (void*)&meta, sizeof(CMPRMETA), &count)) < 0) {
      CDB___db_err(dbenv, "CDB___memp_cmpr_close: write error at 0");
      return CDB___db_panic(dbenv, ret);
    }

    if(count != sizeof(CMPRMETA)) {
      CDB___db_err(dbenv, "CDB___memp_cmpr_close: write error %d bytes instead of %d bytes", count, sizeof(CMPRMETA));
      return CDB___db_panic(dbenv, EINVAL);
    }
  }
  
  return 0;
}

/*
 * CDB___memp_cmpr_alloc --
 *	Get a new free page to store weak compression data.
 *
 * PUBLIC: int CDB___memp_cmpr_alloc __P((DB_MPOOLFILE *, db_pgno_t *, size_t, BH *, int *));
 */
int
CDB___memp_cmpr_alloc(dbmfp, pgnop, pagesize, bhp, first_nonreused_chain_posp)
     DB_MPOOLFILE *dbmfp;
     db_pgno_t *pgnop;
     size_t pagesize;
     BH *bhp;
     int *first_nonreused_chain_posp;
{
  DB_ENV *dbenv = dbmfp->dbmp->dbenv;
  int ret = 0;

#ifdef DEBUG_CMPR
  fprintf(stderr,"CDB___memp_cmpr_alloc:: bhp:%8x bhp->chain:%8x  first_nonreused_chain_posp:%2d\n", bhp, bhp->chain, *first_nonreused_chain_posp);
#endif
  if(F_ISSET(bhp, BH_CMPR) && bhp->chain == NULL) {
      CDB___db_err(dbenv, "CDB___memp_cmpr_alloc: BH_CMPR set and bhp->chain == NULL");
      ret = CDB___db_panic(dbenv, EINVAL);
      goto err;
  }

  if((*first_nonreused_chain_posp) >= (CMPR_MAX - 1)) {
      CDB___db_err(dbenv, "CDB___memp_cmpr_alloc: first_nonreused_chain_pos >= (CMPR_MAX - 1)");
      ret = CDB___db_panic(dbenv, EINVAL);
      goto err;
  }

  /*
   * If possible reuse an existing chain.
   */
  if((*first_nonreused_chain_posp) >= 0 && F_ISSET(bhp, BH_CMPR) && bhp->chain[*first_nonreused_chain_posp]) {
    *pgnop = bhp->chain[*first_nonreused_chain_posp];
    (*first_nonreused_chain_posp)++;
#ifdef DEBUG_CMPR
    fprintf(stderr,"CDB___memp_cmpr_alloc:: reusing page in chain \n");
#endif
  } else {
    MPOOLFILE *mfp = dbmfp->mfp;
    DB_MPOOL *dbmp = dbmfp->dbmp;

    /* all pages in bhp->chain are now reused */
    (*first_nonreused_chain_posp) = -1;
#ifdef DEBUG_CMPR
    fprintf(stderr,"CDB___memp_cmpr_alloc:: no more reusable pages in chain\n");
#endif
    R_LOCK(dbenv, dbmp->reginfo);
    if(mfp->cmpr_free == PGNO_INVALID) {
#ifdef DEBUG_CMPR
      fprintf(stderr,"CDB___memp_cmpr_alloc:: free page pool empty, allocating\n");
#endif
      ret = 0;
      ++dbmfp->mfp->last_pgno;
#ifdef DEBUG
      word_monitor_set(DB_MONITOR(dbenv), WORD_MONITOR_PGNO, dbmfp->mfp->last_pgno);
#endif /* DEBUG */
      *pgnop = dbmfp->mfp->last_pgno;
    } else {
      /*
       * Read the free page, save the next free page number.
       */
      CMPR cmpr;
      size_t count;
      DB_FH *fhp = &dbmfp->fh;

      *pgnop = mfp->cmpr_free;

      if((ret = CDB___os_seek(dbenv, fhp, pagesize, *pgnop, 0, 0, DB_OS_SEEK_SET)) != 0) {
	CDB___db_err(dbenv, "CDB___memp_cmpr_alloc: seek error at %d", *pgnop);
	ret = CDB___db_panic(dbenv, ret);
	goto oops;
      }
      if((ret = CDB___os_read(dbenv, fhp, (void*)&cmpr, sizeof(CMPR), &count)) != 0) {
	CDB___db_err(dbenv, "CDB___memp_cmpr_alloc: read error at %d", *pgnop);
	ret = CDB___db_panic(dbenv, ret);
	goto oops;
      }
      if(count != sizeof(CMPR)) {
	CDB___db_err(dbenv, "CDB___memp_cmpr_alloc: read error %d bytes instead of %d bytes", count, sizeof(CMPR));
	ret = CDB___db_panic(dbenv, ret);
	goto oops;
      }
      if(cmpr.flags != DB_CMPR_FREE) {
	CDB___db_err(dbenv, "CDB___memp_cmpr_alloc: got %d flags instead of DB_CMPR_FREE", cmpr.flags);
	ret = CDB___db_panic(dbenv, ret);
	goto oops;
      }
      mfp->cmpr_free = cmpr.next;

      if(*pgnop == 0) {
	CDB___db_err(dbenv, "CDB___memp_cmpr_alloc: unexpected pgno == 0");
	ret = CDB___db_panic(dbenv, ret);
	goto oops;
      }

#ifdef DEBUG_CMPR
      fprintf(stderr,"CDB___memp_cmpr_alloc:: reuse free page %d\n", *pgnop);
#endif
    }
  oops:
    R_UNLOCK(dbenv, dbmp->reginfo);
  }
 err:
  return ret;
}

/*
 * CDB___memp_cmpr_free --
 *	Release a page used to store weak compression data.
 *
 * PUBLIC: int CDB___memp_cmpr_free __P((DB_MPOOLFILE *, db_pgno_t));
 */
int
CDB___memp_cmpr_free(dbmfp, pgno, pagesize)
     DB_MPOOLFILE *dbmfp;
     db_pgno_t pgno;
     size_t pagesize;
{
  int ret = 0;

  DB_ENV *dbenv = dbmfp->dbmp->dbenv;
  MPOOLFILE *mfp = dbmfp->mfp;
  DB_MPOOL *dbmp = dbmfp->dbmp;
  DB_FH *fhp = &dbmfp->fh;
  CMPR cmpr;
  size_t count;

  R_LOCK(dbenv, dbmp->reginfo);

  cmpr.flags = DB_CMPR_FREE;
  cmpr.next = mfp->cmpr_free;
  mfp->cmpr_free = pgno;

#ifdef DEBUG_CMPR
  fprintf(stderr,"CDB___memp_cmpr_free::  freeing page:%3d \n",pgno);
#endif

  if((ret = CDB___os_seek(dbenv, fhp, pagesize, pgno, 0, 0, DB_OS_SEEK_SET)) != 0) {
    CDB___db_err(dbenv, "CDB___memp_cmpr_free: seek error at %d", pgno);
    ret = CDB___db_panic(dbenv, ret);
    goto err;
  }
  if((ret = CDB___os_write(dbenv, fhp, (void*)&cmpr, sizeof(CMPR), &count)) < 0) {
    CDB___db_err(dbenv, "CDB___memp_cmpr_free: write error at %d", pgno);
    ret = CDB___db_panic(dbenv, ret);
    goto err;
  }
  if(count != sizeof(CMPR)) {
    CDB___db_err(dbenv, "CDB___memp_cmpr_free: write error %d bytes instead of %d bytes", count, sizeof(CMPR));
    ret = CDB___db_panic(dbenv, ret);
    goto err;
  }

 err:
  R_UNLOCK(dbenv, dbmp->reginfo);
  return ret;
}


/*
 * CDB___memp_cmpr_alloc_chain --
 *	Allocate chain entry in BH
 *
 * PUBLIC: int CDB___memp_cmpr_alloc_chain __P((DB_MPOOL *, BH *));
 */

int
CDB___memp_cmpr_alloc_chain(dbmp, bhp, alloc_type)
	DB_MPOOL *dbmp;
	BH *bhp;
	int alloc_type;
{
  DB_ENV *dbenv = dbmp->dbenv;
  int ret = 0;
  if(!bhp->chain) {
    int alloc_ret;
    int alloc_length = sizeof(db_pgno_t)*(CMPR_MAX-1);
    switch(alloc_type) {
    case BH_CMPR_POOL:
      {
	MPOOL *mp = dbmp->reginfo[0].primary;
	int n_cache = NCACHE(mp, bhp->pgno);
	alloc_ret = CDB___memp_alloc(dbmp, &dbmp->reginfo[n_cache], NULL, alloc_length, NULL, (void *)(&bhp->chain));
	F_SET(bhp, BH_CMPR_POOL);
      }
      break;
    case BH_CMPR_OS:
      alloc_ret = CDB___os_malloc(dbenv, alloc_length, NULL, &bhp->chain);
      F_SET(bhp, BH_CMPR_OS);
      break;
    default:
      CDB___db_err(dbenv, "CDB___memp_cmpr_alloc_chain: unknown alloc type :%d", alloc_type);
      ret = CDB___db_panic(dbenv, EINVAL);
      goto err;
      break;
    }
    
    if(alloc_ret) {
      CDB___db_err(dbenv, "CDB___memp_cmpr_alloc_chain: memp_alloc %d bytes failed:%d", alloc_length, alloc_ret);
      ret = CDB___db_panic(dbenv, EINVAL);
      goto err;
    }
    memset((void *)bhp->chain, 0, alloc_length);
#if defined(DEBUG_CMPR) || defined(DEBUG_CMPR_ALLOC)
    fprintf(stderr, "CDB___memp_cmpr_alloc_chain:: allocate chain in %s :%8x\n", (alloc_type == BH_CMPR_OS ? "malloc" : "shalloc"), bhp->chain);
#endif
  } else {
#ifdef DEBUG_CMPR
    fprintf(stderr, "CDB___memp_cmpr_alloc_chain:: existing chain:%8x\n", bhp->chain);
#endif
  }
  F_SET(bhp, BH_CMPR);
 err:
  return ret;
}

/*
 * CDB___memp_cmpr_free_chain --
 *	Free chain entry in BH
 *
 * PUBLIC: int CDB___memp_cmpr_free_chain __P((DB_MPOOL *, BH *));
 */

int
CDB___memp_cmpr_free_chain(dbmp, bhp)
	DB_MPOOL *dbmp;
	BH *bhp;
{
  DB_ENV *dbenv = dbmp->dbenv;

  if(F_ISSET(bhp, BH_CMPR)) {
    if(bhp->chain) {
      int alloc_length = sizeof(db_pgno_t)*(CMPR_MAX-1);
      int alloc_type = bhp->flags & (BH_CMPR_POOL|BH_CMPR_OS);
      switch(alloc_type) {
      case BH_CMPR_POOL:
	{
	  MPOOL *mp = dbmp->reginfo[0].primary;
	  int n_cache = NCACHE(mp, bhp->pgno);
	  CDB___db_shalloc_free(dbmp->reginfo[n_cache].addr, bhp->chain);
	}
	break;
      case BH_CMPR_OS:
	CDB___os_free(bhp->chain, alloc_length);
	break;
      default:
	CDB___db_err(dbenv, "CDB___memp_cmpr_free_chain: unknown alloc type :%d", alloc_type);
	return CDB___db_panic(dbenv, EINVAL);
	break;
      }
#if defined(DEBUG_CMPR) || defined(DEBUG_CMPR_ALLOC)
      fprintf(stderr, "CDB___memp_cmpr_free_chain:: free chain in %s :%8x\n", (alloc_type == BH_CMPR_OS ? "malloc" : "shalloc"), bhp->chain);
#endif
      bhp->chain = NULL;
    } else {
	CDB___db_err(dbenv, "CDB___memp_cmpr_free_chain: BH_CMPR set but null bhp->chain");
	return CDB___db_panic(dbenv, EINVAL);
    }
  } else if(bhp->chain) {
    CDB___db_err(dbenv, "CDB___memp_cmpr_free_chain: BH_CMPR not set but bhp->chain not null");
    return CDB___db_panic(dbenv, EINVAL);
  }

  F_CLR(bhp, BH_CMPR | BH_CMPR_OS | BH_CMPR_POOL);

  return 0;
}
