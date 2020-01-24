/* DO NOT EDIT: automatically built by dist/distrib. */
#ifndef _lock_ext_h_
#define _lock_ext_h_
int CDB___lock_downgrade __P ((DB_ENV *,
                               DB_LOCK *, db_lockmode_t, uint32_t));
int CDB___lock_addfamilylocker __P ((DB_ENV *, uint32_t, uint32_t));
int CDB___lock_freefamilylocker __P ((DB_LOCKTAB *, uint32_t));
void CDB___lock_freelocker __P ((DB_LOCKTAB *,
                                 DB_LOCKREGION *, DB_LOCKER *, uint32_t));
int CDB___lock_getlocker __P ((DB_LOCKTAB *,
                               uint32_t, uint32_t, int, DB_LOCKER **));
int CDB___lock_getobj __P ((DB_LOCKTAB *,
                            const DBT *, uint32_t, int, DB_LOCKOBJ **));
int CDB___lock_promote __P ((DB_LOCKTAB *, DB_LOCKOBJ *));
void CDB___lock_printlock __P ((DB_LOCKTAB *, struct __db_lock *, int));
void CDB___lock_dbenv_create __P ((DB_ENV *));
void CDB___lock_dbenv_close __P ((DB_ENV *));
int CDB___lock_open __P ((DB_ENV *));
int CDB___lock_close __P ((DB_ENV *));
void CDB___lock_dump_region __P ((DB_ENV *, char *, FILE *));
int CDB___lock_cmp __P ((const DBT *, DB_LOCKOBJ *));
int CDB___lock_locker_cmp __P ((uint32_t, DB_LOCKER *));
uint32_t CDB___lock_ohash __P ((const DBT *));
uint32_t CDB___lock_lhash __P ((DB_LOCKOBJ *));
uint32_t CDB___lock_locker_hash __P ((uint32_t));
#endif /* _lock_ext_h_ */
