//
// WordDB.h
//
// WordDB: Interface to Berkeley DB
//         uses String and WordReference instead of Dbt, add some convinience
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
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordDB.h,v 1.3.2.10 2000/02/10 21:10:35 loic Exp $
//

#ifndef _WordDB_h_
#define _WordDB_h_

#include "db_cxx.h"
#include "WordReference.h"
#include "htString.h"

#include <iostream.h>
#include <errno.h>

#include <iomanip.h>
#include <ctype.h>

extern const char* dberror(int errval);

//
// Encapsulate the Berkeley DB Db class
//
// Implements the same methods with String instead of Dbt.
//
// Add convinience methods taking WordReference instead of String
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
  inline WordDB() {
      db = 0; 
  }
  inline ~WordDB() { Close(); }

  inline int Open(const String& filename, DBTYPE type, int flags, int mode) {

    if(db) Close();

    const char* progname = "WordDB";

    //
    // Environment initialization
    //
    // Output errors to the application's log.
    //
    dbenv.set_error_stream(&cerr);
    dbenv.set_errpfx(progname);
    //
    // Do not trust C++ portability of exception handling. I may be
    // wrong about that but have no proof. 
    //
    dbenv.set_error_model(DbEnv::ErrorReturn);

    int error;
    if ((error = dbenv.appinit(0, 0, DB_CREATE)) != 0)
      return error;

    return Db::open(filename, type, (u_int32_t)flags, mode, &dbenv, &dbinfo, &db);
  }

  inline int Close() {
    int error = 0;
    if(db) error = db->close(0);
    db = 0;
    return error;
  }

  inline int Fd(int *fdp) {
    if(!db) return DB_UNKNOWN;
    return db->fd(fdp);
  }

  inline int Stat(void *sp, void *(*db_malloc)(size_t), int flags) {
    if(!db) return DB_UNKNOWN;
    return db->stat(sp, db_malloc, (u_int32_t) flags);
  }
  
  inline int Sync(int flags) {
    if(!db) return DB_UNKNOWN;
    return db->sync((u_int32_t) flags);
  }

  inline int get_byteswapped() const {
    if(!db) return DB_UNKNOWN;
    return db->get_byteswapped();
  }

  inline DBTYPE get_type() const {
    if(!db) return DB_UNKNOWN;
    return db->get_type();
  }

  //
  // String arguments
  //
  inline int Put(DbTxn *, const String& key, const String& data, int flags) {
    Dbt rkey((void*)key.get(), (size_t)key.length());
    Dbt rdata((void*)data.get(), (size_t)data.length());

    return db->put(0, &rkey, &rdata, flags);
  }

  inline int Get(DbTxn *, String& key, String& data, int flags) const {
    Dbt rkey((void*)key.get(), (u_int32_t)key.length());
    Dbt rdata((void*)data.get(), (u_int32_t)data.length());

    int error;
    if((error = db->get(0, &rkey, &rdata, 0)) != 0) {
      if(error != DB_NOTFOUND)
	cerr << "WordDB::Get(" << key << ", " << data << ", " << flags << ") failed " << dberror(error) << "\n";
    } else {
      //
      // Only set arguments if found something.
      //
      key.set((const char*)rkey.get_data(), (int)rkey.get_size());
      data.set((const char*)rdata.get_data(), (int)rdata.get_size());
    }

    return error;
  }

  inline int Del(DbTxn *, const String& key) {
    Dbt rkey((void*)key.get(), (u_int32_t)key.length());

    return db->del(0, &rkey, 0);
  }

  //
  // WordReference argument
  //
  inline int Put(const WordReference& wordRef, int flags) {
    if(!db) return DB_UNKNOWN;

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
    if(!db) return DB_UNKNOWN;

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
    if(!db) return DB_UNKNOWN;

    String key;
    String data;

    if(wordRef.Key().Pack(key) != OK) return DB_RUNRECOVERY;

    return Get(0, key, data, 0);
  }

  //
  // Accessors for description of the compression scheme
  //
  inline DB_CMPR_INFO* CmprInfo() { return dbenv.get_mp_cmpr_info(); }
  inline void CmprInfo(DB_CMPR_INFO* info) { dbenv.set_mp_cmpr_info(info); }

  Db*			db;
  DbEnv	            	dbenv;
  DbInfo            	dbinfo;
};

//
// Interface to Dbc that uses String instead of Dbt
//
class WordDBCursor {
 public:
  inline WordDBCursor() { cursor = 0; }
  inline ~WordDBCursor() {
    Close();
  }

  inline int Open(Db* db) {
    Close();
    return db->cursor(0, &cursor, 0);
  }

  inline int Close() {
    if(cursor) cursor->close();
    cursor = 0;
    return 0;
  }

  //
  // String arguments
  //
  inline int Get(String& key, String& data, int flags) {
    Dbt rkey;
    Dbt rdata;
    switch(flags & DB_OPFLAGS_MASK) {
    case DB_SET_RANGE:
    case DB_SET:
    case DB_GET_BOTH:
      rkey.set_data((void*)key.get());
      rkey.set_size((u_int32_t)key.length());
      break;
    }
    int error;
    if((error = cursor->get(&rkey, &rdata, (u_int32_t)flags)) != 0) {
      if(error != DB_NOTFOUND)
	cerr << "WordDBCursor::Get(" << flags << ") failed " << dberror(error) << "\n";
    } else {
      key.set((const char*)rkey.get_data(), (int)rkey.get_size());
      data.set((const char*)rdata.get_data(), (int)rdata.get_size());
    }
    return error;
  }

  inline int Put(const String& key, const String& data, int flags) {
    Dbt rkey((void*)key.get(), (size_t)key.length());
    Dbt rdata((void*)data.get(), (size_t)data.length());
    return cursor->put(&rkey, &rdata, (u_int32_t)flags);
  }

  inline int Del() {
    return cursor->del((u_int32_t)0);
  }

private:
  Dbc* cursor;
};

#endif /* _WordDB_h */
