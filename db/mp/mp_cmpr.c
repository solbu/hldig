/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1999
 *	Loic Dachary.  All rights reserved.
 * 
 * TODO:
 *   Keith Bostic says:
 *   The only change I'd probably think about is if
 *   we should merge the call to __memp_pg and __memp_cmpr -- kind
 *   of a stack of page modification routines, that sits on top of
 *   __os_io.  That's a bigger change, but it's probably cleaner
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
 *  In __memp_cmpr, niop is always multiplied by compression factor for page 0. 
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

#ifdef HAVE_LIBZ

#ifndef lint
static const char sccsid[] = "@(#)mp_cmpr.c	1.0 (Senga) 01/08/99";
#endif /* not lint */

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <errno.h>
#endif

#include "zlib.h"
#include "db_int.h"
#include "shqueue.h"
#include "db_shash.h"
#include "mp.h"
#include "db_page.h"
#include "common_ext.h"

/*
 * Helpers declarations.
 */
static int __memp_cmpr_page __P((DB_MPOOLFILE *, CMPR *, DB_IO *, ssize_t *));

/*
 * Maximum chain length
 */
#define CMPR_MAX	(dbmfp->cmpr_context.info->max_npages)

#define CMPR_MULTIPLY(n) ((n) << (dbmfp->cmpr_context.info->coefficient))
#define CMPR_DIVIDE(n)   ((n) >> (dbmfp->cmpr_context.info->coefficient))

static DB_CMPR_INFO default_cmpr_info = {
    __memp_cmpr_deflate,    
    __memp_cmpr_inflate,    
    1,
    3,
    NULL
};

/*
 * Entry point. Functionaly equivalent to __os_io.
 * Compress/uncompress pages before returning them or writing them to disk.
 */

/*
 * __memp_cmpr --
 *	Transparent compression read/write
 *
 * PUBLIC: int __memp_cmpr __P((DB_MPOOLFILE *, BH *, DB_IO *, int, ssize_t *));
 */
int
__memp_cmpr(dbmfp, bhp, db_io, flag, niop)
	DB_MPOOLFILE *dbmfp;
	BH *bhp;
	DB_IO *db_io;
	int flag;
	ssize_t *niop;
{
  size_t orig_pagesize = db_io->pagesize;
  db_pgno_t orig_pgno = db_io->pgno;
  size_t orig_bytes = db_io->bytes;
  int ret;

  db_io->pagesize = CMPR_DIVIDE(db_io->pagesize);
  db_io->bytes = CMPR_DIVIDE(db_io->bytes);

  /*
   * Page 0 is a special case. It contains the metadata information (at most 512 bytes)
   * and must not be compressed because it is read with __os_read and not __os_io.
   */
  switch (flag) {
  case DB_IO_READ:
    if(db_io->pgno == 0) {
      ret = __os_io(db_io, DB_IO_READ, niop);
      *niop = CMPR_MULTIPLY(*niop);
    } else 
      ret = __memp_cmpr_read(dbmfp, bhp, db_io, niop);
    break;
  case DB_IO_WRITE:
    if(db_io->pgno == 0) {
      ret = __os_io(db_io, DB_IO_WRITE, niop);
      *niop = CMPR_MULTIPLY(*niop);
    } else
      ret = __memp_cmpr_write(dbmfp, bhp, db_io, niop);
    break;
  }

  db_io->pgno = orig_pgno;
  db_io->pagesize = orig_pagesize;
  db_io->bytes = orig_bytes;

  return ret;
}

/*
 * __memp_cmpr_read --
 *	Transparent compression read
 *
 * PUBLIC: int __memp_cmpr_read __P((DB_MPOOLFILE *, BH *, DB_IO *, ssize_t *));
 */
