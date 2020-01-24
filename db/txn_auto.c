/* Do not edit: automatically built by gen_rec.awk. */
#include <errno.h>
#include "db_config.h"

#ifndef NO_SYSTEM_INCLUDES
#include <sys/types.h>

#include <ctype.h>
#include <string.h>
#endif

#include "db_int.h"
#include "db_page.h"
#include "db_dispatch.h"
#include "db_am.h"
#include "txn.h"

int
CDB___txn_regop_log (dbenv, txnid, ret_lsnp, flags, opcode)
     DB_ENV *dbenv;
     DB_TXN *txnid;
     DB_LSN *ret_lsnp;
     uint32_t flags;
     uint32_t opcode;
{
  DBT logrec;
  DB_LSN *lsnp, null_lsn;
  uint32_t rectype, txn_num;
  int ret;
  uint8_t *bp;

  if (txnid != NULL &&
      TAILQ_FIRST (&txnid->kids) != NULL && CDB___txn_activekids (txnid) != 0)
    return (EPERM);
  rectype = DB_txn_regop;
  txn_num = txnid == NULL ? 0 : txnid->txnid;
  if (txnid == NULL)
  {
    ZERO_LSN (null_lsn);
    lsnp = &null_lsn;
  }
  else
    lsnp = &txnid->last_lsn;
  logrec.size = sizeof (rectype) + sizeof (txn_num) + sizeof (DB_LSN)
    + sizeof (opcode);
  if ((ret = CDB___os_malloc (logrec.size, NULL, &logrec.data)) != 0)
    return (ret);

  bp = logrec.data;
  memcpy (bp, &rectype, sizeof (rectype));
  bp += sizeof (rectype);
  memcpy (bp, &txn_num, sizeof (txn_num));
  bp += sizeof (txn_num);
  memcpy (bp, lsnp, sizeof (DB_LSN));
  bp += sizeof (DB_LSN);
  memcpy (bp, &opcode, sizeof (opcode));
  bp += sizeof (opcode);
  DB_ASSERT ((uint32_t) (bp - (uint8_t *) logrec.data) == logrec.size);
  ret = CDB_log_put (dbenv, ret_lsnp, (DBT *) & logrec, flags);
  if (txnid != NULL)
    txnid->last_lsn = *ret_lsnp;
  CDB___os_free (logrec.data, logrec.size);
  return (ret);
}

int
CDB___txn_regop_print (notused1, dbtp, lsnp, notused2, notused3)
     DB_ENV *notused1;
     DBT *dbtp;
     DB_LSN *lsnp;
     int notused2;
     void *notused3;
{
  __txn_regop_args *argp;
  uint32_t i;
  u_int ch;
  int ret;

  i = 0;
  ch = 0;
  notused1 = NULL;
  notused2 = 0;
  notused3 = NULL;

  if ((ret = CDB___txn_regop_read (dbtp->data, &argp)) != 0)
    return (ret);
  printf ("[%lu][%lu]txn_regop: rec: %lu txnid %lx prevlsn [%lu][%lu]\n",
          (u_long) lsnp->file,
          (u_long) lsnp->offset,
          (u_long) argp->type,
          (u_long) argp->txnid->txnid,
          (u_long) argp->prev_lsn.file, (u_long) argp->prev_lsn.offset);
  printf ("\topcode: %lu\n", (u_long) argp->opcode);
  printf ("\n");
  CDB___os_free (argp, 0);
  return (0);
}

int
CDB___txn_regop_read (recbuf, argpp)
     void *recbuf;
     __txn_regop_args **argpp;
{
  __txn_regop_args *argp;
  uint8_t *bp;
  int ret;

  ret = CDB___os_malloc (sizeof (__txn_regop_args) +
                         sizeof (DB_TXN), NULL, &argp);
  if (ret != 0)
    return (ret);
  argp->txnid = (DB_TXN *) & argp[1];
  bp = recbuf;
  memcpy (&argp->type, bp, sizeof (argp->type));
  bp += sizeof (argp->type);
  memcpy (&argp->txnid->txnid, bp, sizeof (argp->txnid->txnid));
  bp += sizeof (argp->txnid->txnid);
  memcpy (&argp->prev_lsn, bp, sizeof (DB_LSN));
  bp += sizeof (DB_LSN);
  memcpy (&argp->opcode, bp, sizeof (argp->opcode));
  bp += sizeof (argp->opcode);
  *argpp = argp;
  return (0);
}

