//
// Database.h
//
// Database: Class which defines the interface to a generic, 
//           simple database.
//
// $Id: Database.h,v 1.9 1999/09/08 14:42:29 loic Exp $
//
//
//
#ifndef _Database_h_
#define _Database_h_

#include "Object.h"
#include "htString.h"
#include <db.h>

// Database Types
// defined in db.h
// #define DB_BTREE 1
// #define DB_HASH 2
#ifndef GDBM_HASH
#define GDBM_HASH 2
#endif


class Database : public Object
{
    //
    // Make sure no one can actually create an object of this type or
    // the derived types.  The static getDatabaseInstance() method needs to be
    // used.
    //
protected:
    Database();
public:
    ~Database();

    //
    // Since the contructor is protected, the only way to actually get
    // a Database object is through the following class method.
    // The idea here is that the particular type of database used by
    // all the programs is to be defined in one place.
    //
    static Database	*getDatabaseInstance(int type);
	
    //
    // Common interface
    //
    virtual int		OpenReadWrite(char *filename, int mode = 0644) = 0;
    virtual int		OpenRead(char *filename) = 0;
    virtual int		Close() = 0;
    int			Put(char *key, const String &data);
    int			Put(char *key, char *data, int size);
    virtual int		Put(const String &key, const String &data) = 0;
    int			Get(char *key, String &data);
    virtual int		Get(const String &key, String &data) = 0;
    virtual int		Exists(const String &key) = 0;
    int			Exists(char *key);
    virtual int		Delete(const String &key) = 0;
    int			Delete(char *key);

    virtual void	Start_Get() = 0;
    virtual char	*Get_Next() = 0;
    virtual char	*Get_Next(String &data) = 0;
    virtual void	Start_Seq(char *str) = 0;
    virtual char	*Get_Next_Seq() = 0;
};

#endif


