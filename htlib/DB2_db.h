//
// DB2_db.h
//
// $Id: DB2_db.h,v 1.2 1999/01/23 01:25:02 hp Exp $
//
// $Log: DB2_db.h,v $
// Revision 1.2  1999/01/23 01:25:02  hp
// Fixed _some_ missing const qualifiers on common methods (requiring temps)
//
// Revision 1.1  1998/06/21 23:20:06  turtle
// patches by Esa and Jesse to add BerkeleyDB and Prefix searching
//
// Revision 1.1.1.1  1997/02/03 17:11:05  turtle
// Initial CVS
//
//
#ifndef _DB2_db_h_
#define _DB2_db_h_

#include <Database.h>
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


