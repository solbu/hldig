/* Do not edit: automatically built by gen_rec.awk. */

#ifndef bam_AUTO_H
#define bam_AUTO_H

#define  DB_bam_pg_alloc  (DB_bam_BEGIN + 1)

typedef struct _bam_pg_alloc_args
{
  uint32_t type;
  DB_TXN *txnid;
  DB_LSN prev_lsn;
  int32_t fileid;
  DB_LSN meta_lsn;
  DB_LSN page_lsn;
  db_pgno_t pgno;
  uint32_t ptype;
  db_pgno_t next;
} __bam_pg_alloc_args;

int CDB___bam_pg_alloc_log
__P ((DB_ENV *, DB_TXN *, DB_LSN *, uint32_t, int32_t, DB_LSN *, DB_LSN *,
      db_pgno_t, uint32_t, db_pgno_t));
int CDB___bam_pg_alloc_print __P ((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___bam_pg_alloc_read __P ((void *, __bam_pg_alloc_args **));

#define  DB_bam_pg_free  (DB_bam_BEGIN + 2)

typedef struct _bam_pg_free_args
{
  uint32_t type;
  DB_TXN *txnid;
  DB_LSN prev_lsn;
  int32_t fileid;
  db_pgno_t pgno;
  DB_LSN meta_lsn;
  DBT header;
  db_pgno_t next;
} __bam_pg_free_args;

int CDB___bam_pg_free_log
__P ((DB_ENV *, DB_TXN *, DB_LSN *, uint32_t, int32_t, db_pgno_t, DB_LSN *,
      const DBT *, db_pgno_t));
int CDB___bam_pg_free_print __P ((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___bam_pg_free_read __P ((void *, __bam_pg_free_args **));

#define  DB_bam_split  (DB_bam_BEGIN + 3)

typedef struct _bam_split_args
{
  uint32_t type;
  DB_TXN *txnid;
  DB_LSN prev_lsn;
  int32_t fileid;
  db_pgno_t left;
  DB_LSN llsn;
  db_pgno_t right;
  DB_LSN rlsn;
  uint32_t indx;
  db_pgno_t npgno;
  DB_LSN nlsn;
  DBT pg;
} __bam_split_args;

int CDB___bam_split_log
__P ((DB_ENV *, DB_TXN *, DB_LSN *, uint32_t, int32_t, db_pgno_t, DB_LSN *,
      db_pgno_t, DB_LSN *, uint32_t, db_pgno_t, DB_LSN *, const DBT *));
int CDB___bam_split_print __P ((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___bam_split_read __P ((void *, __bam_split_args **));

#define  DB_bam_rsplit  (DB_bam_BEGIN + 4)

typedef struct _bam_rsplit_args
{
  uint32_t type;
  DB_TXN *txnid;
  DB_LSN prev_lsn;
  int32_t fileid;
  db_pgno_t pgno;
  DBT pgdbt;
  db_pgno_t nrec;
  DBT rootent;
  DB_LSN rootlsn;
} __bam_rsplit_args;

int CDB___bam_rsplit_log
__P ((DB_ENV *, DB_TXN *, DB_LSN *, uint32_t, int32_t, db_pgno_t,
      const DBT *, db_pgno_t, const DBT *, DB_LSN *));
int CDB___bam_rsplit_print __P ((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___bam_rsplit_read __P ((void *, __bam_rsplit_args **));

#define  DB_bam_adj  (DB_bam_BEGIN + 5)

typedef struct _bam_adj_args
{
  uint32_t type;
  DB_TXN *txnid;
  DB_LSN prev_lsn;
  int32_t fileid;
  db_pgno_t pgno;
  DB_LSN lsn;
  uint32_t indx;
  uint32_t indx_copy;
  uint32_t is_insert;
} __bam_adj_args;

int CDB___bam_adj_log
__P ((DB_ENV *, DB_TXN *, DB_LSN *, uint32_t, int32_t, db_pgno_t, DB_LSN *,
      uint32_t, uint32_t, uint32_t));
int CDB___bam_adj_print __P ((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___bam_adj_read __P ((void *, __bam_adj_args **));

#define  DB_bam_cadjust  (DB_bam_BEGIN + 6)

typedef struct _bam_cadjust_args
{
  uint32_t type;
  DB_TXN *txnid;
  DB_LSN prev_lsn;
  int32_t fileid;
  db_pgno_t pgno;
  DB_LSN lsn;
  uint32_t indx;
  int32_t adjust;
  int32_t total;
} __bam_cadjust_args;

int CDB___bam_cadjust_log
__P ((DB_ENV *, DB_TXN *, DB_LSN *, uint32_t, int32_t, db_pgno_t, DB_LSN *,
      uint32_t, int32_t, int32_t));
int CDB___bam_cadjust_print __P ((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___bam_cadjust_read __P ((void *, __bam_cadjust_args **));

#define  DB_bam_cdel  (DB_bam_BEGIN + 7)

typedef struct _bam_cdel_args
{
  uint32_t type;
  DB_TXN *txnid;
  DB_LSN prev_lsn;
  int32_t fileid;
  db_pgno_t pgno;
  DB_LSN lsn;
  uint32_t indx;
} __bam_cdel_args;

int CDB___bam_cdel_log
__P ((DB_ENV *, DB_TXN *, DB_LSN *, uint32_t, int32_t, db_pgno_t, DB_LSN *,
      uint32_t));
int CDB___bam_cdel_print __P ((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___bam_cdel_read __P ((void *, __bam_cdel_args **));

#define  DB_bam_repl  (DB_bam_BEGIN + 8)

typedef struct _bam_repl_args
{
  uint32_t type;
  DB_TXN *txnid;
  DB_LSN prev_lsn;
  int32_t fileid;
  db_pgno_t pgno;
  DB_LSN lsn;
  uint32_t indx;
  uint32_t isdeleted;
  DBT orig;
  DBT repl;
  uint32_t prefix;
  uint32_t suffix;
} __bam_repl_args;

int CDB___bam_repl_log
__P ((DB_ENV *, DB_TXN *, DB_LSN *, uint32_t, int32_t, db_pgno_t, DB_LSN *,
      uint32_t, uint32_t, const DBT *, const DBT *, uint32_t, uint32_t));
int CDB___bam_repl_print __P ((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___bam_repl_read __P ((void *, __bam_repl_args **));

#define  DB_bam_root  (DB_bam_BEGIN + 9)

typedef struct _bam_root_args
{
  uint32_t type;
  DB_TXN *txnid;
  DB_LSN prev_lsn;
  int32_t fileid;
  db_pgno_t meta_pgno;
  db_pgno_t root_pgno;
  DB_LSN meta_lsn;
} __bam_root_args;

int CDB___bam_root_log
__P ((DB_ENV *, DB_TXN *, DB_LSN *, uint32_t, int32_t, db_pgno_t, db_pgno_t,
      DB_LSN *));
int CDB___bam_root_print __P ((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___bam_root_read __P ((void *, __bam_root_args **));
int CDB___bam_init_print __P ((DB_ENV *));
#endif
