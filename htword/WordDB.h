//
// WordDB.h
//
// WordDB: Interface to Berkeley DB verbose on errors, return OK or NOTOK,
//         uses String and WordReference instead of Dbt, add some convinience
//	   methods.
//         Beside these the goal is to offer an interface that allows exactly 
//	   the same things than the underlying Berkeley DB classes.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: WordDB.h,v 1.1 1999/10/01 16:45:51 loic Exp $
//

#ifndef _WordDB_h_
#define _WordDB_h_

#include "db_cxx.h"
#include "WordReference.h"
#include "htString.h"

#include <iostream.h>
#include <errno.h>

extern const char* dberror(int errval);

class WordDB {
 public:
  WordDB() { db = 0; }
  ~WordDB() { Close(); }

  int Open(const String& filename, int mode) {

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

    if ((errno = Db::open(filename, DB_BTREE, (u_int32_t)mode, 0666, &dbenv, &dbinfo, &db)) != 0) {
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

  //
  // String arguments
  //
  int Put(DbTxn *txnid, const String& key, const String& data, int flags) {
    Dbt rkey((void*)key.get(), (size_t)key.length());
    Dbt rdata((void*)data.get(), (size_t)data.length());
    if((errno = db->put(0, &rkey, &rdata, flags)) != 0) {
      cerr << "WordDB::Put(" << key << ", " << data << ", " << flags << ") failed " << dberror(errno) << "\n";
    }
    return errno;
  }

  int Get(DbTxn *txnid, String& key, String& data, int flags) {
    Dbt rkey((void*)key.get(), (u_int32_t)key.length());
    Dbt rdata((void*)data.get(), (u_int32_t)data.length());

    if((errno = db->get(0, &rkey, &rdata, 0)) != 0) {
      if(errno != DB_NOTFOUND)
	cerr << "WordDB::Get(" << key << ", " << data << ", " << flags << ") failed " << dberror(errno) << "\n";
    }
    return errno;
  }

  int Del(DbTxn *txnid, const String& key) {
    Dbt rkey((void*)key.get(), (u_int32_t)key.length());
    Dbt rdata;

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

    if((errno = Del(0, key)) != 0) {
      if(errno != DB_NOTFOUND)
	return -1;
      else
	return 0;
    }
  
    return 1;
  }
  
  int Exists(const WordReference& wordRef) {
    if(!db) return NOTOK;

    String key;
    String data;
    wordRef.Key().Pack(key);

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
      if(errno != DB_NOTFOUND) {
	cerr << "WordCursor::Get(" << flags << ") failed " << dberror(errno) << "\n";
      }
      return NOTOK;
    }
    key.set((const char*)rkey.get_data(), (int)rkey.get_size());
    data.set((const char*)rdata.get_data(), (int)rdata.get_size());
    return OK;
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
