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
// $Id: WordDB.h,v 1.3.2.14 2000/09/14 03:13:27 ghutchis Exp $
//

#ifndef _WordDB_h_
#define _WordDB_h_

#include <stdio.h>
#include <errno.h>

#include "db.h"
#include "WordReference.h"
#include "WordDBInfo.h"
#include "htString.h"

class WordDBCache;
class WordDBCacheEntry;

#define WORD_DBT_DCL(v) \
    DBT v; \
    memset((char*)&(v), '\0', sizeof(DBT)); \
    v.app_private = user_data

#define WORD_DBT_SET(v,d,s) \
    v.data = (d); \
    v.size = (s)

#define WORD_DBT_INIT(v,d,s) \
    WORD_DBT_DCL(v); \
    WORD_DBT_SET(v,d,s)

class WordDBCursor;
class WordDBCache;

#define WORD_DB_DICT	(1 << 4)
#define WORD_DB_INDEX	(2 << 4)
#define WORD_DB_DEAD	(3 << 4)
#define WORD_DB_FILES	(4 << 4)

//
// Encapsulate the DB type
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
  inline WordDB(WordDBInfo& ndb_info) :
    db_info(ndb_info)
    {
      db = 0;
      is_open = 0;
      cache = 0;
    }
  inline ~WordDB() {
    if(CacheP()) CacheOff();
    Close();
  }

  int Alloc();

  int Open(const String& filename, const String& subname, DBTYPE type, int flags, int mode, int tags);
  int Remove(const String& filename, const String& subname);
  int Close();

  int Fd(int *fdp);
  int Stat(void *sp, void *(*db_malloc)(size_t), int flags);
  int Sync(int flags);
  int get_byteswapped() const;
  DBTYPE get_type() const;
  unsigned int Size() const;
  void Tags(int tags) { db->tags = tags; }
  int Tags() const { return db->tags; }

  //
  // String arguments
  //
  int Put(DB_TXN *txn, const String& key, const String& data, int flags);
  int Put(DB_TXN *txn, const String& key, const unsigned int& data, int flags);
  int Get(DB_TXN *txn, String& key, String& data, int flags) const;
  int Get(DB_TXN *txn, String& key, unsigned int& data, int flags) const;
  int Del(DB_TXN *txn, const String& key);

  //
  // WordReference argument
  //
  int Put(const WordReference& wordRef, int flags);
  int Del(const WordReference& wordRef);

  //
  // Search entry matching wkey exactly, return key and data
  // in wordRef.
  //
  int Get(WordReference& wordRef) const;

  //
  // Returns 0 of the key of wordRef matches an entry in the database.
  // Could be implemented with Get but is not because we don't
  // need to build a wordRef with the entry found in the base. 
  //
  int Exists(const WordReference& wordRef) const;

  //
  // Accessors
  //
  int set_bt_compare(int (*compare)(const DBT *, const DBT *), void *user_data);
  int set_pagesize(u_int32_t pagesize);

  //
  // Cursor
  //
  WordDBCursor* Cursor();

  //
  // Cache management
  //
  int CacheOn(WordContext* context, int size_hint);
  int CacheOff();
  int CacheFlush();
  int CacheCompare(int (*compare)(WordContext *, const WordDBCacheEntry *, const WordDBCacheEntry *));
  int CacheP() const { return cache ? 1 : 0; }
  
  void*			user_data;
  int			is_open;
  DB*			db;
  WordDBInfo&          	db_info;
  WordDBCache*		cache;
};

//
// Interface to DBC that uses String instead of DBT
//
class WordDBCursor {
 public:
  inline WordDBCursor(WordDB* ndb) {
    db = ndb;
    user_data = db->user_data;
    cursor = 0;
    Open();
  }
  inline ~WordDBCursor() {
    Close();
  }

  inline int Open() {
    Close();
    return db->db->cursor(db->db, 0, &cursor, 0);
  }

  inline int Close() {
    if(cursor) cursor->c_close(cursor);
    cursor = 0;
    return 0;
  }

  inline int Get(String& key, unsigned int& data, int flags) {
    db->CacheFlush();
    
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
    data = 0;
    if((error = cursor->c_get(cursor, &rkey, &rdata, (u_int32_t)flags)) != 0) {
      if(error != DB_NOTFOUND)
	fprintf(stderr, "WordDBCursor::Get(%d) failed %s\n", flags, CDB_db_strerror(error));
    } else {
      key.set((const char*)rkey.data, (int)rkey.size);
      memcpy((char*)&data, (char*)rdata.data, sizeof(unsigned int));
    }
    return error;
  }
  
  //
  // String arguments
  //
  inline int Get(String& key, String& data, int flags) {
    db->CacheFlush();

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

  void* user_data;
  WordDB* db;
  DBC* cursor;
};

#endif /* _WordDB_h */
