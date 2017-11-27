
#ifndef _mp_ext_h_
#define _mp_ext_h_
int CDB___memp_alloc __P ((DB_MPOOL *,
                           REGINFO *, MPOOLFILE *, size_t, roff_t *, void *));
int CDB___memp_bhwrite __P ((DB_MPOOL *, MPOOLFILE *, BH *, int *, int *));
int CDB___memp_pgread __P ((DB_MPOOLFILE *, BH *, int));
int CDB___memp_pgwrite __P ((DB_MPOOL *, DB_MPOOLFILE *, BH *, int *, int *));
int CDB___memp_pg __P ((DB_MPOOLFILE *, BH *, int));
void CDB___memp_bhfree __P ((DB_MPOOL *, BH *, int));
int CDB___memp_fopen __P ((DB_MPOOL *, MPOOLFILE *, const char *,
                           u_int32_t, int, size_t, int, DB_MPOOL_FINFO *,
                           DB_MPOOLFILE **));
int CDB___memp_fremove __P ((DB_MPOOLFILE *));
char *CDB___memp_fn __P ((DB_MPOOLFILE *));
char *CDB___memp_fns __P ((DB_MPOOL *, MPOOLFILE *));
void CDB___memp_dbenv_create __P ((DB_ENV *));
int CDB___memp_open __P ((DB_ENV *));
int CDB___memp_close __P ((DB_ENV *));
void CDB___memp_dump_region __P ((DB_ENV *, char *, FILE *));
int CDB___mp_xxx_fh __P ((DB_MPOOLFILE *, DB_FH **));
int CDB___memp_cmpr __P ((DB_MPOOLFILE *, BH *, DB_IO *, int, ssize_t *));
int CDB___memp_cmpr_read __P ((DB_MPOOLFILE *, BH *, DB_IO *, ssize_t *));
int CDB___memp_cmpr_write __P ((DB_MPOOLFILE *, BH *, DB_IO *, ssize_t *));
int CDB___memp_cmpr_inflate
__P ((const u_int8_t *, int, u_int8_t *, int, void *));
int CDB___memp_cmpr_deflate
__P ((const u_int8_t *, int, u_int8_t **, int *, void *));
u_int8_t CDB___memp_cmpr_coefficient __P ((DB_ENV * dbenv));
int CDB___memp_cmpr_open
__P ((DB_ENV *, const char *, int, int, CMPR_CONTEXT *));
int CDB___memp_cmpr_close __P ((CMPR_CONTEXT *));
int CDB___memp_cmpr_alloc __P ((DB_MPOOLFILE *, db_pgno_t *, BH *, int *));
int CDB___memp_cmpr_free __P ((DB_MPOOLFILE *, db_pgno_t));
int CDB___memp_cmpr_alloc_chain __P ((DB_MPOOL *, BH *, int));
int CDB___memp_cmpr_free_chain __P ((DB_MPOOL *, BH *));
#endif /* _mp_ext_h_ */
