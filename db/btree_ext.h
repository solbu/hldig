/* DO NOT EDIT: automatically built by dist/distrib. */
#ifndef _btree_ext_h_
#define _btree_ext_h_
int CDB___bam_cmp __P((DB *, const DBT *,
   PAGE *, u_int32_t, int (*)(const DBT *, const DBT *)));
int CDB___bam_defcmp __P((const DBT *, const DBT *));
size_t CDB___bam_defpfx __P((const DBT *, const DBT *));
int CDB___bam_pgin __P((db_pgno_t, void *, DBT *));
int CDB___bam_pgout __P((db_pgno_t, void *, DBT *));
int CDB___bam_mswap __P((PAGE *));
int CDB___bam_cprint __P((DB *));
int CDB___bam_ca_delete __P((DB *, db_pgno_t, u_int32_t, int));
void CDB___bam_ca_di __P((DB *, db_pgno_t, u_int32_t, int));
void CDB___bam_ca_dup __P((DB *,
   db_pgno_t, u_int32_t, u_int32_t, db_pgno_t, u_int32_t));
void CDB___bam_ca_rsplit __P((DB *, db_pgno_t, db_pgno_t));
void CDB___bam_ca_split __P((DB *,
   db_pgno_t, db_pgno_t, db_pgno_t, u_int32_t, int));
void CDB___bam_ca_repl __P((DB *,
   db_pgno_t, u_int32_t, db_pgno_t, u_int32_t));
int CDB___bam_c_init __P((DBC *));
int CDB___bam_c_dup __P((DBC *, DBC *));
int CDB___bam_delete __P((DB *, DB_TXN *, DBT *, u_int32_t));
int CDB___bam_ditem __P((DBC *, PAGE *, u_int32_t));
int CDB___bam_adjindx __P((DBC *, PAGE *, u_int32_t, u_int32_t, int));
int CDB___bam_dpage __P((DBC *, const DBT *));
int CDB___bam_dpages __P((DBC *));
int CDB___bam_db_create __P((DB *));
int CDB___bam_db_close __P((DB *));
int CDB___bam_set_flags __P((DB *, u_int32_t *flagsp));
int CDB___ram_set_flags __P((DB *, u_int32_t *flagsp));
int CDB___bam_open __P((DB *, const char *, db_pgno_t));
void CDB___bam_setovflsize __P((DB *));
int CDB___bam_metachk __P((DB *, const char *, BTMETA *));
int CDB___bam_read_root __P((DB *, const char *, db_pgno_t));
int CDB___bam_iitem __P((DBC *,
   PAGE **, db_indx_t *, DBT *, DBT *, u_int32_t, u_int32_t));
u_int32_t CDB___bam_partsize __P((u_int32_t, DBT *, PAGE *, u_int32_t));
int CDB___bam_build __P((DBC *, u_int32_t,
    DBT *, PAGE *, u_int32_t, u_int32_t));
int CDB___bam_ritem __P((DBC *, PAGE *, u_int32_t, DBT *));
int CDB___bam_pg_alloc_recover
  __P((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___bam_pg_free_recover
  __P((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___bam_split_recover
  __P((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___bam_rsplit_recover
  __P((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___bam_adj_recover
  __P((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___bam_cadjust_recover
  __P((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___bam_cdel_recover
  __P((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___bam_repl_recover
  __P((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___bam_root_recover
  __P((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___bam_reclaim __P((DB *, DB_TXN *));
int CDB___ram_open __P((DB *, const char *, db_pgno_t));
int CDB___ram_c_del __P((DBC *, u_int32_t));
int CDB___ram_c_get __P((DBC *, DBT *, DBT *, u_int32_t));
int CDB___ram_c_put __P((DBC *, DBT *, DBT *, u_int32_t));
void CDB___ram_ca __P((DB *, db_recno_t, ca_recno_arg));
int CDB___ram_getno __P((DBC *, const DBT *, db_recno_t *, int));
int CDB___ram_writeback __P((DB *));
int CDB___bam_rsearch __P((DBC *, db_recno_t *, u_int32_t, int, int *));
int CDB___bam_adjust __P((DBC *, int32_t));
int CDB___bam_nrecs __P((DBC *, db_recno_t *));
db_recno_t CDB___bam_total __P((PAGE *));
int CDB___bam_search __P((DBC *,
    const DBT *, u_int32_t, int, db_recno_t *, int *));
int CDB___bam_stkrel __P((DBC *, int));
int CDB___bam_stkgrow __P((BTREE_CURSOR *));
int CDB___bam_split __P((DBC *, void *));
int CDB___bam_copy __P((DB *, PAGE *, PAGE *, u_int32_t, u_int32_t));
int CDB___bam_stat __P((DB *, void *, void *(*)(size_t), u_int32_t));
int CDB___bam_traverse __P((DBC *, db_lockmode_t,
    db_pgno_t, int (*)(DB *, PAGE *, void *, int *), void *));
int CDB___bam_upgrade __P((DB *, int, char *, DB_FH *, char *));
int CDB___bam_init_recover __P((DB_ENV *));
#endif /* _btree_ext_h_ */
