/* DO NOT EDIT: automatically built by dist/distrib. */
#ifndef _common_ext_h_
#define _common_ext_h_
int CDB___db_byteorder __P ((DB_ENV *, int));
int CDB___db_fchk __P ((DB_ENV *, const char *, u_int32_t, u_int32_t));
int CDB___db_fcchk
__P ((DB_ENV *, const char *, u_int32_t, u_int32_t, u_int32_t));
int CDB___db_ferr __P ((const DB_ENV *, const char *, int));
int CDB___db_pgerr __P ((DB *, db_pgno_t));
int CDB___db_pgfmt __P ((DB *, db_pgno_t));
#ifdef DIAGNOSTIC
void __db_assert __P ((const char *, const char *, int));
#endif
int CDB___db_panic_msg __P ((DB_ENV *));
int CDB___db_panic __P ((DB_ENV *, int));
#ifdef __STDC__
void CDB___db_err __P ((const DB_ENV *, const char *, ...));
#else
void CDB___db_err ();
#endif
void CDB___db_real_err
__P ((const DB_ENV *, int, int, int, const char *, va_list));
#ifdef __STDC__
int CDB___db_logmsg __P ((DB_ENV *,
                          DB_TXN *, const char *, u_int32_t, const char *,
                          ...));
#else
int CDB___db_logmsg ();
#endif
int CDB___db_getlong __P ((DB *, const char *, char *, long, long, long *));
u_int32_t CDB___db_log2 __P ((u_int32_t));
#endif /* _common_ext_h_ */