int
CDB___txn_ckp_log (dbenv, txnid, ret_lsnp, flags, ckp_lsn, last_ckp)
     DB_ENV *dbenv;
     DB_TXN *txnid;
     DB_LSN *ret_lsnp;
     uint32_t flags;
     DB_LSN *ckp_lsn;
     DB_LSN *last_ckp;
{
  DBT logrec;
  DB_LSN *lsnp, null_lsn;
  uint32_t rectype, txn_num;
  int ret;
  uint8_t *bp;

  if (txnid != NULL &&
      TAILQ_FIRST (&txnid->kids) != NULL && CDB___txn_activekids (txnid) != 0)
    return (EPERM);
  rectype = DB_txn_ckp;
  txn_num = txnid == NULL ? 0 : txnid->txnid;
  if (txnid == NULL)
  {
    ZERO_LSN (null_lsn);
    lsnp = &null_lsn;
  }
  else
    lsnp = &txnid->last_lsn;
  logrec.size = sizeof (rectype) + sizeof (txn_num) + sizeof (DB_LSN)
    + sizeof (*ckp_lsn) + sizeof (*last_ckp);
  if ((ret = CDB___os_malloc (logrec.size, NULL, &logrec.data)) != 0)
    return (ret);

  bp = logrec.data;
  memcpy (bp, &rectype, sizeof (rectype));
  bp += sizeof (rectype);
  memcpy (bp, &txn_num, sizeof (txn_num));
  bp += sizeof (txn_num);
  memcpy (bp, lsnp, sizeof (DB_LSN));
  bp += sizeof (DB_LSN);
  if (ckp_lsn != NULL)
    memcpy (bp, ckp_lsn, sizeof (*ckp_lsn));
  else
    memset (bp, 0, sizeof (*ckp_lsn));
  bp += sizeof (*ckp_lsn);
  if (last_ckp != NULL)
    memcpy (bp, last_ckp, sizeof (*last_ckp));
  else
    memset (bp, 0, sizeof (*last_ckp));
  bp += sizeof (*last_ckp);
  DB_ASSERT ((uint32_t) (bp - (uint8_t *) logrec.data) == logrec.size);
  ret = CDB_log_put (dbenv, ret_lsnp, (DBT *) & logrec, flags);
  if (txnid != NULL)
    txnid->last_lsn = *ret_lsnp;
  CDB___os_free (logrec.data, logrec.size);
  return (ret);
}

int
CDB___txn_ckp_print (notused1, dbtp, lsnp, notused2, notused3)
     DB_ENV *notused1;
     DBT *dbtp;
     DB_LSN *lsnp;
     int notused2;
     void *notused3;
{
  __txn_ckp_args *argp;
  uint32_t i;
  u_int ch;
  int ret;

  i = 0;
  ch = 0;
  notused1 = NULL;
  notused2 = 0;
  notused3 = NULL;

  if ((ret = CDB___txn_ckp_read (dbtp->data, &argp)) != 0)
    return (ret);
  printf ("[%lu][%lu]txn_ckp: rec: %lu txnid %lx prevlsn [%lu][%lu]\n",
          (u_long) lsnp->file,
          (u_long) lsnp->offset,
          (u_long) argp->type,
          (u_long) argp->txnid->txnid,
          (u_long) argp->prev_lsn.file, (u_long) argp->prev_lsn.offset);
  printf ("\tckp_lsn: [%lu][%lu]\n",
          (u_long) argp->ckp_lsn.file, (u_long) argp->ckp_lsn.offset);
  printf ("\tlast_ckp: [%lu][%lu]\n",
          (u_long) argp->last_ckp.file, (u_long) argp->last_ckp.offset);
  printf ("\n");
  CDB___os_free (argp, 0);
  return (0);
}

