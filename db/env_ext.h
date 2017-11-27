/* DO NOT EDIT: automatically built by dist/distrib. */
#ifndef _env_ext_h_
#define _env_ext_h_
void CDB___db_shalloc_init __P ((void *, size_t));
int CDB___db_shalloc __P ((void *, size_t, size_t, void *));
void CDB___db_shalloc_free __P ((void *, void *));
size_t CDB___db_shalloc_count __P ((void *));
size_t CDB___db_shsizeof __P ((void *));
void CDB___db_shalloc_dump __P ((void *, FILE *));
int CDB___db_tablesize __P ((u_int32_t));
void CDB___db_hashinit __P ((void *, u_int32_t));
int CDB___dbenv_init __P ((DB_ENV *));
int CDB___db_mi_env __P ((DB_ENV *, const char *));
int CDB___db_mi_open __P ((DB_ENV *, const char *, int));
int CDB___db_env_config __P ((DB_ENV *, int));
int CDB___dbenv_open __P ((DB_ENV *,
                           const char *, char *const *, u_int32_t, int));
int CDB___dbenv_remove __P ((DB_ENV *,
                             const char *, char *const *, u_int32_t));
int CDB___dbenv_close __P ((DB_ENV *, u_int32_t));
int CDB___db_appname __P ((DB_ENV *, APPNAME,
                           const char *, const char *, u_int32_t, DB_FH *,
                           char **));
int CDB___db_apprec __P ((DB_ENV *, u_int32_t));
int CDB___db_e_attach __P ((DB_ENV *));
int CDB___db_e_detach __P ((DB_ENV *, int));
int CDB___db_e_remove __P ((DB_ENV *, int));
int CDB___db_e_stat __P ((DB_ENV *, REGENV *, REGION *, int *));
int CDB___db_r_attach __P ((DB_ENV *, REGINFO *, size_t));
int CDB___db_r_detach __P ((DB_ENV *, REGINFO *, int));
#endif /* _env_ext_h_ */
