//
// Database.cc
//
// Implementation of Database
//
// $Log: Database.cc,v $
// Revision 1.4  1999/01/23 01:25:02  hp
// Fixed _some_ missing const qualifiers on common methods (requiring temps)
//
// Revision 1.3  1998/06/21 23:20:06  turtle
// patches by Esa and Jesse to add BerkeleyDB and Prefix searching
//
// Revision 1.2  1998/05/26 03:58:07  turtle
// Got rid of compiler warnings.
//
// Revision 1.1.1.1  1997/02/03 17:11:04  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: Database.cc,v 1.4 1999/01/23 01:25:02 hp Exp $";
#endif

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
Database::getDatabaseInstance()
{
    return DB2_db::getDatabaseInstance();
}



