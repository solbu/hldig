//
// Database.cc
//
// Implementation of Database
//
// $Log: Database.cc,v $
// Revision 1.1.1.1  1997/02/03 17:11:04  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: Database.cc,v 1.1.1.1 1997/02/03 17:11:04 turtle Exp $";
#endif

#include "Database.h"
#include "GDBM_db.h"

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
// int Database::Put(char *key, String &data)
//
int
Database::Put(char *key, String &data)
{
    String		k = key;

    return Put(k, data);
}


//*****************************************************************************
// int Database::Put(char *key, char *data, int size)
//
int
Database::Put(char *key, char *data, int size)
{
    String		k = key;
    String		d = 0;

    d.append(data, size);
    return Put(k, d);
}


//*****************************************************************************
// int Database::Get(char *key, String &data)
//
int
Database::Get(char *key, String &data)
{
    String		k = key;

    return Get(k, data);
}


//*****************************************************************************
// int Database::Exists(char *key)
//
int
Database::Exists(char *key)
{
    String		k = key;

    return Exists(k);
}


//*****************************************************************************
// int Database::Delete(char *key)
//
int
Database::Delete(char *key)
{
    String		k = key;

    return Delete(k);
}


//*****************************************************************************
// Database *Database::getDatabaseInstance()
//
Database *
Database::getDatabaseInstance()
{
    return GDBM_db::getDatabaseInstance();
}



