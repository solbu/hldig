//
// IndexDB.cc
//
// IndexDB: Add a comment here...
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "IndexDB.h"
#include "IndexDBRef.h"
#include "Database.h"
#include "HtURLCodec.h"
#include "IntObject.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#ifndef _MSC_VER /* _WIN32 */
#include <unistd.h>
#endif

#ifdef HAVE_STD
#include <iostream>
#include <fstream>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif
#else
#include <iostream.h>
#include <fstream.h>
#endif /* HAVE_STD */

#include <errno.h>

//*****************************************************************************
// IndexDB::IndexDB()
//
IndexDB::IndexDB()
{
    isopen = 0;
    i_dbf = 0;
}


//*****************************************************************************
// IndexDB::~IndexDB()
//
IndexDB::~IndexDB()
{
    Close();
}


//*****************************************************************************
// int IndexDB::Open(char *filename, char *indexname, char *headname)
//   We will attempt to open up an existing document database.  If it
//   doesn't exist, we'll create a new one.
//
int IndexDB::Open(const String& indexfilename)
{
    // 
    // If the database is already open, we'll close it
    // We might be opening this object with a new filename, so we'll be safe
    // 
    Close();

    i_dbf = Database::getDatabaseInstance(DB_HASH);

    if (i_dbf->OpenReadWrite(indexfilename, 0666) == OK)
    {
        isopen = 1;
        return OK;
    }
    else
    {
        cerr << "IndexDB::Open: " << indexfilename << " " << strerror(errno) << "\n";
        return NOTOK;
    }
}



//*****************************************************************************
// int IndexDB::Close()
//   Close the database.  Before we close it, we first need to update
//   the special record which keeps track our nextDocID variable.
//
int IndexDB::Close()
{
    //
    // Check to see if it's closed already
    // 
    if (!isopen)
        return OK;

    i_dbf->Close();
    delete i_dbf;

    i_dbf = 0;
    isopen = 0;

    return OK;
}


//*****************************************************************************
// int IndexDB::Add(DocumentRef &doc)
//
int IndexDB::Add(IndexDBRef &iref)
{
    if (i_dbf)
    {
        String value = 0;
        iref.Serialize(value);
        String key = iref.DocURL();
        return i_dbf->Put(key, value);
    }
    else
    {
      // If there was no index when we write, something is wrong.
      return NOTOK;
    }
}


//*****************************************************************************
// int IndexDB::Delete(const String& u)
//
int IndexDB::Delete(const String& u)
{
    if (i_dbf)
    {
        String  url(u);
        return i_dbf->Delete(url);
    }
    else
    {
        return NOTOK;
    }
}


//*****************************************************************************
// int IndexDB::Exists(const String& u)
//
IndexDBRef *IndexDB::Exists(const String& u)
{
    if (i_dbf)
    {
        String      url(u);
        String      data;

        if (i_dbf->Get(url, data) == NOTOK)
            return 0;

        IndexDBRef  *iref = new IndexDBRef;

        iref->Deserialize(data);
        iref->DocURL(url.get());
        return iref;
    }
    else
    {
        return 0;
    }
}



//*****************************************************************************
// List *IndexDB::URLs()
//   Return a list of all the URLs in the database
//
List *IndexDB::URLs()
{
    if (i_dbf)
    {
        List    *list = new List;
        char    *coded_key;

        i_dbf->Start_Get();
        while ((coded_key = i_dbf->Get_Next()))
        {
            String *key = new String(coded_key);
            list->Add(key);
        }
        return list;
    }
    else
    {
        return 0;
    }
}


// End of IndexDB.cc
