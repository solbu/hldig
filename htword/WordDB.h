//
// WordDB.h
//
// WordDB: Interface to Berkeley DB
//         uses String and WordReference instead of Dbt, add some convenience
//	   methods and implements string translation of Berkeley DB error codes.
//         It does not include the 'join' feature.
//         Beside this, the interface it identical to the Db class.
//         The next evolution for this set of class is to have a single object per
//         application so that they all share the same environment (transactions,
//         shared pool, database directory). This implies a static common object
//         that is refered by each actual instance of WordDB. The static object
//         holds the DbEnv and DbInfo, the instances of WordDB only have an open
//         descriptor using the same DbEnv and DbInfo.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordDB.h,v 1.3.2.12 2000/05/06 20:46:42 loic Exp $
//

#ifndef _WordDB_h_
#define _WordDB_h_

#include <stdio.h>
#include <errno.h>

#include "db.h"
#include "WordReference.h"
#include "WordDBInfo.h"
#include "htString.h"

#define WORD_DBT_DCL(v) \
    DBT v; \
    memset((char*)&(v), '\0', sizeof(DBT))

#define WORD_DBT_SET(v,d,s) \
    v.data = (d); \
    v.size = (s)

#define WORD_DBT_INIT(v,d,s) \
    WORD_DBT_DCL(v); \
    WORD_DBT_SET(v,d,s)

//
// Encapsulate the Berkeley DB DB type
//
// Implements the same methods with String instead of Dbt.
//
// Add convenience methods taking WordReference instead of String
//
// The error model is *not* to use exceptions. 
//
// To get a cursor use the Open method of WordDBCursor. I find this
// more convinient than getting a cursor from WordDB.
//
// The WordDB has DbInfo and DbEnv members that can be set before
// calling Open to configure it.
//
class WordDB {
 public:
  inline WordDB() { Alloc(); }
  inline ~WordDB() { Dealloc(); }

  inline int Alloc() {
    db = 0;
    is_open = 0;
    dbenv = WordDBInfo::Instance()->dbenv;
    return CDB_db_create(&db, dbenv, 0);
  }

  inline int Dealloc() {
    int error = 0;
    is_open = 0;
    if(db)
      error = db->close(db, 0);
    else
      fprintf(stderr, "WordDB::Dealloc: null db\n");
    dbenv = 0;
    db = 0;
    return error;
  }

  inline int Open(const String& filename, DBTYPE type, int flags, int mode) {
    if(is_open) {
      int error = 0;
      if((error = Close()) != 0)
	return error;
    }

    if(!dbenv) {
      const char* progname = "WordDB";

      //
      // Environment initialization
      //
      // Output errors to the application's log.
      //
      db->set_errfile(db, stderr);
      db->set_errpfx(db, progname);
    }

    int error = db->open(db, filename, NULL, type, (u_int32_t)flags, mode);

    if(error == 0)
      is_open = 1;

    return error;
  }

  inline int Close() {
    int error;
    if((error = Dealloc()) != 0)
      return error;
    return Alloc();
  }

  inline int Fd(int *fdp) {
    if(!is_open) return DB_UNKNOWN;
    return db->fd(db, fdp);
  }

  inline int Stat(void *sp, void *(*db_malloc)(size_t), int flags) {
    if(!is_open) return DB_UNKNOWN;
    return db->stat(db, sp, db_malloc, (u_int32_t) flags);
  }
  
  inline int Sync(int flags) {
    if(!is_open) return DB_UNKNOWN;
    return db->sync(db, (u_int32_t) flags);
  }

  inline int get_byteswapped() const {
    if(!is_open) return DB_UNKNOWN;
    return db->get_byteswapped(db);
  }

  inline DBTYPE get_type() const {
    if(!is_open) return DB_UNKNOWN;
    return db->get_type(db);
  }

  //
  // String arguments
  //
  inline int Put(DB_TXN *txn, const String& key, const String& data, int flags) {
    WORD_DBT_INIT(rkey, (void*)key.get(), key.length());
    WORD_DBT_INIT(rdata, (void*)data.get(), data.length());

    return db->put(db, txn, &rkey, &rdata, flags);
  }

