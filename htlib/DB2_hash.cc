//
// DB2_hash.cc
//
// DB2_hash: Implements the Berkeley Hash database as a Database object
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: DB2_hash.cc,v 1.5 1999/09/11 05:03:51 ghutchis Exp $
//

#include "DB2_hash.h"

#include <errno.h>
#include <stdlib.h>
#include <fstream.h>
#include <malloc.h>
#include <unistd.h>

// Default cache size in kilobytes.
// Maybe this should be an config option, just for easy testing and
//   determination for best system performance
// NOTE: page size is 1KB - do not change!!
#define CACHE_SIZE_IN_KB 64

//*****************************************************************************
// DB2_hash::DB2_hash()
//
DB2_hash::DB2_hash()
{
    isOpen = 0;
}


//*****************************************************************************
// DB2_hash::~DB2_hash()
//
DB2_hash::~DB2_hash()
{
    if (isOpen)
    {
	Close();
    }
}


//*****************************************************************************
// int DB_db::OpenReadWrite(char *filename, int mode)
//
int
DB2_hash::OpenReadWrite(char *filename, int mode)
{
    //
    // Initialize the database environment.
    //
    dbenv = db_init((char *)NULL);
    memset(&dbinfo, 0, sizeof(dbinfo));
//    dbinfo.db_cachesize = CACHE_SIZE_IN_KB * 1024;	// Cachesize: 64K.
//    dbinfo.db_pagesize = 1024;      			// Page size: 1K.

    //
    // Create the database.
    //
    if (access(filename, F_OK) == 0)
      errno = db_open(filename, DB_HASH, 0, 0, dbenv, &dbinfo, &dbp);
    else
      errno = db_open(filename, DB_HASH, DB_CREATE, mode, dbenv, &dbinfo, &dbp);
    if (errno == 0)
    {
        //
	// Acquire a cursor for the database.
	//
        if ((seqrc = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0)
	{
            seqerr = seqrc;
	    isOpen = 0;
            Close();
	    return NOTOK;
        }
	isOpen = 1;
	return OK;
    }
    else
    {
	return NOTOK;
    }
}


//*****************************************************************************
// int DB2_hash::OpenRead(char *filename)
//
int
DB2_hash::OpenRead(char *filename)
{
    //
    // Initialize the database environment.
    //
    dbenv = db_init((char *)NULL);
    memset(&dbinfo, 0, sizeof(dbinfo));
//    dbinfo.db_cachesize = CACHE_SIZE_IN_KB * 1024;	// Cachesize: 64K.
//    dbinfo.db_pagesize = 1024;			// Page size: 1K.

    //
    // Open the database.
    //
    if ((errno = db_open(filename, DB_HASH, DB_RDONLY, 0, dbenv,
			 &dbinfo, &dbp)) == 0)
    {
        //
	// Acquire a cursor for the database.
	//
        if ((seqrc = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0)
	{
            seqerr = seqrc;
	    isOpen = 0;
            Close();
	    return NOTOK;
        }
	isOpen = 1;
	return OK;
    }
    else
    {
	return NOTOK;
    }
}


//*****************************************************************************
// int DB2_hash::Close()
//
int
DB2_hash::Close()
{
    if (isOpen)
    {
	//
	// Close cursor, database and clean up environment
	//
        (void)(dbcp->c_close)(dbcp);
	(void)(dbp->close)(dbp, 0);
	(void) db_appexit(dbenv);
    }
    isOpen = 0;
    return OK;
}


//*****************************************************************************
// void DB2_hash::Start_Get()
//
void
DB2_hash::Start_Get()
{
    //
    // skey and nextkey are just dummies
    //
    memset(&data, 0, sizeof(DBT));
    memset(&skey, 0, sizeof(DBT));

//    skey.data = "";
//    skey.size = 0;
//    skey.flags = 0;
    if (isOpen && dbp)
    {
	//
	// Set the cursor to the first position.
	//
        seqrc = dbcp->c_get(dbcp, &skey, &data, DB_FIRST);
	seqerr = seqrc;
    }
}


//*****************************************************************************
// char *DB2_hash::Get_Next()
//
char *
DB2_hash::Get_Next()
{
    //
    // Looks like get Get_Next() and Get_Next_Seq() are pretty much the same...
    //

    if (isOpen && !seqrc)
    {
	lkey = 0;
	lkey.append((char *)skey.data, skey.size);
	// DON'T forget to set the flags to 0!
	skey.flags = 0;
        seqrc = dbcp->c_get(dbcp, &skey, &data, DB_NEXT);
	seqerr = seqrc;
	return lkey.get();
    }
    else
	return 0;
}

//*****************************************************************************
// char *DB2_hash::Get_Next(String &item)
//
char *
DB2_hash::Get_Next(String &item)
{
    if (isOpen && !seqrc)
    {
	lkey = 0;
	lkey.append((char *)skey.data, skey.size);

	item = 0;
	item.append((char *)data.data, data.size);

	// DON'T forget to set the flags to 0!
	skey.flags = 0;
        seqrc = dbcp->c_get(dbcp, &skey, &data, DB_NEXT);
	seqerr = seqrc;

	return lkey.get();
    }
    else
	return 0;
}

//*****************************************************************************
// void DB2_hash::Start_Seq()
//
void
DB2_hash::Start_Seq(char *str)
{

    memset(&skey, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));

    skey.data = str;
    skey.size = strlen(str);
    if (isOpen && dbp)
    {
	//
	// Okay, get the first key. Use DB_SET_RANGE for finding partial
	// keys also. If you set it to DB_SET, and the words book, books
	// and bookstore do exists, it will find them if you specify
	// book*. However if you specify boo* if will not find
	// anything. Setting to DB_SET_RANGE will still find the `first'
	// word after boo* (which is book).
	//
        seqrc = dbcp->c_get(dbcp, &skey, &data, DB_SET_RANGE);
	seqerr = seqrc;
    }
}


//*****************************************************************************
// char *DB2_hash::Get_Next_Seq()
//
char *
DB2_hash::Get_Next_Seq()
{

    if (isOpen && !seqrc)
    {
	lkey = 0;
	lkey.append((char *)skey.data, skey.size);
	skey.flags = 0;
        seqrc = dbcp->c_get(dbcp, &skey, &data, DB_NEXT);
	seqerr = seqrc;
	return lkey.get();
    }
    else
	return 0;
}



//*****************************************************************************
// int DB2_hash::Put(const String &key, const String &data)
//
int
DB2_hash::Put(const String &key, const String &data)
{
    DBT	k, d;

    memset(&k, 0, sizeof(DBT));
    memset(&d, 0, sizeof(DBT));

    if (!isOpen)
	return NOTOK;

    k.data = key.get();
    k.size = key.length();

    d.data = data.get();
    d.size = data.length();

    //
    // A 0 in the flags in put means replace, if you didn't specify DB_DUP
    // somewhere else...
    //
    return (dbp->put)(dbp, NULL, &k, &d, 0) == 0 ? OK : NOTOK;
}


//*****************************************************************************
// int DB2_hash::Get(const String &key, String &data)
//
int
DB2_hash::Get(const String &key, String &data)
{
    DBT	k, d;

    memset(&k, 0, sizeof(DBT));
    memset(&d, 0, sizeof(DBT));

    k.data = key.get();
    k.size = key.length();

    int rc = dbp->get(dbp, NULL, &k, &d, 0);
    if (rc)
	return NOTOK;

    data = 0;
    data.append((char *)d.data, d.size);
    return OK;
}


//*****************************************************************************
// int DB2_hash::Exists(const String &key)
//
int
DB2_hash::Exists(const String &key)
{
    String data;

    if (!isOpen)
	return 0;

    return Get(key, data);
}


//*****************************************************************************
// int DB2_hash::Delete(const String &key)
//
int
DB2_hash::Delete(const String &key)
{
    DBT	k;

    memset(&k, 0, sizeof(DBT));

    if (!isOpen)
	return 0;

    k.data = key.get();
    k.size = key.length();

    return (dbp->del)(dbp, NULL, &k, 0);
}


//*****************************************************************************
// DB2_hash *DB2_hash::getDatabaseInstance()
//
DB2_hash *
DB2_hash::getDatabaseInstance()
{
    return new DB2_hash();
}


//******************************************************************************

/*
 * db_init --
 *      Initialize the environment. Only returns a pointer
 */
DB_ENV *
DB2_hash::db_init(char *home)
{
    DB_ENV *dbenv;
    char *progname = "DB2 problem...";

    //
    // Rely on calloc to initialize the structure.
    //
    if ((dbenv = (DB_ENV *)calloc(sizeof(DB_ENV), 1)) == NULL)
    {
	fprintf(stderr, "%s: %s\n", progname, strerror(ENOMEM));
	exit (1);
    }
    dbenv->db_errfile = stderr;
    dbenv->db_errpfx = progname;

    if ((errno = db_appinit(home, NULL, dbenv, DB_CREATE)) != 0)
    {
	fprintf(stderr, "%s: db_appinit: %s\n", progname, strerror(errno));
	exit (1);
    }
    return (dbenv);
}

