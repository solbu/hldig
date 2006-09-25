//
// Database.cc
//
// Database: Class which defines the interface to a generic, 
//           simple database.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Database.cc,v 1.1.2.1 2006/09/25 23:50:30 aarnone Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */
#include "HtDebug.h"

#include "Database.h"
#include "DB2_db.h"

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
    HtDebug * debug = HtDebug::Instance();
    debug->outlog(10, "Database destructor start\n");

    debug->outlog(10, "Database destructor end\n");
}


//*****************************************************************************
// Database *Database::getDatabaseInstance()
//
Database *
Database::getDatabaseInstance(DBTYPE type = DB_BTREE)
{
  Database* db = DB2_db::getDatabaseInstance(type);

  db->db_type = type;

  return db;
}



