//
// DB2_db.h
//
// $Id: DB2_db.h,v 1.4 1999/08/28 21:12:27 ghutchis Exp $
//
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