int
__memp_cmpr_read(dbmfp, bhp, db_io, niop)
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
  int chain_count = 0;
  db_pgno_t first_pgno = db_io->pgno;
  DB_CMPR_INFO *cmpr_info = dbmfp->cmpr_context.info;
  /*
   * By default the compression does not use too much space,
   * hence the chain is empty.
   */
  F_CLR(bhp, BH_CMPR);

  /*MB read first page (if no overflow, this is the only one) */
  ret = __os_io(db_io, DB_IO_READ, niop);

  /*
   * An error or partial read on the first page means that we're not
   * going anywhere.
   */
  if(ret || *niop < db_io->pagesize)
    goto err;

  /*MB read the cmpr header from read page */
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
    __db_err(dbmfp->dbmp->dbenv, "__memp_cmpr_read: expected DB_CMPR_FIRST flag set at pgno = %ld", db_io->pgno);
    ret = __db_panic(dbmfp->dbmp->dbenv, EINVAL);
    goto err;
  }
  
  if((ret = __os_malloc(db_io->pagesize * CMPR_MAX, NULL, &buffcmpr)) != 0)
    goto err;

  do {
    /*MB read the first part of the compressed data from read page */
    memcpy(buffcmpr + buffcmpr_length, DB_CMPR_DATA(db_io), DB_CMPR_PAGESIZE(db_io));
    buffcmpr_length += DB_CMPR_PAGESIZE(db_io);

    if(chain_count++ >= CMPR_MAX) {
      __db_err(dbmfp->dbmp->dbenv, "__memp_cmpr_read: compression chain too long at pgno = %ld", db_io->pgno);
      ret = __db_panic(dbmfp->dbmp->dbenv, EINVAL);
      goto err;
    }

    /*
     * Flag must only contain FIRST|INTERNAL and/or CHAIN. If other bits are
     * set, the data is corrupted. Removing the FIRST bit and checking
     * the CHAIN bit with == instead of F_ISSET verify this.
     */
    F_CLR(&cmpr, DB_CMPR_FIRST | DB_CMPR_INTERNAL);
    chain = cmpr.flags;

    if(chain == DB_CMPR_CHAIN) {
      /*MB overflow case! continue reading data from xtra pages*/

      if(cmpr.next == 0) {
	__db_err(dbmfp->dbmp->dbenv, "__memp_cmpr_read: cmpr.next is null at pgno = %ld", chain, db_io->pgno);
	ret = __db_panic(dbmfp->dbmp->dbenv, EINVAL);
	goto err;
      }
      /*
       * Keep the chain in buffer header.
       */
      __memp_cmpr_alloc_chain(dbmfp, bhp);

      bhp->chain[chain_count - 1] = cmpr.next;
      db_io->pgno = cmpr.next;
      /*MB read data from xtra page  */
      if((ret = __os_io(db_io, DB_IO_READ, niop)) != 0 ||
	 *niop != db_io->pagesize) {
	ret = EIO;
	goto err;
      }
      /*MB read the cmpr header from this xtra page */
      memcpy(&cmpr, db_io->buf, sizeof(CMPR));
    } else if(chain != 0) {
      __db_err(dbmfp->dbmp->dbenv, "__memp_cmpr_read: unexpected compression flag value 0x%x at pgno = %ld", chain, db_io->pgno);
      ret = __db_panic(dbmfp->dbmp->dbenv, ret);
      goto err;
    } else if(cmpr.next != 0) {
      __db_err(dbmfp->dbmp->dbenv, "__memp_cmpr_read: cmpr.next is not null at pgno = %ld", chain, db_io->pgno);
      ret = __db_panic(dbmfp->dbmp->dbenv, ret);
      goto err;
    }
  } while(chain);
  
  /*
   * We gathered all the compressed data in buffcmpr, inflate it.
   */
  if((ret = (*cmpr_info->uncompress)(buffcmpr, buffcmpr_length, db_io->buf, CMPR_MULTIPLY(db_io->pagesize), cmpr_info->user_data)) != 0) {
    __db_err(dbmfp->dbmp->dbenv, "__memp_cmpr_read: unable to uncompress page at pgno = %ld", first_pgno);
    ret = __db_panic(dbmfp->dbmp->dbenv, ret);
    goto err;
  }

  *niop = CMPR_MULTIPLY(db_io->pagesize);

 err:
  if(buffcmpr) __os_free(buffcmpr, 0);
  return ret;
}

/*
 * __memp_cmpr_write --
 *	Transparent compression write
 *
 * PUBLIC: int __memp_cmpr_write __P((DB_MPOOLFILE *, BH *, DB_IO *, ssize_t *));
 */
