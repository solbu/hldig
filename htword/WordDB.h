//
// WordDB.h
//
// WordDB: Interface to Berkeley DB verbose on errors, return OK or NOTOK,
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
// $Id: WordDB.h,v 1.3 1999/10/05 16:16:09 loic Exp $
//

#ifndef _WordDB_h_
#define _WordDB_h_

#include "db_cxx.h"
#include "WordReference.h"
#include "htString.h"

#include <iostream.h>
#include <errno.h>

extern const char* dberror(int errval);

//
// Encapsulate the Berkeley DB Db class
//
// Implements the same methods with String instead of Dbt.
//
// Add convinience methods taking WordReference instead of String
//
// Most method return OK or NOTOK and issue an error message on cerr
// if appropriate. 
//
// The error model is *not* to use exceptions. 
//
// To get a cursor use the Open method of WordCursor. I find this
// more convinient than getting a cursor from WordDB.
//
// The WordDB has DbInfo and DbEnv members that can be set before
// calling Open to configure it.
//
class WordDB {
 public:
  WordDB() { db = 0; }
  ~WordDB() { Close(); }

  int Open(const String& filename, DBTYPE type, int flags, int mode) {

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

    if ((errno = dbenv.appinit(0, 0, DB_CREATE)) != 0) {
      cerr << progname << ": DbEnv::appinit: " << dberror(errno) << "\n";
      return NOTOK;
    }

    if ((errno = Db::open(filename, type, (u_int32_t)flags, mode, &dbenv, &dbinfo, &db)) != 0) {
      cerr << progname << ": Db::open: " << dberror(errno) << "\n";
      return NOTOK;
    }

    return OK;
  }

  int Close() {
    if(db) db->close(0);
    db = 0;
    return OK;
  }

  int Fd(int *fdp) {
    if(!db) return NOTOK;

    if((errno = db->fd(fdp)) != 0) {
      cerr << "WordDB::Fd() failed " << dberror(errno) << "\n";
      return NOTOK;
    }
    return OK;
  }

  int Stat(void *sp, void *(*db_malloc)(size_t), int flags) {
    if(!db) return NOTOK;
    if((errno = db->stat(sp, db_malloc, (u_int32_t) flags)) != 0) {
      cerr << "WordDB::Stat(" << flags << ") failed " << dberror(errno) << "\n";
      return NOTOK;
    }
    return OK;
  }
  
  int Sync(int flags) {
    if(!db) return NOTOK;
    if((errno = db->sync((u_int32_t) flags)) != 0) {
      cerr << "WordDB::Sync(" << flags << ") failed " << dberror(errno) << "\n";
      return NOTOK;
    }
    return OK;
  }

  int get_byteswapped() const {
    if(!db) return -1;
    return db->get_byteswapped();
  }

  DBTYPE get_type() const {
    if(!db) return DB_UNKNOWN;
    return db->get_type();
  }

  //
  // String arguments
  //
  int Put(DbTxn *, const String& key, const String& data, int flags) {
    Dbt rkey((void*)key.get(), (size_t)key.length());
    Dbt rdata((void*)data.get(), (size_t)data.length());
    if((errno = db->put(0, &rkey, &rdata, flags)) != 0) {
      cerr << "WordDB::Put(" << key << ", " << data << ", " << flags << ") failed " << dberror(errno) << "\n";
    }
    return errno;
  }

  int Get(DbTxn *, String& key, String& data, int flags) const {
    Dbt rkey((void*)key.get(), (u_int32_t)key.length());
    Dbt rdata((void*)data.get(), (u_int32_t)data.length());

    if((errno = db->get(0, &rkey, &rdata, 0)) != 0) {
      if(errno != DB_NOTFOUND)
	cerr << "WordDB::Get(" << key << ", " << data << ", " << flags << ") failed " << dberror(errno) << "\n";
    } else {
      //
      // Only set arguments if found something.
      //
      key.set((const char*)rkey.get_data(), (int)rkey.get_size());
      data.set((const char*)rdata.get_data(), (int)rdata.get_size());
    }

    return errno;
  }

