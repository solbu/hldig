//
// WordDB.cc
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordDB.cc,v 1.1 1999/10/01 16:45:51 loic Exp $
//

#include "WordDB.h"

const char* dberror(int errval) {
#define DB_MAX_ERROR	(-DB_TXN_CKP + 1)
  static const char* dbstr[DB_MAX_ERROR] = {
    "",
    "DB_INCOMPLETE",
    "DB_KEYEMPTY",
    "DB_KEYEXISTS",
    "DB_LOCK_DEADLOCK",
    "DB_LOCK_NOTGRANTED",
    "DB_LOCK_NOTHELD",
    "DB_NOTFOUND",
    "DB_RUNRECOVERY",
    "DB_DELETED",
    "DB_NEEDSPLIT",
    "DB_SWAPBYTES",
    "DB_TXN_CKP",
  };
  if(errval < 0 && -errval < DB_MAX_ERROR)
    return dbstr[-errval];
  else
    return strerror(errval);
}