int
__memp_cmpr_write(dbmfp, bhp, db_io, niop)
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
  db_pgno_t first_pgno = db_io->pgno;
  DB_CMPR_INFO *cmpr_info = dbmfp->cmpr_context.info;

  if((ret = __os_malloc(CMPR_MULTIPLY(db_io->bytes), NULL, &db_io->buf)) != 0)
    goto err;

  if((ret = (*cmpr_info->compress)(orig_buff, CMPR_MULTIPLY(db_io->pagesize), &buffcmpr, &buffcmpr_length, cmpr_info->user_data)) != 0) {
    __db_err(dbmfp->dbmp->dbenv, "__memp_cmpr_write: unable to compress page at pgno = %ld", db_io->pgno);
    ret = __db_panic(dbmfp->dbmp->dbenv, ret);
    goto err;
  }

  /*
   * This can never happen.
   */
  if(buffcmpr_length > DB_CMPR_PAGESIZE(db_io) * CMPR_MAX) {
    __db_err(dbmfp->dbmp->dbenv, "__memp_cmpr_write: compressed data is too big at pgno = %ld", db_io->pgno);
    ret = __db_panic(dbmfp->dbmp->dbenv, EINVAL);
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
      if(chain_length >= CMPR_MAX) {
	  __db_err(dbmfp->dbmp->dbenv, "__memp_cmpr_write: chain_length overflow");
	  ret = __db_panic(dbmfp->dbmp->dbenv, EINVAL);
	  goto err;
      }
      F_SET(&cmpr, DB_CMPR_CHAIN);
      if((ret = __memp_cmpr_alloc(dbmfp, &cmpr.next, bhp, &first_nonreused_chain_pos)) != 0)
	goto err;
      __memp_cmpr_alloc_chain(dbmfp, bhp);
      bhp->chain[chain_length++] = cmpr.next;
    }
    /* write in the cmpr header */
    memcpy(db_io->buf, &cmpr, sizeof(CMPR));
    /* write in what's left of the compressed buffer (and that also fits in) */
    memcpy(db_io->buf + sizeof(CMPR), buffp, copy_length);
    buffp += copy_length;
    /* actual output  */
    if((ret = __os_io(db_io, DB_IO_WRITE, niop)) != 0 ||
       *niop != db_io->pagesize) {
      ret = EIO;
      goto err;
    }
    db_io->pgno = cmpr.next;
    cmpr.flags = DB_CMPR_INTERNAL;
    cmpr.next = 0;
  } while(buffp - buffcmpr < buffcmpr_length);

#ifdef DEBUG
  fprintf(stderr,"__memp_cmpr_write:: chain_length ... (num overflow pages):%2d\n",chain_length);
#endif
  /*
   * If the chain was not completely reused, free the remaining pages (the page compression
   * rate is better).
   */
  if(F_ISSET(bhp, BH_CMPR) && first_nonreused_chain_pos >= 0) {
    int i;
    CMPR cmpr;
    cmpr.flags = DB_CMPR_FREE;
    cmpr.next = 0;
    memcpy(db_io->buf, &cmpr, sizeof(CMPR));
    for(i = first_nonreused_chain_pos; i < CMPR_MAX && bhp->chain[i]; i++) {
      if((ret = __memp_cmpr_free(dbmfp, bhp->chain[i])) != 0)
	goto err;
      /*
       * Mark the page as free for recovery.
       */
      db_io->pgno = bhp->chain[i];
      if((ret = __os_io(db_io, DB_IO_WRITE, niop)) != 0 ||
	 *niop != db_io->pagesize) {
	ret = EIO;
	goto err;
      }
      bhp->chain[i] = 0;
    }
  }
  
  if(chain_length == 0) {
      __memp_cmpr_free_chain(dbmfp, bhp);
  }
      

  /*
   * In case of success, always pretend that we exactly wrote the
   * all bytes of the original pagesize.
   */
  *niop = CMPR_MULTIPLY(db_io->pagesize);

 err:
  __os_free(db_io->buf, 0);
  db_io->buf = orig_buff;
  if(buffcmpr) __os_free(buffcmpr, 0);

  return ret;
}

/*
 * Helpers
 */

/*
 * __memp_cmpr_page --
 *	Build a fake page. This function is a __memp_cmpr_read helper.
 *
 */
