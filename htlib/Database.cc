//
// Database.cc
//
// Database: Class which defines the interface to a generic, 
//           simple database.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Database.cc,v 1.6 1999/09/11 05:03:51 ghutchis Exp $
//

#include "Database.h"
#include "DB2_db.h"
#include "DB2_hash.h"

//*****************************************************************************
// Database::Database()
//
Database::Database()
{
}


//*****************************************************************************
// Database::~Database()
//
Database::~Database()
{
}


//*****************************************************************************
// int Database::Put(char *key, const String &data)
//
int
Database::Put(char *key, const String &data)
{
    String		k(key);

    return Put(k, data);
}


//*****************************************************************************
// int Database::Put(char *key, char *data, int size)
//
int
Database::Put(char *key, char *data, int size)
{
    String		k(key);
    String		d;

    d.append(data, size);
    return Put(k, d);
}


//*****************************************************************************
// int Database::Get(char *key, String &data)
//
int
Database::Get(char *key, String &data)
{
    String		k(key);

    return Get(k, data);
}


//*****************************************************************************
// int Database::Exists(char *key)
//
int
Database::Exists(char *key)
{
    String		k(key);

    return Exists(k);
}


//*****************************************************************************
// int Database::Delete(char *key)
//
int
Database::Delete(char *key)
{
    String		k(key);

    return Delete(k);
}


//*****************************************************************************
// Database *Database::getDatabaseInstance()
//
Database *
Database::getDatabaseInstance(int type = DB_BTREE)
{
  if (type == DB_HASH) // Hash
    return DB2_hash::getDatabaseInstance();
  else
    return DB2_db::getDatabaseInstance();
}



