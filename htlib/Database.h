//
// Database.h
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
// $Id: Database.h,v 1.15 2004/05/28 13:15:20 lha Exp $
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
    static Database	*getDatabaseInstance(DBTYPE type);
	
    //
    // Common interface
    //
    virtual int		OpenReadWrite(const char *filename, int mode = 0666) = 0;
    virtual int		OpenRead(const char *filename) = 0;
    void		SetCompare(int (*func)(const DBT *a, const DBT *b)) { _compare = func; }
    void		SetPrefix(size_t (*func)(const DBT *a, const DBT *b)) { _prefix = func; }
    virtual int		Close() = 0;
    virtual int		Put(const String &key, const String &data) = 0;
    virtual int		Get(const String &key, String &data) = 0;
    virtual int		Exists(const String &key) = 0;
    virtual int		Delete(const String &key) = 0;

    virtual void	Start_Get() = 0;
    virtual char	*Get_Next() { String item; String key; return Get_Next(item, key); }
    virtual char	*Get_Next(String &item) { String key; return Get_Next(item, key); }
    virtual char	*Get_Next(String &item, String &key) = 0;
    virtual void	Start_Seq(const String& str) = 0;
    virtual char	*Get_Next_Seq() { return Get_Next(); }

protected:
    int			isOpen;
    DB			*dbp;		// database
    DBC			*dbcp;		// cursor

    String		skey;		// Next key to search for iterator
    String		data;		// Next data to return for iterator
    String		lkey;		// Contains the last key returned by iterator

    DB_ENV		*dbenv;		// database enviroment
    int			(*_compare)(const DBT *a, const DBT *b); // Key comparison
    size_t		(*_prefix)(const DBT *a, const DBT *b);  // Key reduction

    int			seqrc;
    int			seqerr;
    DBTYPE		db_type;
};

#endif