int
CDB___txn_ckp_read (recbuf, argpp)
     void *recbuf;
     __txn_ckp_args **argpp;
{
  __txn_ckp_args *argp;
  uint8_t *bp;
  int ret;

  ret = CDB___os_malloc (sizeof (__txn_ckp_args) +
                         sizeof (DB_TXN), NULL, &argp);
  if (ret != 0)
    return (ret);
  argp->txnid = (DB_TXN *) & argp[1];
  bp = recbuf;
  memcpy (&argp->type, bp, sizeof (argp->type));
  bp += sizeof (argp->type);
  memcpy (&argp->txnid->txnid, bp, sizeof (argp->txnid->txnid));
  bp += sizeof (argp->txnid->txnid);
  memcpy (&argp->prev_lsn, bp, sizeof (DB_LSN));
  bp += sizeof (DB_LSN);
  memcpy (&argp->ckp_lsn, bp, sizeof (argp->ckp_lsn));
  bp += sizeof (argp->ckp_lsn);
  memcpy (&argp->last_ckp, bp, sizeof (argp->last_ckp));
  bp += sizeof (argp->last_ckp);
  *argpp = argp;
  return (0);
}

int
CDB___txn_xa_regop_log (dbenv, txnid, ret_lsnp, flags,
                        opcode, xid, formatID, gtrid, bqual)
     DB_ENV *dbenv;
     DB_TXN *txnid;
     DB_LSN *ret_lsnp;
     uint32_t flags;
     uint32_t opcode;
     const DBT *xid;
     int32_t formatID;
     uint32_t gtrid;
     uint32_t bqual;
{
  DBT logrec;
  DB_LSN *lsnp, null_lsn;
  uint32_t zero;
  uint32_t rectype, txn_num;
  int ret;
  uint8_t *bp;

  if (txnid != NULL &&
      TAILQ_FIRST (&txnid->kids) != NULL && CDB___txn_activekids (txnid) != 0)
    return (EPERM);
  rectype = DB_txn_xa_regop;
  txn_num = txnid == NULL ? 0 : txnid->txnid;
  if (txnid == NULL)
  {
    ZERO_LSN (null_lsn);
    lsnp = &null_lsn;
  }
  else
    lsnp = &txnid->last_lsn;
  logrec.size = sizeof (rectype) + sizeof (txn_num) + sizeof (DB_LSN)
    + sizeof (opcode)
    + sizeof (uint32_t) + (xid == NULL ? 0 : xid->size)
    + sizeof (formatID) + sizeof (gtrid) + sizeof (bqual);
  if ((ret = CDB___os_malloc (logrec.size, NULL, &logrec.data)) != 0)
    return (ret);

  bp = logrec.data;
  memcpy (bp, &rectype, sizeof (rectype));
  bp += sizeof (rectype);
  memcpy (bp, &txn_num, sizeof (txn_num));
  bp += sizeof (txn_num);
  memcpy (bp, lsnp, sizeof (DB_LSN));
  bp += sizeof (DB_LSN);
  memcpy (bp, &opcode, sizeof (opcode));
  bp += sizeof (opcode);
  if (xid == NULL)
  {
    zero = 0;
    memcpy (bp, &zero, sizeof (uint32_t));
    bp += sizeof (uint32_t);
  }
  else
  {
    memcpy (bp, &xid->size, sizeof (xid->size));
    bp += sizeof (xid->size);
    memcpy (bp, xid->data, xid->size);
    bp += xid->size;
  }
  memcpy (bp, &formatID, sizeof (formatID));
  bp += sizeof (formatID);
  memcpy (bp, &gtrid, sizeof (gtrid));
  bp += sizeof (gtrid);
  memcpy (bp, &bqual, sizeof (bqual));
  bp += sizeof (bqual);
  DB_ASSERT ((uint32_t) (bp - (uint8_t *) logrec.data) == logrec.size);
  ret = CDB_log_put (dbenv, ret_lsnp, (DBT *) & logrec, flags);
  if (txnid != NULL)
    txnid->last_lsn = *ret_lsnp;
  CDB___os_free (logrec.data, logrec.size);
  return (ret);
}

