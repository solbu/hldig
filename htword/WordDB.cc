//
// WordDB.cc
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordDB.cc,v 1.2.2.5 2000/09/14 03:13:27 ghutchis Exp $
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <fcntl.h>
#include <stdlib.h>

extern "C" {
#include "db_int.h"
#include "db_page.h"
#include "db_shash.h"
#include "lock.h"
#include "mp.h"
}

#include "myqsort.h"
#include "WordDB.h"
#include "WordDBCache.h"

class WordContext;

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

int WordDB::Alloc() {
  if(db == 0) {
    db = 0;
    is_open = 0;
    return CDB_db_create(&db, db_info.dbenv, 0);
  } else {
    return 0;
  }
}

int WordDB::Open(const String& filename, const String& subname, DBTYPE type, int flags, int mode, int tags) {
  int error;
  if(is_open) {
    if((error = Close()) != 0)
      return error;
  }

  if((error = Alloc()) != 0) return error;

  if(!db_info.dbenv) {
    const char* progname = "WordDB";

    //
    // Environment initialization
    //
    // Output errors to the application's log.
    //
    db->set_errfile(db, stderr);
    db->set_errpfx(db, progname);
  }

  Tags(tags);

  error = db->open(db,
		   (const char*)filename,
		   (subname.empty() ? (const char*)0 : (const char*)subname),
		   type, (u_int32_t)flags, mode);

  //
  // When sharing files among processes, it may not be possible to flush
  // all the buffers because another process is locking them. This is not
  // fatal since the other process will take care of it.
  //
  if(error == DB_INCOMPLETE)
    error = 0;

  if(error == 0)
    is_open = 1;
  else
    fprintf(stderr, "WordDB::Open(%s,%s,%d,%d,%d) failed %s\n", (const char*)filename, (const char*)subname, type, flags, mode, CDB_db_strerror(error));


  return error;
}

int WordDB::Remove(const String& filename, const String& subname) {
  int error = 0;
  if((error = Close()) != 0) return error;
  if((error = Alloc()) != 0) return error;

  DB* tmp = db;
  db = 0;

  return tmp->remove(tmp, (const char*)filename, (const char*)subname, 0);
}

int WordDB::Close() {
  int error = 0;
  if((error = CacheOff()) != 0) return error;
  is_open = 0;
  if(db) error = db->close(db, 0);
  db = 0;
  return error;
}

int WordDB::Fd(int *fdp) {
  if(!is_open) return DB_UNKNOWN;
  return db->fd(db, fdp);
}

int WordDB::Stat(void *sp, void *(*db_malloc)(size_t), int flags) {
  if(!is_open) return DB_UNKNOWN;
  return db->stat(db, sp, db_malloc, (u_int32_t) flags);
}
  
int WordDB::Sync(int flags) {
  if(!is_open) return DB_UNKNOWN;
  return db->sync(db, (u_int32_t) flags);
}

int WordDB::get_byteswapped() const {
  if(!is_open) return DB_UNKNOWN;
  return db->get_byteswapped(db);
}

DBTYPE WordDB::get_type() const {
  if(!is_open) return DB_UNKNOWN;
  return db->get_type(db);
}

unsigned int WordDB::Size() const
{
  return (unsigned int)db->mpf->mfp->last_pgno;
}

//
// String arguments
//
int WordDB::Put(DB_TXN *txn, const String& key, const String& data, int flags) {
  if(!is_open) return DB_UNKNOWN;
  WORD_DBT_INIT(rkey, (void*)key.get(), key.length());
  WORD_DBT_INIT(rdata, (void*)data.get(), data.length());

  if(CacheP()) {
    int ret;
    //
    // If Put is not default (is NOOVERWRITE, for instance),
    // flush cache. 
    //
    if(flags != 0) {
      if((ret = CacheFlush()) != 0)
	return ret;
      return db->put(db, txn, &rkey, &rdata, flags);
    } else {
      if((ret = cache->Allocate(rkey.size + rdata.size)) == ENOMEM) {
	//	fprintf(stderr, "No mem ! No mem !\n");
	if((ret = CacheFlush()) != 0) return ret;
	if((ret = cache->Allocate(rkey.size + rdata.size))) return ret;
      }
      return cache->Add((char*)rkey.data, rkey.size, (char*)rdata.data, rdata.size);
    }
  } else {
    return db->put(db, txn, &rkey, &rdata, flags);
  }

  //
  // Should never reach this point
  // 
  return DB_RUNRECOVERY;
}

int WordDB::Put(DB_TXN *txn, const String& key, const unsigned int& data, int flags) {
  if(!is_open) return DB_UNKNOWN;
  WORD_DBT_INIT(rkey, (void*)key.get(), key.length());
  WORD_DBT_DCL(rdata);
  rdata.data = (void*)&data;
  rdata.size = sizeof(unsigned int);

  return db->put(db, txn, &rkey, &rdata, flags);
}

