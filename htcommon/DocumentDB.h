//
// DocumentDB.h
//
// DocumentDB: This class is the interface to the database of document
//             references. This database is only used while digging.  
//             An extract of this database is used for searching.  
//             This is because digging requires a different index
//             than searching.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: DocumentDB.h,v 1.11.2.1 2000/02/14 06:08:17 ghutchis Exp $
//

#ifndef _DocumentDB_h_
#define _DocumentDB_h_

#include "DocumentRef.h"
#include "List.h"
#include "Database.h"
#include "IntObject.h"

/* This is where the running document counter is stored.
   The first real document number is the next. */
#define NEXT_DOC_ID_RECORD 1


class DocumentDB
{
public:
    //
    // Construction/Destruction
    //
    DocumentDB();
    ~DocumentDB();

    //
    // The database used for searching is generated from our internal database:
    //
    int			CreateSearchDB(const String& filename);

    //
    // Standard database operations
    //
    int			Open(const String& filename, const String& indexfilename, const String& headname);
    int			Read(const String& filename, const String& indexfilename = 0, const String& headfilename = 0);
    int			Close();

    int			Add(DocumentRef &);
    // These do not read in the excerpt
    DocumentRef		*operator [] (int DocID);
    DocumentRef		*operator [] (const String& url);
    // You must call this to read the excerpt
    int			ReadExcerpt(DocumentRef &);
    int			Exists(int DocID);
    int			Delete(int DocID);

    //
    // The database keeps track of document ids.  Here is a way to get
    // the next document id.
    //
    int			NextDocID()		{return nextDocID++;}

    // And here's a way to increment NextDocID after adding lots of records
    // (for example when merging databases!)
    void                IncNextDocID (int next)     {nextDocID += next;}

    //
    // We will need to be able to iterate over the complete database.
    //

    // This returns a list of all the URLs, as String *
    List		*URLs();

    // This returns a list of all the DocIDs, as IntObject *
    List		*DocIDs();

private:
    Database		*dbf;
    Database		*i_dbf;
    Database		*h_dbf;
    int			isopen;
    int			isread;
    int			nextDocID;
};

#endif


