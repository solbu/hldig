//
// GDBM_db.h
//
// $Id: GDBM_db.h,v 1.2 1999/01/23 01:25:03 hp Exp $
//
// $Log: GDBM_db.h,v $
// Revision 1.2  1999/01/23 01:25:03  hp
// Fixed _some_ missing const qualifiers on common methods (requiring temps)
//
// Revision 1.1.1.1  1997/02/03 17:11:05  turtle
// Initial CVS
//
//
#ifndef _GDBM_db_h_
#define _GDBM_db_h_

#include <Database.h>
#include <gdbm.h>

class GDBM_db : public Database
{
    //
    // Construction/Destruction
    //
protected:
    GDBM_db();
public:
    ~GDBM_db();

    static GDBM_db	*getDatabaseInstance();
	
    virtual int		OpenReadWrite(char *filename, int mode);
    virtual int		OpenRead(char *filename);
    virtual int		Close();
    virtual int		Delete(const String &);
	
    virtual void	Start_Get();
    virtual char	*Get_Next();
	
private:
    int			isOpen;
    GDBM_FILE		dbf;
    datum		skey;
    String		lkey;

    virtual int		Get(const String &, String &);
    virtual int		Put(const String &, String &);
    virtual int		Exists(const String &);
};

#endif


