/* DO NOT EDIT: automatically built by dist/distrib. */
#ifndef _os_ext_h_
#define _os_ext_h_
int CDB___os_abspath __P((const char *));
int CDB___os_strdup __P((const char *, void *));
int CDB___os_calloc __P((size_t, size_t, void *));
int CDB___os_malloc __P((size_t, void *(*)(size_t), void *));
int CDB___os_realloc __P((size_t, void *(*)(void *, size_t), void *));
void CDB___os_free __P((void *, size_t));
void CDB___os_freestr __P((void *));
void *CDB___ua_memcpy __P((void *, const void *, size_t));
int CDB___os_dirlist __P((const char *, char ***, int *));
void CDB___os_dirfree __P((char **, int));
int CDB___os_get_errno __P((void));
void CDB___os_set_errno __P((int));
int CDB___os_fileid __P((DB_ENV *, const char *, int, u_int8_t *));
int CDB___os_finit __P((DB_FH *, size_t, int));
int CDB___os_fpinit __P((DB_FH *, db_pgno_t, int, int));
int CDB___os_fsync __P((DB_FH *));
int CDB___os_openhandle __P((const char *, int, int, DB_FH *));
int CDB___os_closehandle __P((DB_FH *));
int CDB___os_r_sysattach __P((DB_ENV *, REGINFO *, REGION *));
int CDB___os_r_sysdetach __P((DB_ENV *, REGINFO *, int));
int CDB___os_mapfile __P((DB_ENV *,
    char *, DB_FH *, size_t, int, void **));
int CDB___os_unmapfile __P((DB_ENV *, void *, size_t));
void CDB___os_dbenv_create __P((DB_ENV *));
u_int32_t CDB___db_oflags __P((int));
int CDB___db_omode __P((const char *));
int CDB___os_open __P((const char *, u_int32_t, int, DB_FH *));
int CDB___os_r_attach __P((DB_ENV *, REGINFO *, REGION *));
int CDB___os_r_detach __P((DB_ENV *, REGINFO *, int));
int CDB___os_rename __P((const char *, const char *));
int CDB___os_isroot __P((void));
char *CDB___db_rpath __P((const char *));
int CDB___os_io __P((DB_IO *, int, ssize_t *));
int CDB___os_read __P((DB_FH *, void *, size_t, ssize_t *));
int CDB___os_write __P((DB_FH *, void *, size_t, ssize_t *));
int CDB___os_seek
    __P((DB_FH *, size_t, db_pgno_t, u_int32_t, int, DB_OS_SEEK));
int CDB___os_sleep __P((u_long, u_long));
int CDB___os_spin __P((void));
void CDB___os_yield __P((u_long));
int CDB___os_exists __P((const char *, int *));
int CDB___os_ioinfo __P((const char *,
   DB_FH *, u_int32_t *, u_int32_t *, u_int32_t *));
int CDB___os_tmpdir __P((DB_ENV *, u_int32_t));
int CDB___os_unlink __P((const char *));
#if defined(_WIN32)
int __os_win32_errno __P((void));
#endif
int CDB___os_fpinit __P((DB_FH *, db_pgno_t, int, int));
int __os_is_winnt __P((void));
#endif /* _os_ext_h_ */
