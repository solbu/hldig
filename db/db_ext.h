/* DO NOT EDIT: automatically built by dist/distrib. */
#ifndef _db_ext_h_
#define _db_ext_h_
int CDB___crdel_init_recover __P ((DB_ENV *));
int CDB___crdel_fileopen_recover
__P ((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___crdel_metasub_recover
__P ((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___crdel_metapage_recover
__P ((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___crdel_delete_recover __P ((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___db_open __P ((DB *,
                        const char *, const char *, DBTYPE, u_int32_t, int));
int CDB___db_close __P ((DB *, u_int32_t));
int CDB___db_remove __P ((DB *, const char *, const char *, u_int32_t));
int CDB___db_backup_name __P ((const char *, char **, DB_LSN *));
int __db_testcopy __P ((DB *, const char *));
int CDB___db_cursor __P ((DB *, DB_TXN *, DBC **, u_int32_t));
int CDB___db_c_dup __P ((DBC *, DBC **, u_int32_t));
int CDB___db_cprint __P ((DB *));
int CDB___db_c_destroy __P ((DBC *));
int CDB___db_fd __P ((DB *, int *));
int CDB___db_get __P ((DB *, DB_TXN *, DBT *, DBT *, u_int32_t));
int CDB___db_put __P ((DB *, DB_TXN *, DBT *, DBT *, u_int32_t));
int CDB___db_sync __P ((DB *, u_int32_t));
int CDB___db_log_page __P ((DB *, const char *, DB_LSN *, db_pgno_t, PAGE *));
int CDB___db_init_recover __P ((DB_ENV *));
int CDB___db_pgin __P ((db_pgno_t, void *, DBT *));
int CDB___db_pgout __P ((db_pgno_t, void *, DBT *));
void CDB___db_metaswap __P ((PAGE *));
int CDB___db_byteswap __P ((db_pgno_t, PAGE *, size_t, int));
int CDB___db_dispatch __P ((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___db_add_recovery __P ((DB_ENV *,
                                int (*)(DB_ENV *, DBT *, DB_LSN *, int,
                                        void *), u_int32_t));
int CDB___db_txnlist_init __P ((void *));
int CDB___db_txnlist_add __P ((void *, u_int32_t));
int CDB___db_txnlist_close __P ((void *, u_int32_t, u_int32_t));
int CDB___db_txnlist_delete __P ((void *, char *, u_int32_t, int));
void CDB___db_txnlist_end __P ((DB_ENV *, void *));
int CDB___db_txnlist_find __P ((void *, u_int32_t));
void CDB___db_txnlist_gen __P ((void *, int));
void CDB___db_txnlist_print __P ((void *));
int CDB___db_dput __P ((DBC *, DBT *, PAGE **, db_indx_t *));
int CDB___db_drem __P ((DBC *, PAGE **, u_int32_t));
int CDB___db_dend __P ((DBC *, db_pgno_t, PAGE **));
int CDB___db_ditem __P ((DBC *, PAGE *, u_int32_t, u_int32_t));
int CDB___db_pitem __P ((DBC *, PAGE *, u_int32_t, u_int32_t, DBT *, DBT *));
int CDB___db_relink __P ((DBC *, u_int32_t, PAGE *, PAGE **, int));
int CDB___db_ddup __P ((DBC *, db_pgno_t));
int CDB___db_dsearch __P ((DBC *,
                           int, DBT *, db_pgno_t, db_indx_t *, PAGE **,
                           int *));
int CDB___db_cursorchk __P ((const DB *, u_int32_t, int));
int CDB___db_cdelchk __P ((const DB *, u_int32_t, int, int));
int CDB___db_cgetchk __P ((const DB *, DBT *, DBT *, u_int32_t, int));
int CDB___db_cputchk __P ((const DB *,
                           const DBT *, DBT *, u_int32_t, int, int));
int CDB___db_closechk __P ((const DB *, u_int32_t));
int CDB___db_delchk __P ((const DB *, DBT *, u_int32_t, int));
int CDB___db_getchk __P ((const DB *, const DBT *, DBT *, u_int32_t));
int CDB___db_joinchk __P ((const DB *, u_int32_t));
int CDB___db_putchk
__P ((const DB *, DBT *, const DBT *, u_int32_t, int, int));
int CDB___db_statchk __P ((const DB *, u_int32_t));
int CDB___db_syncchk __P ((const DB *, u_int32_t));
int CDB___db_eopnotsup __P ((const DB_ENV *));
int CDB___db_removechk __P ((const DB *, u_int32_t));
int CDB___db_join __P ((DB *, DBC **, DBC **, u_int32_t));
int CDB___db_new __P ((DBC *, u_int32_t, PAGE **));
int CDB___db_free __P ((DBC *, PAGE *));
int CDB___db_lt __P ((DBC *));
int CDB___db_lget __P ((DBC *,
                        int, db_pgno_t, db_lockmode_t, int, DB_LOCK *));
int CDB___dbh_am_chk __P ((DB *, u_int32_t));
int CDB___db_goff __P ((DB *, DBT *,
                        u_int32_t, db_pgno_t, void **, u_int32_t *));
int CDB___db_poff __P ((DBC *, const DBT *, db_pgno_t *));
int CDB___db_ovref __P ((DBC *, db_pgno_t, int32_t));
int CDB___db_doff __P ((DBC *, db_pgno_t));
int CDB___db_moff __P ((DB *, const DBT *, db_pgno_t, u_int32_t,
                        int (*)(const DBT *, const DBT *), int *));
void CDB___db_loadme __P ((void));
int CDB___db_dump __P ((DB *, char *, char *));
int CDB___db_prnpage __P ((DB *, db_pgno_t));
int CDB___db_prpage __P ((DB *, PAGE *, u_int32_t));
int CDB___db_isbad __P ((PAGE *, int));
void CDB___db_pr __P ((u_int8_t *, u_int32_t));
int CDB___db_prdbt __P ((DBT *, int, const char *, FILE *, int));
void CDB___db_prflags __P ((u_int32_t, const FN *, FILE *));
int CDB___db_addrem_recover __P ((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___db_split_recover __P ((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___db_big_recover __P ((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___db_ovref_recover __P ((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___db_relink_recover __P ((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___db_addpage_recover __P ((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___db_debug_recover __P ((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___db_noop_recover __P ((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___db_traverse_dup __P ((DB *,
                                db_pgno_t, int (*)(DB *, PAGE *, void *,
                                                   int *), void *));
int CDB___db_traverse_big
__P ((DB *, db_pgno_t, int (*)(DB *, PAGE *, void *, int *), void *));
int CDB___db_reclaim_callback __P ((DB *, PAGE *, void *, int *));
int CDB___db_ret __P ((DB *, PAGE *, u_int32_t, DBT *, void **, u_int32_t *));
int CDB___db_retcopy __P ((DB *, DBT *,
                           void *, u_int32_t, void **, u_int32_t *));
int CDB___db_upgrade __P ((DB *, const char *, u_int32_t));
#endif /* _db_ext_h_ */
