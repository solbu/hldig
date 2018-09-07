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
// $Id: Database.cc,v 1.12 2004/05/28 13:15:20 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "hlconfig.h"
#endif /* HAVE_CONFIG_H */

#include "Database.h"
#include "DB2_db.h"

//*****************************************************************************
// Database::Database()
//
Database::Database ()
{
}


//*****************************************************************************
// Database::~Database()
//
Database::~Database ()
{
}


//*****************************************************************************
// Database *Database::getDatabaseInstance()
//
Database *
Database::getDatabaseInstance (DBTYPE type = DB_BTREE)
{
  Database *db = DB2_db::getDatabaseInstance (type);

  db->db_type = type;

  return db;
}