static int
__memp_cmpr_page(dbmfp, cmpr, db_io, niop)
     DB_MPOOLFILE *dbmfp;    
     CMPR *cmpr;
     DB_IO *db_io;
     ssize_t *niop;
{
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

/*
 * __memp_cmpr_inflate --
 *	Decompress buffer
 *
 * PUBLIC: int __memp_cmpr_inflate __P((const u_int8_t *, int, u_int8_t *, int, void *));
 */
int
__memp_cmpr_inflate(inbuff, inbuff_length, outbuff, outbuff_length, user_data)
     const u_int8_t* inbuff;
     int inbuff_length;
     u_int8_t* outbuff;
     int outbuff_length;
     void *user_data;
{
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
}



/*
 * __memp_cmpr_deflate --
 *	Compress buffer
 *
 * PUBLIC: int __memp_cmpr_deflate __P((const u_int8_t *, int, u_int8_t **, int*, void *));
 */
int
__memp_cmpr_deflate(inbuff, inbuff_length, outbuffp, outbuff_lengthp, user_data)
     const u_int8_t* inbuff;
     int inbuff_length;
     u_int8_t** outbuffp;
     int* outbuff_lengthp;
     void *user_data;
{
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

  if(__os_malloc(outbuff_length, NULL, &outbuff) != 0) {
    ret = ENOMEM;
    goto err;
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
    __os_free(outbuff, outbuff_length);
  }

 err:
  return ret;
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
	__db_err(dbenv, "__memp_cmpr_info_valid: cmpr_info == NULL");
	ret = __db_panic(dbenv, EINVAL);
	goto err;
    }

    if(!cmpr_info->compress   ) {
	__db_err(dbenv, "__memp_cmpr_info_valid: compress == NULL!");
	ret = __db_panic(dbenv, EINVAL);
	goto err;
    }

    if(!cmpr_info->uncompress   ) {
	__db_err(dbenv, "__memp_cmpr_info_valid: uncompress == NULL!");
	ret = __db_panic(dbenv, EINVAL);
	goto err;
    }

    if(cmpr_info->coefficient == 0 ||  cmpr_info->coefficient > 5  ) {
	__db_err(dbenv, "__memp_cmpr_info_valid:  coefficient should be > 0 and < 5 coefficient=%d ", cmpr_info->coefficient);
	ret = __db_panic(dbenv, EINVAL);
	goto err;
    }

    if(cmpr_info->max_npages == 0 ||  cmpr_info->max_npages > 128  ) {
	__db_err(dbenv, "__memp_cmpr_info_valid:  max_npages should be > 0 and < 128 max_npages=%d ", cmpr_info->max_npages);
	ret = __db_panic(dbenv, EINVAL);
	goto err;
    }
err:
    return ret;
}

/*
 * __memp_cmpr_pagesize --
 *	Compute compressed page size
 *
 * PUBLIC: u_int8_t __memp_cmpr_coefficient __P((DB_ENV *dbenv));
 */
u_int8_t
__memp_cmpr_coefficient(dbenv)
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

/*
 * __memp_cmpr_open --
 *	Open the db that contains the free compression pages.
 *
 * PUBLIC: int __memp_cmpr_open __P((const char *, DB_ENV *, CMPR_CONTEXT *));
 */
int
__memp_cmpr_open(path, dbenv, cmpr_context)
     const char *path;
     DB_ENV *dbenv;
     CMPR_CONTEXT *cmpr_context;
{
  int ret;
  char* tmp = 0;
  int tmp_length = strlen(path) + strlen(DB_CMPR_SUFFIX) + 1;

  /*
   * Management of pages containing data when the compression does not achieve
   * the expected compression ratio.
   */
  {
      DB_INFO dbinfo;
      
      memset((char*)&dbinfo, '\0', sizeof(DB_INFO));
      
      if((ret = __os_malloc(tmp_length, NULL, &tmp)) != 0)
	  goto err;
		
      dbinfo.re_len = 1;
      dbinfo.flags = DB_RECNUM;
      
      sprintf(tmp, "%s%s", path, DB_CMPR_SUFFIX);
      
      if((ret = db_open(tmp, DB_BTREE, DB_CREATE, 0666, dbenv, &dbinfo, &(cmpr_context->weakcmpr))) != 0)
	  goto err;
  }


  /*
   * Initialisation of cmpr_context
   */
  {
      if(!dbenv || !dbenv->mp_cmpr_info) {
	  cmpr_context->info = &default_cmpr_info;
      } else {
	  cmpr_context->info = dbenv->mp_cmpr_info;
      }
  } 
  /*
   * Check if cmpr_info is sane
   */
  if((ret = __memp_cmpr_info_valid(dbenv, cmpr_context->info)))
      goto err;

 err:
  if(tmp) __os_free(tmp, tmp_length);
  return ret;
}

/*
 * __memp_cmpr_close --
 *	Close the db that contains the free compression pages.
 *
 * PUBLIC: int __memp_cmpr_close __P((CMPR_CONTEXT *));
 */
