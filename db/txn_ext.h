/* DO NOT EDIT: automatically built by dist/distrib. */
#ifndef _txn_ext_h_
#define _txn_ext_h_
int CDB___txn_xa_begin __P((DB_ENV *, DB_TXN *));
int CDB___txn_end __P((DB_TXN *, int));
int CDB___txn_is_ancestor __P((DB_ENV *, size_t, size_t));
int CDB___txn_activekids __P((DB_TXN *));
int CDB___txn_init_recover __P((DB_ENV *));
int CDB___txn_regop_recover
   __P((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___txn_xa_regop_recover
   __P((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___txn_ckp_recover __P((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___txn_child_recover
   __P((DB_ENV *, DBT *, DB_LSN *, int, void *));
void CDB___txn_dbenv_create __P((DB_ENV *));
int CDB___txn_open __P((DB_ENV *));
int CDB___txn_close __P((DB_ENV *));
#endif /* _txn_ext_h_ */
