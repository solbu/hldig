//
// DB2_db.h
//
// DB2_db: Implements the Berkeley B-Tree database as a Database object
//        (including duplicate values to allow duplicate word entries)
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: DB2_db.h,v 1.6 1999/09/11 05:03:51 ghutchis Exp $
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

    static DB2_db	*getDatabaseInstance();
	
    virtual int		OpenReadWrite(char *filename, int mode);
    virtual int		OpenRead(char *filename);
    virtual int		Close();
    virtual int		Delete(const String &);
	
    virtual void	Start_Get();
    virtual char	*Get_Next();
    virtual char	*Get_Next(String &item);
    virtual void	Start_Seq(char *str);
    virtual char	*Get_Next_Seq();
	
private:
    int			isOpen;
    DB			*dbp;		// database
    DBC			*dbcp;		// cursor
    DBT			skey;
    DBT			data;
    DB_ENV		*dbenv;		// database enviroment
    DB_INFO		dbinfo;

    String		lkey;
    int			seqrc;
    int			seqerr;

    DB_ENV		*db_init(char *);

    virtual int		Get(const String &, String &);
    virtual int		Put(const String &, const String &);
    virtual int		Exists(const String &);
};

#endif


