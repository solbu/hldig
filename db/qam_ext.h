/* DO NOT EDIT: automatically built by dist/distrib. */
#ifndef	_qam_ext_h_
#define	_qam_ext_h_
#if defined(__cplusplus)
extern "C" {
#endif
int CDB___qam_pitem
    __P((DBC *,  QPAGE *, u_int32_t, db_recno_t, DBT *));
int CDB___qam_put __P((DB *, DB_TXN *, DBT *, DBT *, u_int32_t));
int CDB___qam_delete __P((DB *, DB_TXN *, DBT *, u_int32_t));
int CDB___qam_c_dup __P((DBC *, DBC *));
int CDB___qam_c_init __P((DBC *));
int CDB___qam_mswap __P((PAGE *));
int CDB___qam_pgin_out __P((DB_ENV *, db_pgno_t, void *, DBT *));
int CDB___qam_db_create __P((DB *));
int CDB___qam_db_close __P((DB *));
int CDB___qam_open __P((DB *, const char *, db_pgno_t, u_int32_t));
int CDB___qam_metachk __P((DB *, const char *, QMETA *));
int CDB___qam_inc_recover __P((DB_ENV *, DBT *, DB_LSN *, db_recops, void *));
int CDB___qam_incfirst_recover
  __P((DB_ENV *, DBT *, DB_LSN *, db_recops, void *));
int CDB___qam_mvptr_recover
  __P((DB_ENV *, DBT *, DB_LSN *, db_recops, void *));
int CDB___qam_del_recover __P((DB_ENV *, DBT *, DB_LSN *, db_recops, void *));
int CDB___qam_add_recover __P((DB_ENV *, DBT *, DB_LSN *, db_recops, void *));
int CDB___qam_stat __P((DB *, void *, void *(*)(size_t), u_int32_t));
int CDB___qam_31_qammeta __P((DB *, char *, u_int8_t *));
int CDB___qam_vrfy_meta __P((DB *, VRFY_DBINFO *, QMETA *,
    db_pgno_t, u_int32_t));
int CDB___qam_vrfy_data __P((DB *, VRFY_DBINFO *, QPAGE *,
    db_pgno_t, u_int32_t));
int CDB___qam_vrfy_structure __P((DB *, VRFY_DBINFO *, u_int32_t));
#if defined(__cplusplus)
}
#endif
#endif /* _qam_ext_h_ */
