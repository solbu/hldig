//
// DB2_db.h
//
// DB2_db: Implements the Berkeley B-Tree database as a Database object
//        (including duplicate values to allow duplicate word entries)
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999, 2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: DB2_db.h,v 1.8.2.1 2000/05/10 18:23:43 loic Exp $
//

#ifndef _DB2_db_h_
#define _DB2_db_h_

#include "Database.h"
#include <db.h>
#include <fcntl.h>

class DB2_db : public Database
{
    //
    // Construction/Destruction
    //
protected:
    DB2_db();
public:
    ~DB2_db();

    static DB2_db	*getDatabaseInstance(DBTYPE type);
	
    virtual int		OpenReadWrite(const char *filename, int mode) { return Open(filename, DB_CREATE, mode); }
    virtual int		OpenRead(const char *filename) { return Open(filename, DB_RDONLY, 0666); }
    virtual int		Close();
    virtual int		Get(const String &, String &);
    virtual int		Put(const String &, const String &);
    virtual int		Exists(const String &);
    virtual int		Delete(const String &);
	
    virtual void	Start_Get();
    virtual char	*Get_Next(String &item, String &key);
    virtual void	Start_Seq(const String& key);
	
private:
    DB_ENV		*db_init(char *);

    int			Open(const char *filename, int flags, int mode);
};

#endif
