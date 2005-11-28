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
    isread = 0;
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
    // If the database is already open, we'll close it
    // We might be opening this object with a new filename, so we'll be safe
    Close();

    i_dbf = 0;

    i_dbf = Database::getDatabaseInstance(DB_HASH);

    if (i_dbf->OpenReadWrite(indexfilename, 0666) != OK) {
        cerr << "IndexDB::Open: " << indexfilename << " " << strerror(errno) << "\n";
        return NOTOK;
    }
    else
        return OK;
}


//*****************************************************************************
// int IndexDB::Read(char *filename, char *indexname, char *headname)
//   We will attempt to open up an existing database
//
int IndexDB::Read(const String& indexfilename)
{
    // If the database is already open, we'll close it
    // We might be opening this object with a new filename, so we'll be safe
    Close();

    i_dbf = 0;

    if (!indexfilename.empty())
    {
        i_dbf = Database::getDatabaseInstance(DB_HASH);

        if (i_dbf->OpenRead(indexfilename) != OK) {
            return NOTOK;
        }
        else
        {
            isopen = 1;
            isread = 1;
            return OK;
        }
    }
    else
    {
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
    if (!isopen) return OK;

    isopen = 0;
    isread = 0; 
    i_dbf->Close();
    i_dbf = 0;
    delete i_dbf;

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

        return i_dbf->Put(HtURLCodec::instance()->encode(key), value);
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
    String  url(u);
  
    if (i_dbf == 0)
      return NOTOK;
    else
      return i_dbf->Delete(HtURLCodec::instance()->encode(url));  
}


//*****************************************************************************
// DocumentRef *IndexDB::operator [] (const String& u)
//
IndexDBRef *IndexDB::operator[] (const String& u)
{
    if (i_dbf)
    {
        String      url(u);
        String      data;

        if (i_dbf->Get(HtURLCodec::instance()->encode(url), data) == NOTOK)
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
// int IndexDB::Exists(const String& u)
//
IndexDBRef *IndexDB::Exists(const String& u)
{
    if (i_dbf)
    {
        String      url(u);
        String      data;

        cout << "attempting retrieval of " << url << " from index DB" << endl;
        if (i_dbf->Get(HtURLCodec::instance()->encode(url), data) == NOTOK)
            return 0;

        IndexDBRef  *iref = new IndexDBRef;

        cout << "attempting to deserialize " << data << endl;
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
//   Only available when there's an URL -> DocID index db handy.
//
List *IndexDB::URLs()
{
    List	*list = new List;
    char	*coded_key;

    if (i_dbf)
    {
        i_dbf->Start_Get();
        while ((coded_key = i_dbf->Get_Next()))
        {
            String *key = new String(HtURLCodec::instance()->decode(coded_key));
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