int WordDB::Get(DB_TXN *txn, String& key, String& data, int flags) const {
  if(!is_open) return DB_UNKNOWN;
  WORD_DBT_INIT(rkey, (void*)key.get(), (u_int32_t)key.length());
  WORD_DBT_INIT(rdata, (void*)data.get(), (u_int32_t)data.length());

  int error;

  if((error = ((WordDB*)this)->CacheFlush()) != 0) return error;

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

int WordDB::Get(DB_TXN *txn, String& key, unsigned int& data, int flags) const {
  if(!is_open) return DB_UNKNOWN;
  WORD_DBT_INIT(rkey, (void*)key.get(), (u_int32_t)key.length());
  WORD_DBT_DCL(rdata);

  int error;

  if((error = ((WordDB*)this)->CacheFlush()) != 0) return error;

  if((error = db->get(db, txn, &rkey, &rdata, 0)) != 0) {
    if(error != DB_NOTFOUND)
      fprintf(stderr, "WordDB::Get(%s,%s) using %d failed %s\n", (char*)key, (char*)data, flags, CDB_db_strerror(error));
  } else {
    //
    // Only set arguments if found something.
    //
    key.set((const char*)rkey.data, (int)rkey.size);
    if(rdata.size == sizeof(unsigned int)) 
      memcpy((char*)&data, (char*)rdata.data, sizeof(unsigned int));
    else
      data = 0;
  }

  return error;
}

int WordDB::Del(DB_TXN *txn, const String& key) {
  if(!is_open) return DB_UNKNOWN;
  WORD_DBT_INIT(rkey, (void*)key.get(), (u_int32_t)key.length());

  int error;

  if((error = CacheFlush()) != 0) return error;

  return db->del(db, txn, &rkey, 0);
}

//
// WordReference argument
//
int WordDB::Put(const WordReference& wordRef, int flags) {
  if(!is_open) return DB_UNKNOWN;

  int ret;
  
  String key;
  String record;

  if((ret = wordRef.Pack(key, record)) != OK) return DB_RUNRECOVERY;

  return Put(0, key, record, flags);
}

int WordDB::Del(const WordReference& wordRef) {
  if(!is_open) return DB_UNKNOWN;

  String key;

  wordRef.Key().Pack(key);

  return Del(0, key);
}

//
// Search entry matching wkey exactly, return key and data
// in wordRef.
//
int WordDB::Get(WordReference& wordRef) const {
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
int WordDB::Exists(const WordReference& wordRef) const {
  if(!is_open) return DB_UNKNOWN;

  String key;
  String data;

  if(wordRef.Key().Pack(key) != OK) return DB_RUNRECOVERY;

  return Get(0, key, data, 0);
}

//
// Accessors
//
int WordDB::set_bt_compare(int (*compare)(const DBT *, const DBT *), void *nuser_data) {
  int ret;
  if((ret = Alloc()) != 0) return ret;
  user_data = nuser_data;
  return db->set_bt_compare(db, compare);
}

int WordDB::set_pagesize(u_int32_t pagesize) {
  if(pagesize <= 0) return 0;
  int ret;
  if((ret = Alloc()) != 0) return ret;
  return db->set_pagesize(db, pagesize);
}

//
// Cursor
//
WordDBCursor* WordDB::Cursor() {
  if(!is_open) return 0;

  WordDBCursor *cursor = new WordDBCursor(this);
  return cursor->cursor ? cursor : 0;
}

//
// Cache management
//
int WordDB::CacheOn(WordContext* context, int size_hint)
{
  if(!CacheP()) {
    cache = new WordDBCache(context);
    cache->SetMax(size_hint);
  }

  return 0;
}

int WordDB::CacheOff()
{
  if(CacheP()) {
    int ret;
    if((ret = CacheFlush()) != 0) return ret;

    delete cache;
    cache = 0;
  }

  return 0;
}

int WordDB::CacheFlush()
{
  if(CacheP() && !cache->Empty()) {
    //    fprintf(stderr, "Flush ! Flush !\n");
    cache->Sort();
    WordDBCacheEntry* entries;
    int entries_length;
    int ret;
    if((ret = cache->Entries(entries, entries_length)) != 0)
      return ret;
    int i;
    WORD_DBT_DCL(key);
    WORD_DBT_DCL(record);
    for(i = 0; i < entries_length; i++) {
      WORD_DBT_SET(key, entries[i].key, entries[i].key_size);
      WORD_DBT_SET(record, entries[i].data, entries[i].data_size);
      if((ret = db->put(db, 0, &key, &record, 0)) != 0)
	return ret;
    }
    cache->Flush();
    //    fprintf(stderr, "Flushed ! Flushed !\n");
  } 

  return 0;
}

int WordDB::CacheCompare(int (*compare)(WordContext *, const WordDBCacheEntry *, const WordDBCacheEntry *)) {
  if(!CacheP()) {
    fprintf(stderr, "WordDB::CacheCompare: cannot set comparison function for cache on because the cache is not active\n");
    return DB_RUNRECOVERY;
  }
  return cache->SetCompare(compare);
}