int
CDB___txn_xa_regop_print (notused1, dbtp, lsnp, notused2, notused3)
     DB_ENV *notused1;
     DBT *dbtp;
     DB_LSN *lsnp;
     int notused2;
     void *notused3;
{
  __txn_xa_regop_args *argp;
  uint32_t i;
  u_int ch;
  int ret;

  i = 0;
  ch = 0;
  notused1 = NULL;
  notused2 = 0;
  notused3 = NULL;

  if ((ret = CDB___txn_xa_regop_read (dbtp->data, &argp)) != 0)
    return (ret);
  printf ("[%lu][%lu]txn_xa_regop: rec: %lu txnid %lx prevlsn [%lu][%lu]\n",
          (u_long) lsnp->file,
          (u_long) lsnp->offset,
          (u_long) argp->type,
          (u_long) argp->txnid->txnid,
          (u_long) argp->prev_lsn.file, (u_long) argp->prev_lsn.offset);
  printf ("\topcode: %lu\n", (u_long) argp->opcode);
  printf ("\txid: ");
  for (i = 0; i < argp->xid.size; i++)
  {
    ch = ((uint8_t *) argp->xid.data)[i];
    if (isprint (ch) || ch == 0xa)
      putchar (ch);
    else
      printf ("%#x ", ch);
  }
  printf ("\n");
  printf ("\tformatID: %ld\n", (long) argp->formatID);
  printf ("\tgtrid: %u\n", argp->gtrid);
  printf ("\tbqual: %u\n", argp->bqual);
  printf ("\n");
  CDB___os_free (argp, 0);
  return (0);
}

int
CDB___txn_xa_regop_read (recbuf, argpp)
     void *recbuf;
     __txn_xa_regop_args **argpp;
{
  __txn_xa_regop_args *argp;
  uint8_t *bp;
  int ret;

  ret = CDB___os_malloc (sizeof (__txn_xa_regop_args) +
                         sizeof (DB_TXN), NULL, &argp);
  if (ret != 0)
    return (ret);
  argp->txnid = (DB_TXN *) & argp[1];
  bp = recbuf;
  memcpy (&argp->type, bp, sizeof (argp->type));
  bp += sizeof (argp->type);
  memcpy (&argp->txnid->txnid, bp, sizeof (argp->txnid->txnid));
  bp += sizeof (argp->txnid->txnid);
  memcpy (&argp->prev_lsn, bp, sizeof (DB_LSN));
  bp += sizeof (DB_LSN);
  memcpy (&argp->opcode, bp, sizeof (argp->opcode));
  bp += sizeof (argp->opcode);
  memset (&argp->xid, 0, sizeof (argp->xid));
  memcpy (&argp->xid.size, bp, sizeof (uint32_t));
  bp += sizeof (uint32_t);
  argp->xid.data = bp;
  bp += argp->xid.size;
  memcpy (&argp->formatID, bp, sizeof (argp->formatID));
  bp += sizeof (argp->formatID);
  memcpy (&argp->gtrid, bp, sizeof (argp->gtrid));
  bp += sizeof (argp->gtrid);
  memcpy (&argp->bqual, bp, sizeof (argp->bqual));
  bp += sizeof (argp->bqual);
  *argpp = argp;
  return (0);
}

int
CDB___txn_child_log (dbenv, txnid, ret_lsnp, flags, opcode, parent)
     DB_ENV *dbenv;
     DB_TXN *txnid;
     DB_LSN *ret_lsnp;
     uint32_t flags;
     uint32_t opcode;
     uint32_t parent;
{
  DBT logrec;
  DB_LSN *lsnp, null_lsn;
  uint32_t rectype, txn_num;
  int ret;
  uint8_t *bp;

  if (txnid != NULL &&
      TAILQ_FIRST (&txnid->kids) != NULL && CDB___txn_activekids (txnid) != 0)
    return (EPERM);
  rectype = DB_txn_child;
  txn_num = txnid == NULL ? 0 : txnid->txnid;
  if (txnid == NULL)
  {
    ZERO_LSN (null_lsn);
    lsnp = &null_lsn;
  }
  else
    lsnp = &txnid->last_lsn;
  logrec.size = sizeof (rectype) + sizeof (txn_num) + sizeof (DB_LSN)
    + sizeof (opcode) + sizeof (parent);
  if ((ret = CDB___os_malloc (logrec.size, NULL, &logrec.data)) != 0)
    return (ret);

  bp = logrec.data;
  memcpy (bp, &rectype, sizeof (rectype));
  bp += sizeof (rectype);
  memcpy (bp, &txn_num, sizeof (txn_num));
  bp += sizeof (txn_num);
  memcpy (bp, lsnp, sizeof (DB_LSN));
  bp += sizeof (DB_LSN);
  memcpy (bp, &opcode, sizeof (opcode));
  bp += sizeof (opcode);
  memcpy (bp, &parent, sizeof (parent));
  bp += sizeof (parent);
  DB_ASSERT ((uint32_t) (bp - (uint8_t *) logrec.data) == logrec.size);
  ret = CDB_log_put (dbenv, ret_lsnp, (DBT *) & logrec, flags);
  if (txnid != NULL)
    txnid->last_lsn = *ret_lsnp;
  CDB___os_free (logrec.data, logrec.size);
  return (ret);
}

