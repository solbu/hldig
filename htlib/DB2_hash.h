//
// DB2_hash.h
//
// $Id: DB2_hash.h,v 1.1 1999/03/03 04:23:15 ghutchis Exp $
//
//
#ifndef _DB2_hash_h_
#define _DB2_hash_h_

#include <Database.h>
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
    virtual void	Start_Seq(char *str);
    virtual char	*Get_Next_Seq();
	
private:
    int			isOpen;
    DB			*dbp;		// database
    DBC			*dbcp;		// cursor
    DBT			skey;
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


