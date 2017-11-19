//
// DB2_db.cc
//
// DB2_db: Implements the Berkeley B-Tree database as a Database object
//        (including duplicate values to allow duplicate word entries)
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: DB2_db.cc,v 1.26 2004/05/28 13:15:20 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>

#ifdef HAVE_STD
#include <fstream>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif
#else
#include <fstream.h>
#endif /* HAVE_STD */

#ifndef _MSC_VER /* _WIN32 */
#include <unistd.h>
#endif

#include "DB2_db.h"
#include "HtConfiguration.h"

// Default cache size in kilobytes.
// Maybe this should be an config option, just for easy testing and
//   determination for best system performance
// NOTE: page size is 1KB - do not change!!
#define CACHE_SIZE_IN_KB 64

//*****************************************************************************
// DB2_db::DB2_db()
//
DB2_db::DB2_db()
{
    isOpen = 0;
    _compare = 0;
    _prefix = 0;
}


//*****************************************************************************
// DB2_db::~DB2_db()
//
DB2_db::~DB2_db()
{
  Close();
}


//*****************************************************************************
//
int
DB2_db::Open(const char *filename, int flags, int mode)
{
  //
  // Initialize the database environment.
  //
  if((dbenv = db_init((char *)NULL)) == 0) return NOTOK;

  if(CDB_db_create(&dbp, dbenv, 0) != 0) return NOTOK;

  if(_compare) dbp->set_bt_compare(dbp, _compare);
  if(_prefix) dbp->set_bt_prefix(dbp, _prefix);

    //
    // Open the database.
    //
    if((errno = dbp->open(dbp, filename, NULL, db_type, flags, mode)) == 0)
    {
        //
  // Acquire a cursor for the database.
  //
        if ((seqrc = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0)
  {
            seqerr = seqrc;
            Close();
      return NOTOK;
        }
  isOpen = 1;
  return OK;
    }
    else
    {
  return NOTOK;
    }
}


//*****************************************************************************
// int DB2_db::Close()
//
int
DB2_db::Close()
{
    if(isOpen)
    {
  //
  // Close cursor, database and clean up environment
  //
        (void)(dbcp->c_close)(dbcp);
  (void)(dbp->close)(dbp, 0);
  (void)(dbenv->close(dbenv, 0));
  dbenv = 0;
    }
    isOpen = 0;
    return OK;
}


//*****************************************************************************
// char *DB2_db::Get_Next(String &item, String &key)
//
char *
DB2_db::Get_Next(String &item, String &key)
{
  if (isOpen && !seqrc)
    {
      //
      // Return values
      //
      key = skey;
      lkey = skey;
      item = data;

      //
      // Search for the next record
      //
      DBT local_key;
      DBT local_data;

      memset(&local_key, 0, sizeof(DBT));
      memset(&local_data, 0, sizeof(DBT));

      local_key.data = skey.get();
      local_key.size = skey.length();

      seqrc = dbcp->c_get(dbcp, &local_key, &local_data, DB_NEXT);
      seqerr = seqrc;

      if(!seqrc) {
  data = 0;
  data.append((char*)local_data.data, (int)local_data.size);
  skey = 0;
  skey.append((char*)local_key.data, (int)local_key.size);
      }

      return lkey.get();
    }
  else
    return 0;
}

//*****************************************************************************
// void DB2_db::Start_Seq()
//
void
DB2_db::Start_Seq(const String& key)
{
    DBT local_key;
    DBT local_data;

    memset(&local_key, 0, sizeof(DBT));
    memset(&local_data, 0, sizeof(DBT));

    skey = key;

    local_key.data = skey.get();
    local_key.size = skey.length();

    if (isOpen && dbp)
    {
  //
  // Okay, get the first key. Use DB_SET_RANGE for finding partial
  // keys also. If you set it to DB_SET, and the words book, books
  // and bookstore do exists, it will find them if you specify
  // book*. However if you specify boo* if will not find
  // anything. Setting to DB_SET_RANGE will still find the `first'
  // word after boo* (which is book).
  //
        seqrc = dbcp->c_get(dbcp, &local_key, &local_data, DB_SET_RANGE);
  seqerr = seqrc;

  if(!seqrc) {
    data = 0;
    data.append((char*)local_data.data, (int)local_data.size);
    skey = 0;
    skey.append((char*)local_key.data, (int)local_key.size);
  }
    }
}

//*****************************************************************************
// void DB2_db::Start_Get()
//
void
DB2_db::Start_Get()
{
    DBT local_key;
    DBT local_data;

    memset(&local_key, 0, sizeof(DBT));
    memset(&local_data, 0, sizeof(DBT));

    if (isOpen && dbp)
    {
  //
  // Okay, get the first key. Use DB_SET_RANGE for finding partial
  // keys also. If you set it to DB_SET, and the words book, books
  // and bookstore do exists, it will find them if you specify
  // book*. However if you specify boo* if will not find
  // anything. Setting to DB_SET_RANGE will still find the `first'
  // word after boo* (which is book).
  //
        seqrc = dbcp->c_get(dbcp, &local_key, &local_data, DB_FIRST);
  seqerr = seqrc;

  if(!seqrc) {
    data = 0;
    data.append((char*)local_data.data, (int)local_data.size);
    skey = 0;
    skey.append((char*)local_key.data, (int)local_key.size);
  }
    }
}

//*****************************************************************************
// int DB2_db::Put(const String &key, const String &data)
//
int
DB2_db::Put(const String &key, const String &data)
{
    DBT  k, d;

    memset(&k, 0, sizeof(DBT));
    memset(&d, 0, sizeof(DBT));

    if (!isOpen)
  return NOTOK;

    k.data = (char*)key.get();
    k.size = key.length();

    d.data = (char*)data.get();
    d.size = data.length();

    //
    // A 0 in the flags in put means replace, if you didn't specify DB_DUP
    // somewhere else...
    //
    return (dbp->put)(dbp, NULL, &k, &d, 0) == 0 ? OK : NOTOK;
}


//*****************************************************************************
// int DB2_db::Get(const String &key, String &data)
//
int
DB2_db::Get(const String &key, String &data)
{
    DBT  k, d;

    memset(&k, 0, sizeof(DBT));
    memset(&d, 0, sizeof(DBT));

    //
    // k arg of get should be const but is not. Harmless cast.
    //
    k.data = (char*)key.get();
    k.size = key.length();

    int rc = dbp->get(dbp, NULL, &k, &d, 0);
    if (rc)
  return NOTOK;

    data = 0;
    data.append((char *)d.data, d.size);
    return OK;
}


//*****************************************************************************
// int DB2_db::Exists(const String &key)
//
int
DB2_db::Exists(const String &key)
{
    String data;

    if (!isOpen)
  return 0;

    return Get(key, data);
}


//*****************************************************************************
// int DB2_db::Delete(const String &key)
//
int
DB2_db::Delete(const String &key)
{
    DBT  k;

    memset(&k, 0, sizeof(DBT));

    if (!isOpen)
  return 0;

    k.data = (char*)key.get();
    k.size = key.length();

    return (dbp->del)(dbp, NULL, &k, 0);
}


//*****************************************************************************
// DB2_db *DB2_db::getDatabaseInstance()
//
DB2_db *
DB2_db::getDatabaseInstance(DBTYPE)
{
    return new DB2_db();
}

//*****************************************************************************
// void Error(const char *error_prefix, char *message);
//
void Error(const char *error_prefix, char *message)
{
  // We don't do anything here, it's mostly a stub so we can set a breakpoint
  // for debugging purposes
  fprintf(stderr, "%s: %s\n", error_prefix, message);
}

//******************************************************************************

/*
 * db_init --
 *      Initialize the environment. Only returns a pointer
 */
DB_ENV *
DB2_db::db_init(char *home)
{
    DB_ENV *dbenv;
    const char *progname = "DB2 problem...";

    int error;
    if((error = CDB_db_env_create(&dbenv, 0)) != 0) {
      fprintf(stderr, "DB2_db: CDB_db_env_create %s\n", CDB_db_strerror(error));
      return 0;
    }

    dbenv->set_errpfx(dbenv, progname);
    dbenv->set_errcall(dbenv, &Error);

    if((error = dbenv->open(dbenv, (const char*)home, NULL, DB_CREATE | DB_PRIVATE | DB_INIT_LOCK | DB_INIT_MPOOL, 0666)) != 0) {
      dbenv->err(dbenv, error, "open %s", (home ? home : ""));
      return 0;
    }

    return (dbenv);
}

