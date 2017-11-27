/* DO NOT EDIT: automatically built by dist/distrib. */
#ifndef _log_ext_h_
#define _log_ext_h_
int CDB___log_open __P ((DB_ENV *));
int CDB___log_find __P ((DB_LOG *, int, int *));
int CDB___log_valid __P ((DB_LOG *, u_int32_t, int));
int CDB___log_close __P ((DB_ENV *));
int CDB___log_init_recover __P ((DB_ENV *));
int CDB___log_findckp __P ((DB_ENV *, DB_LSN *));
int CDB___log_get __P ((DB_LOG *, DB_LSN *, DBT *, u_int32_t, int));
void CDB___log_dbenv_create __P ((DB_ENV *));
int CDB___log_put __P ((DB_ENV *, DB_LSN *, const DBT *, u_int32_t));
int CDB___log_name __P ((DB_LOG *, u_int32_t, char **, DB_FH *, u_int32_t));
int CDB___log_register_recover __P ((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___log_add_logid __P ((DB_LOG *, DB *, u_int32_t));
int CDB___db_fileid_to_db __P ((DB_ENV *, DB **, int32_t, int));
void CDB___log_close_files __P ((DB_ENV *));
void CDB___log_rem_logid __P ((DB_LOG *, u_int32_t));
#endif /* _log_ext_h_ */
