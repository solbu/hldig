//
// DocumentDB.h
//
// This class is the interface to the database of document references.
// This database is only used while digging.  An extract of this
// database is used for searching.  This is because digging requires a
// different index than searching.
//
// $Id: DocumentDB.h,v 1.5.2.2 2001/07/24 18:33:48 grdetil Exp $
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2001 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
#ifndef _DocumentDB_h_
#define _DocumentDB_h_

#include "DocumentRef.h"
#include "List.h"
#include "Database.h"


class DocumentDB
{
public:
    //
    // Construction/Destruction
    //
    DocumentDB();
    ~DocumentDB();

    //
    // Standard database operations
    //
    int			Open(char *filename);
    int			Read(char *filename);
    int			Close();

    int			Add(DocumentRef &);
    DocumentRef		*operator [] (char *url);
    DocumentRef		*FindCoded(char *url);
    int			Exists(char *url);
    int			Delete(char *url);

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
    List		*URLs();	// This returns a list of all the URLs

    // Dump the database out to an ASCII text file
    int			DumpDB(char *filename, int verbose = 0);

    // Read in the database from an ASCII text file
    // (created by DumpDB)
    int			LoadDB(char *filename, int verbose = 0);

    //
    // Set compatibility mode (try to support when database
    // contains *unencoded* URLs as keys).
    //
    inline void                SetCompatibility(int on_flag = 1)
    { myTryUncoded = on_flag; }

private:
    Database		*dbf;
    int			isopen;
    int			isread;
    int			nextDocID;
    String                    temp;
    int			myTryUncoded;	// "Compatibility" mode.
};

#endif


