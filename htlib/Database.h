//
// DB.h
//
// This is a class which defines the interface to a generic, simple database.
//
// $Id: Database.h,v 1.2 1997/03/24 04:33:19 turtle Exp $
//
// $Log: Database.h,v $
// Revision 1.2  1997/03/24 04:33:19  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.1.1.1  1997/02/03 17:11:04  turtle
// Initial CVS
//
//
#ifndef _Database_h_
#define _Database_h_

#include "Object.h"
#include "htString.h"

class Database : public Object
{
    //
    // Make sure noone can actually create an object of this type or
    // the derived types.  The static netDatabase() method needs to be
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
    static Database	*getDatabaseInstance();
	
    //
    // Common interface
    //
    virtual int		OpenReadWrite(char *filename, int mode = 0644) = 0;
    virtual int		OpenRead(char *filename) = 0;
    virtual int		Close() = 0;
    int			Put(char *key, String &data);
    int			Put(char *key, char *data, int size);
    virtual int		Put(String &key, String &data) = 0;
    int			Get(char *key, String &data);
    virtual int		Get(String &key, String &data) = 0;
    virtual int		Exists(String &key) = 0;
    int			Exists(char *key);
    virtual int		Delete(String &key) = 0;
    int			Delete(char *key);

    virtual void	Start_Get() = 0;
    virtual char	*Get_Next() = 0;
};

#endif