  int Del(DbTxn *, const String& key) {
    Dbt rkey((void*)key.get(), (u_int32_t)key.length());

    if((errno = db->del(0, &rkey, 0)) != 0) {
      if(errno != DB_NOTFOUND)
	cerr << "WordDB::Del(" << key << ") failed " << dberror(errno) << "\n";
    }
  
    return errno;
  }

  //
  // WordReference argument
  //
  int Put(const WordReference& wordRef, int flags) {
    if(!db) return NOTOK;

    int ret;
    String key;
    String record;

    if((ret = wordRef.Pack(key, record)) == OK) {
      ret = Put(0, key, record, flags) == 0 ? OK : NOTOK;
    }
    return ret;
  }

  int Del(const WordReference& wordRef) {
    String key;

    wordRef.Key().Pack(key);

    return Del(0, key);
  }

  //
  // Search entry matching wkey exactly, return key and data
  // in wordRef.
  //
  int Get(WordReference& wordRef) const {
    if(!db) return DB_RUNRECOVERY;

    String data;
    String key;

    if(wordRef.Key().Pack(key) != OK) return DB_RUNRECOVERY;

    int ret;
    if((ret = Get(0, key, data, 0)) != 0)
      return ret;

    return wordRef.Unpack(key, data) == OK ? 0 : DB_RUNRECOVERY;
  }

  //
  // Returns OK of the key of wordRef matches an entry in the base.
  // Could be implemented with Get but is not because we don't
  // need to build a wordRef with the entry found in the base. 
  //
  int Exists(const WordReference& wordRef) const {
    if(!db) return NOTOK;

    String key;
    String data;

    if(wordRef.Key().Pack(key) != OK) return NOTOK;

    if(Get(0, key, data, 0) != 0)
      return NOTOK;
  
    return OK;
  }

  Db*			db;
  DbEnv	            	dbenv;
  DbInfo            	dbinfo;
};

//
// Interface to Dbc that uses String instead of Dbt
// Methods report errors on cerr and return OK/NOTOK status.
//
class WordCursor {
 public:
  WordCursor() { cursor = 0; }
  ~WordCursor() {
    Close();
  }

  int Open(Db* db) {
    Close();
    if((errno = db->cursor(0, &cursor, 0)) != 0) {
      cerr << "WordCursor::Open failed " << dberror(errno) << "\n";
      return NOTOK;
    }
    return OK;
  }

  int Close() {
    if(cursor) cursor->close();
    cursor = 0;
    return OK;
  }

  //
  // String arguments
  //
  int Get(String& key, String& data, int flags) {
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
    if((errno = cursor->get(&rkey, &rdata, (u_int32_t)flags)) != 0) {
      if(errno != DB_NOTFOUND)
	cerr << "WordCursor::Get(" << flags << ") failed " << dberror(errno) << "\n";
      return errno;
    }
    key.set((const char*)rkey.get_data(), (int)rkey.get_size());
    data.set((const char*)rdata.get_data(), (int)rdata.get_size());
    return errno;
  }

  int Put(const String& key, const String& data, int flags) {
    Dbt rkey((void*)key.get(), (size_t)key.length());
    Dbt rdata((void*)data.get(), (size_t)data.length());
    if((errno = cursor->put(&rkey, &rdata, (u_int32_t)flags)) != 0) {
      cerr << "WordCursor::Put(" << key << ", " << data << ", " << flags << ") failed " << dberror(errno) << "\n";
      return NOTOK;
    }
    return OK;
  }

  int Del() {
    if((errno = cursor->del((u_int32_t)0)) != 0) {
      cerr << "WordCursor::Del() failed " << dberror(errno) << "\n";
      return NOTOK;
    }
    return OK;
  }
private:
    Dbc* cursor;
};

#endif