  inline int Get(DB_TXN *txn, String& key, String& data, int flags) const {
    WORD_DBT_INIT(rkey, (void*)key.get(), (u_int32_t)key.length());
    WORD_DBT_INIT(rdata, (void*)data.get(), (u_int32_t)data.length());

    int error;
    if((error = db->get(db, txn, &rkey, &rdata, 0)) != 0) {
      if(error != DB_NOTFOUND)
	fprintf(stderr, "WordDB::Get(%s,%s) using %d failed %s\n", (char*)key, (char*)data, flags, CDB_db_strerror(error));
    } else {
      //
      // Only set arguments if found something.
      //
      key.set((const char*)rkey.data, (int)rkey.size);
      data.set((const char*)rdata.data, (int)rdata.size);
    }

    return error;
  }

  inline int Del(DB_TXN *txn, const String& key) {
    WORD_DBT_INIT(rkey, (void*)key.get(), (u_int32_t)key.length());

    return db->del(db, txn, &rkey, 0);
  }

  //
  // WordReference argument
  //
  inline int Put(const WordReference& wordRef, int flags) {
    if(!is_open) return DB_UNKNOWN;

    int ret;
    String key;
    String record;

    if((ret = wordRef.Pack(key, record)) != OK) return DB_RUNRECOVERY;

    return Put(0, key, record, flags);
  }

  inline int Del(const WordReference& wordRef) {
    String key;

    wordRef.Key().Pack(key);

    return Del(0, key);
  }

  //
  // Search entry matching wkey exactly, return key and data
  // in wordRef.
  //
  inline int Get(WordReference& wordRef) const {
    if(!is_open) return DB_UNKNOWN;

    String data;
    String key;

    if(wordRef.Key().Pack(key) != OK) return DB_RUNRECOVERY;

    int ret;
    if((ret = Get(0, key, data, 0)) != 0)
      return ret;

    return wordRef.Unpack(key, data) == OK ? 0 : DB_RUNRECOVERY;
  }

  //
  // Returns 0 of the key of wordRef matches an entry in the database.
  // Could be implemented with Get but is not because we don't
  // need to build a wordRef with the entry found in the base. 
  //
  inline int Exists(const WordReference& wordRef) const {
    if(!is_open) return DB_UNKNOWN;

    String key;
    String data;

    if(wordRef.Key().Pack(key) != OK) return DB_RUNRECOVERY;

    return Get(0, key, data, 0);
  }

  //
  // Accessors
  //
  inline int set_bt_compare(int (*compare)(const DBT *, const DBT *)) {
    return db->set_bt_compare(db, compare);
  }

  inline int set_pagesize(u_int32_t pagesize) {
    return db->set_pagesize(db, pagesize);
  }

  //
  // Accessors for description of the compression scheme
  //
  inline DB_CMPR_INFO* CmprInfo() { return dbenv->mp_cmpr_info; }
  inline void CmprInfo(DB_CMPR_INFO* info) { dbenv->mp_cmpr_info = info; }

  int			is_open;
  DB*			db;
  DB_ENV*            	dbenv;
};

//
// Interface to DBC that uses String instead of DBT
//
class WordDBCursor {
 public:
  inline WordDBCursor() { cursor = 0; }
  inline ~WordDBCursor() {
    Close();
  }

  inline int Open(DB* db) {
    Close();
    return db->cursor(db, 0, &cursor, 0);
  }

  inline int Close() {
    if(cursor) cursor->c_close(cursor);
    cursor = 0;
    return 0;
  }

  //
  // String arguments
  //
  inline int Get(String& key, String& data, int flags) {
    WORD_DBT_DCL(rkey);
    WORD_DBT_DCL(rdata);
    switch(flags & DB_OPFLAGS_MASK) {
    case DB_SET_RANGE:
    case DB_SET:
    case DB_GET_BOTH:
      WORD_DBT_SET(rkey, (void*)key.get(), key.length());
      break;
    }
    int error;
    if((error = cursor->c_get(cursor, &rkey, &rdata, (u_int32_t)flags)) != 0) {
      if(error != DB_NOTFOUND)
	fprintf(stderr, "WordDBCursor::Get(%d) failed %s\n", flags, CDB_db_strerror(error));
    } else {
      key.set((const char*)rkey.data, (int)rkey.size);
      data.set((const char*)rdata.data, (int)rdata.size);
    }
    return error;
  }

  inline int Put(const String& key, const String& data, int flags) {
    WORD_DBT_INIT(rkey, (void*)key.get(), (size_t)key.length());
    WORD_DBT_INIT(rdata, (void*)data.get(), (size_t)data.length());
    return cursor->c_put(cursor, &rkey, &rdata, (u_int32_t)flags);
  }

  inline int Del() {
    return cursor->c_del(cursor, (u_int32_t)0);
  }

private:
  DBC* cursor;
};

#endif /* _WordDB_h */
