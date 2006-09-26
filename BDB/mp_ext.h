
#ifndef _mp_ext_h_
#define _mp_ext_h_
int CDB___memp_alloc __P((DB_MPOOL *,
    REGINFO *, MPOOLFILE *, size_t, roff_t *, void *));
int CDB___memp_bhwrite
    __P((DB_MPOOL *, MPOOLFILE *, BH *, int *, int *));
int CDB___memp_pgread __P((DB_MPOOLFILE *, BH *, int));
int CDB___memp_pgwrite
    __P((DB_MPOOL *, DB_MPOOLFILE *, BH *, int *, int *));
int CDB___memp_pg __P((DB_MPOOLFILE *, BH *, int));
void CDB___memp_bhfree __P((DB_MPOOL *, BH *, int));
int CDB___memp_fopen __P((DB_MPOOL *, MPOOLFILE *, const char *,
   u_int32_t, int, size_t, int, DB_MPOOL_FINFO *, DB_MPOOLFILE **));
int CDB___memp_fremove __P((DB_MPOOLFILE *));
char * CDB___memp_fn __P((DB_MPOOLFILE *));
char * CDB___memp_fns __P((DB_MPOOL *, MPOOLFILE *));
void CDB___memp_dbenv_create __P((DB_ENV *));
int CDB___memp_open __P((DB_ENV *));
int CDB___memp_close __P((DB_ENV *));
void CDB___memp_dump_region __P((DB_ENV *, char *, FILE *));
int CDB___mp_xxx_fh __P((DB_MPOOLFILE *, DB_FH **));
#endif /* _mp_ext_h_ */
