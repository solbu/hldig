//
// DocumentDB.h
//
// This class is the interface to the database of document references.
// This database is only used while digging.  An extract of this
// database is used for searching.  This is because digging requires a
// different index than searching.
//
// $Id: DocumentDB.h,v 1.6 1999/03/12 00:47:00 hp Exp $
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
    int			CreateSearchDB(char *filename);

    //
    // Standard database operations
    //
    int			Open(char *filename, char *indexfilename);
    int			Read(char *filename, char *indexfilename = 0);
    int			Close();

    int			Add(DocumentRef &);
    DocumentRef		*operator [] (int DocID);
    DocumentRef		*operator [] (char *url);
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

    //
    // Set compatibility mode (try to support when index
    // contains *unencoded* URLs as keys).
    //
    inline void                SetCompatibility(int on_flag = 1)
    { myTryUncoded = on_flag; }

private:
    Database		*dbf;
    Database		*i_dbf;
    int			isopen;
    int			isread;
    int			nextDocID;
    String                    temp;
    int			myTryUncoded;	// "Compatibility" mode.
};

#endif


