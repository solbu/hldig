//
// DB2_hash.h
//
// DB2_hash: Implements the Berkeley Hash database as a Database object
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: DB2_hash.h,v 1.5 1999/09/11 05:03:51 ghutchis Exp $
//

#ifndef _DB2_hash_h_
#define _DB2_hash_h_

#include "Database.h"

#include <db.h>
#include <fcntl.h>

class DB2_hash : public Database
{
    //
    // Construction/Destruction
    //
protected:
    DB2_hash();
public:
    ~DB2_hash();

    static DB2_hash	*getDatabaseInstance();
	
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


