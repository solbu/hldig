/* DO NOT EDIT: automatically built by dist/distrib. */
#ifndef	_txn_ext_h_
#define	_txn_ext_h_
#if defined(__cplusplus)
extern "C" {
#endif
int CDB___txn_xa_begin __P((DB_ENV *, DB_TXN *));
int CDB___txn_end __P((DB_TXN *, int));
int CDB___txn_activekids __P((DB_TXN *));
int CDB___txn_regop_recover
   __P((DB_ENV *, DBT *, DB_LSN *, db_recops, void *));
int CDB___txn_xa_regop_recover
   __P((DB_ENV *, DBT *, DB_LSN *, db_recops, void *));
int CDB___txn_ckp_recover
__P((DB_ENV *, DBT *, DB_LSN *, db_recops, void *));
int CDB___txn_child_recover
   __P((DB_ENV *, DBT *, DB_LSN *, db_recops, void *));
void CDB___txn_dbenv_create __P((DB_ENV *));
int CDB___txn_open __P((DB_ENV *));
int CDB___txn_close __P((DB_ENV *));
#if defined(__cplusplus)
}
#endif
#endif /* _txn_ext_h_ */