int
__memp_cmpr_close(cmpr_context)
     CMPR_CONTEXT *cmpr_context;
{
  int ret = 0;

  if(cmpr_context->weakcmpr == 0) {
    ret = EINVAL;
    goto err;
  }

  if((ret = cmpr_context->weakcmpr->close(cmpr_context->weakcmpr, 0)) != 0)
    goto err;

  cmpr_context->weakcmpr = 0;

 err:
  return ret;
}

/*
 * __memp_cmpr_alloc --
 *	Get a new free page to store weak compression data.
 *
 * PUBLIC: int __memp_cmpr_alloc __P((DB_MPOOLFILE *, db_pgno_t *, BH *, int *));
 */
int
__memp_cmpr_alloc(dbmfp, pgnop, bhp, first_nonreused_chain_posp)
     DB_MPOOLFILE *dbmfp;
     db_pgno_t *pgnop;
     BH *bhp;
     int *first_nonreused_chain_posp;
{
  int ret = 0;

#ifdef DEBUG
  fprintf(stderr,"__memp_cmpr_alloc:: bhp:%8x bhp->chain:%8x  first_nonreused_chain_posp:%2d\n",bhp,bhp->chain,*first_nonreused_chain_posp);
#endif
  if(F_ISSET(bhp, BH_CMPR) && bhp->chain == NULL) {
      __db_err(dbmfp->dbmp->dbenv, "__memp_cmpr_alloc: BH_CMPR set and bhp->chain == NULL");
      ret = __db_panic(dbmfp->dbmp->dbenv, EINVAL);
      goto err;
  }

  if((*first_nonreused_chain_posp) >= CMPR_MAX) {
      __db_err(dbmfp->dbmp->dbenv, "__memp_cmpr_alloc: first_nonreused_chain_pos >= CMPR_MAX");
      ret = __db_panic(dbmfp->dbmp->dbenv, EINVAL);
      goto err;
  }

  /*
   * If possible reuse an existing chain.
   */
  if((*first_nonreused_chain_posp) >= 0 && F_ISSET(bhp, BH_CMPR) && bhp->chain[*first_nonreused_chain_posp]) {
    *pgnop = bhp->chain[*first_nonreused_chain_posp];
    (*first_nonreused_chain_posp)++;
#ifdef DEBUG
    fprintf(stderr,"__memp_cmpr_alloc:: reusing page in chain \n");
#endif
  } else {
    DB *db = dbmfp->cmpr_context.weakcmpr;
    DBC* cursor;
    DBT key;
    DBT data;
    int t_ret = 0;
    db_recno_t recno = 1;

    /* all pages in bhp->chain are now reused */
    (*first_nonreused_chain_posp) = -1;
#ifdef DEBUG
    fprintf(stderr,"__memp_cmpr_alloc:: no more resuable pages in chain\n");
#endif
    
    if(db == 0) {
      __db_err(dbmfp->dbmp->dbenv, "__memp_cmpr_alloc: dbmfp->cmpr_context.weakcmpr is null");
      ret = __db_panic(dbmfp->dbmp->dbenv, EINVAL);
      goto err;
    }
  
    /*
     * If the existing chain is too short, pop a free page from
     * the free pages database.
     */
    if((ret = db->cursor(db, NULL, &cursor, 0)) != 0)
      goto err;

    memset(&key, '\0', sizeof(DBT));
    memset(&data, '\0', sizeof(DBT));

    key.data = &recno;
    key.size = sizeof(recno);

    if((ret = cursor->c_get(cursor, &key, &data, DB_SET_RECNO | DB_RMW)) != 0) {
      /*
       * If the free list is empty, create a new page.
       */
#ifdef DEBUG
      fprintf(stderr,"__memp_cmpr_alloc:: no more free pages in weakcmpr! allocating\n");
#endif
      if(ret == DB_NOTFOUND) {
	DB_MPOOL *dbmp = dbmfp->dbmp;
	ret = 0;
	LOCKREGION(dbmp);
	++dbmfp->mfp->last_pgno;
	*pgnop = dbmfp->mfp->last_pgno;
	UNLOCKREGION(dbmp);
	ret = 0;
      } else {
	__db_err(dbmfp->dbmp->dbenv, "__memp_cmpr_alloc: unexpected error from weakcmpr base");
	ret = __db_panic(dbmfp->dbmp->dbenv, ret);
      }
    } else {
#ifdef DEBUG
      fprintf(stderr,"__memp_cmpr_alloc:: found free page  in weakcmpr! \n");
#endif
      if(key.size != sizeof(db_pgno_t)) {
	__db_err(dbmfp->dbmp->dbenv, "__memp_cmpr_alloc: unexpected key size from weakcmpr base (%d instead of %d)", key.size, sizeof(db_pgno_t));
	ret = __db_panic(dbmfp->dbmp->dbenv, ret);
      } else {
	memcpy((char*)pgnop, (char*)key.data, key.size);
	ret = cursor->c_del(cursor, 0);
	if(*pgnop == 0) {
	  __db_err(dbmfp->dbmp->dbenv, "__memp_cmpr_alloc: unexpected pgno == 0");
	  ret = __db_panic(dbmfp->dbmp->dbenv, ret);
	}
      }
    }

    if((t_ret = cursor->c_close(cursor)) != 0 && ret == 0)
	ret = t_ret;
  }

 err:
  return ret;
}

