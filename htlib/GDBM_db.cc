//
// GDBM_db.cc
//
// Implementation of GDBM_db
//
// $Log: GDBM_db.cc,v $
// Revision 1.2  1999/01/23 01:25:02  hp
// Fixed _some_ missing const qualifiers on common methods (requiring temps)
//
// Revision 1.1.1.1  1997/02/03 17:11:05  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: GDBM_db.cc,v 1.2 1999/01/23 01:25:02 hp Exp $";
#endif

#include "GDBM_db.h"
#include <stdlib.h>


//*****************************************************************************
// GDBM_db::GDBM_db()
//
GDBM_db::GDBM_db()
{
    isOpen = 0;
}


//*****************************************************************************
// GDBM_db::~GDBM_db()
//
GDBM_db::~GDBM_db()
{
    if (isOpen)
    {
	Close();
    }
}


//*****************************************************************************
// int GDBM_db::OpenReadWrite(char *filename, int mode)
//
int
GDBM_db::OpenReadWrite(char *filename, int mode)
{
    dbf = gdbm_open(filename, 1024, GDBM_WRCREAT | GDBM_FAST, mode, 0);
    if (dbf)
    {
	isOpen = 1;
	return OK;
    }
    else
    {
	return NOTOK;
    }
}


//*****************************************************************************
// int GDBM_db::OpenRead(char *filename)
//
int
GDBM_db::OpenRead(char *filename)
{
    dbf = gdbm_open(filename, 1024, GDBM_READER, 0, 0);
    if (dbf)
    {
	isOpen = 1;
	return OK;
    }
    else
    {
	return NOTOK;
    }
}


//*****************************************************************************
// int GDBM_db::Close()
//
int
GDBM_db::Close()
{
    if (isOpen)
    {
	gdbm_close(dbf);
    }
    isOpen = 0;
    return OK;
}


//*****************************************************************************
// void GDBM_db::Start_Get()
//
void
GDBM_db::Start_Get()
{
    if (isOpen && dbf)
	skey = gdbm_firstkey(dbf);
}


//*****************************************************************************
// char *GDBM_db::Get_Next()
//
char *
GDBM_db::Get_Next()
{
    datum	nextkey;
	
    if (isOpen && skey.dptr)
    {
	lkey = 0;
	lkey.append(skey.dptr, skey.dsize);
	nextkey = gdbm_nextkey(dbf, skey);
	free(skey.dptr);
	skey = nextkey;
	return lkey.get();
    }
    else
	return 0;
}


//*****************************************************************************
// int GDBM_db::Put(const String &key, const String &data)
//
int
GDBM_db::Put(const String &key, const String &data)
{
    datum	k, d;

    if (!isOpen)
	return NOTOK;

    k.dptr = key.get();
    k.dsize = key.length();

    d.dptr = data.get();
    d.dsize = data.length();
	
    return gdbm_store(dbf, k, d, GDBM_REPLACE) == 0 ? OK : NOTOK;
}


//*****************************************************************************
// int GDBM_db::Get(const String &key, String &data)
//
int
GDBM_db::Get(const String &key, String &data)
{
    datum	k, d;

    if (!isOpen)
	return NOTOK;

    k.dptr = key.get();
    k.dsize = key.length();

    d = gdbm_fetch(dbf, k);
    if (!d.dptr)
	return NOTOK;

    data = 0;
    data.append(d.dptr, d.dsize);
    free(d.dptr);
    return OK;
}


//*****************************************************************************
// int GDBM_db::Exists(const String &key)
//
int
GDBM_db::Exists(const String &key)
{
    datum	k;

    if (!isOpen)
	return 0;

    k.dptr = key.get();
    k.dsize = key.length();

    return gdbm_exists(dbf, k);
}


//*****************************************************************************
// int GDBM_db::Delete(const String &key)
//
int
GDBM_db::Delete(const String &key)
{
    datum	k;

    if (!isOpen)
	return 0;

    k.dptr = key.get();
    k.dsize = key.length();

    return gdbm_delete(dbf, k);
}


//*****************************************************************************
// GDBM_db *GDBM_db::getDatabaseInstance()
//
GDBM_db *
GDBM_db::getDatabaseInstance()
{
    return new GDBM_db();
}


