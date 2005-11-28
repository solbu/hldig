//
// IndexDB.h
//
// IndexDB: This class is the interface to the database of document
//             index references. This database is only used while digging.  
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
//

#ifndef _IndexDB_h_
#define _IndexDB_h_

#include "IndexDBRef.h"
#include "List.h"
#include "Database.h"
#include "IntObject.h"

/* This is where the running document counter is stored.
   The first real document number is the next. */
#define NEXT_DOC_ID_RECORD 1


class IndexDB
{
public:
    //
    // Construction/Destruction
    //
    IndexDB();
    ~IndexDB();


    //
    // Standard database operations
    //
    int         Open(const String& indexfilename);
    int         Read(const String& indexfilename);
    int         Close();

    int         Add(IndexDBRef & iref);
    int         Delete(const String& url);
    // These do not read in the excerpt
    IndexDBRef  *operator[] (const String& u);
    IndexDBRef  *Exists(const String& u);
    // You must call this to read the excerpt

    //
    // We will need to be able to iterate over the complete database.
    //

    // This returns a list of all the URLs, as String *
    List        *URLs();

private:
    Database    *i_dbf;
    int         isopen;
    int         isread;
};

#endif


