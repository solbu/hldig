/* Do not edit: automatically built by gen_rec.awk. */

#ifndef log_AUTO_H
#define log_AUTO_H

#define  DB_log_register  (DB_log_BEGIN + 1)

typedef struct _log_register_args
{
  uint32_t type;
  DB_TXN *txnid;
  DB_LSN prev_lsn;
  uint32_t opcode;
  DBT name;
  DBT uid;
  uint32_t id;
  DBTYPE ftype;
} __log_register_args;

int CDB___log_register_log
__P ((DB_ENV *, DB_TXN *, DB_LSN *, uint32_t, uint32_t, const DBT *,
      const DBT *, uint32_t, DBTYPE));
int CDB___log_register_print __P ((DB_ENV *, DBT *, DB_LSN *, int, void *));
int CDB___log_register_read __P ((void *, __log_register_args **));
int CDB___log_init_print __P ((DB_ENV *));
#endif