int
CDB___txn_child_print (notused1, dbtp, lsnp, notused2, notused3)
     DB_ENV *notused1;
     DBT *dbtp;
     DB_LSN *lsnp;
     int notused2;
     void *notused3;
{
  __txn_child_args *argp;
  uint32_t i;
  u_int ch;
  int ret;

  i = 0;
  ch = 0;
  notused1 = NULL;
  notused2 = 0;
  notused3 = NULL;

  if ((ret = CDB___txn_child_read (dbtp->data, &argp)) != 0)
    return (ret);
  printf ("[%lu][%lu]txn_child: rec: %lu txnid %lx prevlsn [%lu][%lu]\n",
          (u_long) lsnp->file,
          (u_long) lsnp->offset,
          (u_long) argp->type,
          (u_long) argp->txnid->txnid,
          (u_long) argp->prev_lsn.file, (u_long) argp->prev_lsn.offset);
  printf ("\topcode: %lu\n", (u_long) argp->opcode);
  printf ("\tparent: 0x%lx\n", (u_long) argp->parent);
  printf ("\n");
  CDB___os_free (argp, 0);
  return (0);
}

int
CDB___txn_child_read (recbuf, argpp)
     void *recbuf;
     __txn_child_args **argpp;
{
  __txn_child_args *argp;
  uint8_t *bp;
  int ret;

  ret = CDB___os_malloc (sizeof (__txn_child_args) +
                         sizeof (DB_TXN), NULL, &argp);
  if (ret != 0)
    return (ret);
  argp->txnid = (DB_TXN *) & argp[1];
  bp = recbuf;
  memcpy (&argp->type, bp, sizeof (argp->type));
  bp += sizeof (argp->type);
  memcpy (&argp->txnid->txnid, bp, sizeof (argp->txnid->txnid));
  bp += sizeof (argp->txnid->txnid);
  memcpy (&argp->prev_lsn, bp, sizeof (DB_LSN));
  bp += sizeof (DB_LSN);
  memcpy (&argp->opcode, bp, sizeof (argp->opcode));
  bp += sizeof (argp->opcode);
  memcpy (&argp->parent, bp, sizeof (argp->parent));
  bp += sizeof (argp->parent);
  *argpp = argp;
  return (0);
}

int
CDB___txn_init_print (dbenv)
     DB_ENV *dbenv;
{
  int ret;

  if ((ret = CDB___db_add_recovery (dbenv,
                                    CDB___txn_regop_print,
                                    DB_txn_regop)) != 0)
    return (ret);
  if ((ret = CDB___db_add_recovery (dbenv,
                                    CDB___txn_ckp_print, DB_txn_ckp)) != 0)
    return (ret);
  if ((ret = CDB___db_add_recovery (dbenv,
                                    CDB___txn_xa_regop_print,
                                    DB_txn_xa_regop)) != 0)
    return (ret);
  if ((ret = CDB___db_add_recovery (dbenv,
                                    CDB___txn_child_print,
                                    DB_txn_child)) != 0)
    return (ret);
  return (0);
}

/*
 * PUBLIC: int CDB___txn_init_recover __P((DB_ENV *));
 */
int
CDB___txn_init_recover (dbenv)
     DB_ENV *dbenv;
{
  int ret;

  if ((ret = CDB___db_add_recovery (dbenv,
                                    CDB___txn_regop_recover,
                                    DB_txn_regop)) != 0)
    return (ret);
  if ((ret = CDB___db_add_recovery (dbenv,
                                    CDB___txn_ckp_recover, DB_txn_ckp)) != 0)
    return (ret);
  if ((ret = CDB___db_add_recovery (dbenv,
                                    CDB___txn_xa_regop_recover,
                                    DB_txn_xa_regop)) != 0)
    return (ret);
  if ((ret = CDB___db_add_recovery (dbenv,
                                    CDB___txn_child_recover,
                                    DB_txn_child)) != 0)
    return (ret);
  return (0);
}
