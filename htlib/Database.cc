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
// $Id: Database.cc,v 1.9.2.1 2000/05/06 20:46:39 loic Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

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