/*
 * __memp_cmpr_free --
 *	Release a page used to store weak compression data.
 *
 * PUBLIC: int __memp_cmpr_free __P((DB_MPOOLFILE *, db_pgno_t));
 */
int
__memp_cmpr_free(dbmfp, pgno)
     DB_MPOOLFILE *dbmfp;
     db_pgno_t pgno;
{
  int ret = 0;
  DB *db = dbmfp->cmpr_context.weakcmpr;
  DBT key;
  DBT data;
  CMPR cmpr;

#ifdef DEBUG
  fprintf(stderr,"__memp_cmpr_free::  freeing page (inserting into weakcmpr):%3d \n",pgno);
#endif
  if(db == 0) {
    __db_err(dbmfp->dbmp->dbenv, "__memp_cmpr_free: dbmfp->cmpr_context.weakcmpr is null");
    ret = __db_panic(dbmfp->dbmp->dbenv, EINVAL);
    goto err;
  }
  
  memset(&key, '\0', sizeof(DBT));
  memset(&data, '\0', sizeof(DBT));

  key.data = &pgno;
  key.size = sizeof(db_pgno_t);

  data.data = " ";
  data.size = 1;
  
  if((ret = db->put(db, 0, &key, &data, DB_NOOVERWRITE)) != 0) {
    __db_err(dbmfp->dbmp->dbenv, "__memp_cmpr_free: unexpected error from weakcmpr base");
    ret = __db_panic(dbmfp->dbmp->dbenv, ret);
    goto err;
  }
  
 err:
  return ret;
}


/*
 * __memp_cmpr_alloc_chain --
 *	Allocate chain entry in BH
 *
 * PUBLIC: int __memp_cmpr_alloc_chain __P((DB_MPOOLFILE *, BH *));
 */

int
__memp_cmpr_alloc_chain(dbmfp, bhp)
	DB_MPOOLFILE *dbmfp;
	BH *bhp;
{
    int ret;
    F_SET(bhp, BH_CMPR);
    if(!bhp->chain) {
	int alloc_ret;
	alloc_ret = __memp_alloc(dbmfp->dbmp, sizeof(db_pgno_t)*(CMPR_MAX-1), NULL, (void *)(&bhp->chain));
	memset((void *)bhp->chain, 0, sizeof(db_pgno_t)*(CMPR_MAX-1));
	if(alloc_ret) {
	    __db_err(dbmfp->dbmp->dbenv, "__memp_cmpr_read: memp_alloc failed:%d", alloc_ret);
	    ret = __db_panic(dbmfp->dbmp->dbenv, EINVAL);
	    goto err;
	}
#ifdef DEBUG
	fprintf(stderr,"__memp_cmpr_alloc_chain:: allocated chain:%8x\n",bhp->chain);
#endif

    }
 err:
    return ret;
}

/*
 * __memp_cmpr_free_chain --
 *	Free chain entry in BH
 *
 * PUBLIC: int __memp_cmpr_free_chain __P((DB_MPOOLFILE *, BH *));
 */

int
__memp_cmpr_free_chain(dbmfp, bhp)
	DB_MPOOLFILE *dbmfp;
	BH *bhp;
{
    if(F_ISSET(bhp, BH_CMPR) && bhp->chain) {
	__db_shalloc_free(dbmfp->dbmp->addr, bhp->chain);
	bhp->chain = NULL;
    }
}
#endif /* HAVE_LIBZ */
