/* DO NOT EDIT: automatically built by dist/distrib. */
#ifndef	_lock_ext_h_
#define	_lock_ext_h_
#if defined(__cplusplus)
extern "C" {
#endif
int CDB___lock_downgrade __P((DB_ENV *,
    DB_LOCK *, db_lockmode_t, u_int32_t));
int CDB___lock_addfamilylocker __P((DB_ENV *, u_int32_t, u_int32_t));
int CDB___lock_freefamilylocker  __P((DB_LOCKTAB *, u_int32_t));
void CDB___lock_freelocker __P((DB_LOCKTAB *,
    DB_LOCKREGION *, DB_LOCKER *, u_int32_t));
int CDB___lock_getlocker __P((DB_LOCKTAB *,
    u_int32_t, u_int32_t, int, DB_LOCKER **));
int CDB___lock_getobj __P((DB_LOCKTAB *,
    const DBT *, u_int32_t, int, DB_LOCKOBJ **));
int CDB___lock_promote __P((DB_LOCKTAB *, DB_LOCKOBJ *));
void CDB___lock_printlock __P((DB_LOCKTAB *, struct __db_lock *, int));
void CDB___lock_dbenv_create __P((DB_ENV *));
void CDB___lock_dbenv_close __P((DB_ENV *));
int CDB___lock_open __P((DB_ENV *));
int CDB___lock_close __P((DB_ENV *));
void CDB___lock_dump_region __P((DB_ENV *, char *, FILE *));
int CDB___lock_cmp __P((const DBT *, DB_LOCKOBJ *));
int CDB___lock_locker_cmp __P((u_int32_t, DB_LOCKER *));
u_int32_t CDB___lock_ohash __P((const DBT *));
u_int32_t CDB___lock_lhash __P((DB_LOCKOBJ *));
u_int32_t CDB___lock_locker_hash __P((u_int32_t));
#if defined(__cplusplus)
}
#endif
#endif /* _lock_ext_h_ */
