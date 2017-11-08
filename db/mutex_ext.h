/* DO NOT EDIT: automatically built by dist/distrib. */
#ifndef _mutex_ext_h_
#define _mutex_ext_h_
int CDB___db_fcntl_mutex_init __P((DB_ENV *, MUTEX *, u_int32_t));
int CDB___db_fcntl_mutex_lock __P((MUTEX *, DB_FH *));
int CDB___db_fcntl_mutex_unlock __P((MUTEX *));
int CDB___db_pthread_mutex_init __P((DB_ENV *, MUTEX *, u_int32_t));
int CDB___db_pthread_mutex_lock __P((MUTEX *));
int CDB___db_pthread_mutex_unlock __P((MUTEX *));
int CDB___db_tas_mutex_init __P((DB_ENV *, MUTEX *, u_int32_t));
int CDB___db_tas_mutex_lock __P((MUTEX *));
int CDB___db_tas_mutex_unlock __P((MUTEX *));
int CDB___db_mutex_alloc __P((DB_ENV *, REGINFO *, MUTEX **));
void CDB___db_mutex_free __P((DB_ENV *, REGINFO *, MUTEX *));
#endif /* _mutex_ext_h_ */
