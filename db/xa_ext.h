/* DO NOT EDIT: automatically built by dist/distrib. */
#ifndef _xa_ext_h_
#define _xa_ext_h_
int CDB___db_xa_create __P((DB *));
int CDB___db_rmid_to_env __P((int rmid, DB_ENV **envp));
int CDB___db_xid_to_txn __P((DB_ENV *, XID *, size_t *));
int CDB___db_map_rmid __P((int, DB_ENV *));
int CDB___db_unmap_rmid __P((int));
int CDB___db_map_xid __P((DB_ENV *, XID *, size_t));
void CDB___db_unmap_xid __P((DB_ENV *, XID *, size_t));
#endif /* _xa_ext_h_ */
