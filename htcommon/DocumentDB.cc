//
// DocumentDB.cc
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
// $Id: DocumentDB.cc,v 1.23 1999/09/10 11:45:29 loic Exp $
//

#include "DocumentDB.h"
#include "Database.h"
#include "HtURLCodec.h"
#include "IntObject.h"
#include "HtZlibCodec.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <fstream.h>


//*****************************************************************************
// DocumentDB::DocumentDB()
//
DocumentDB::DocumentDB()
{
    isopen = 0;
    isread = 0;

    // The first document number (NEXT_DOC_ID_RECORD) is used to
    // store the nextDocID number itself into.  We avoid using
    // an all-0 key for this, mostly for being superstitious
    // about letting in bugs.
    nextDocID = NEXT_DOC_ID_RECORD + 1;
    myTryUncoded = 1;
}


//*****************************************************************************
// DocumentDB::~DocumentDB()
//
DocumentDB::~DocumentDB()
{
    if (isopen)
	Close();
}


//*****************************************************************************
// int DocumentDB::Open(char *filename, char *indexname, char *headname)
//   We will attempt to open up an existing document database.  If it
//   doesn't exist, we'll create a new one.  If we are succesful in
//   opening the database, we need to look for our special record
//   which contains the next document ID to use.
//    There may also be an URL -> DocID index database to take
//   care of, as well as a DocID -> DocHead excerpt database.
//
int DocumentDB::Open(char *filename, char *indexname, char *headname)
{
    // If the database is already open, we'll close it
    // We might be opening this object with a new filename, so we'll be safe
    if (isopen)
      Close();

    dbf = 0;
    i_dbf = 0;
    h_dbf = 0;

    i_dbf = Database::getDatabaseInstance(DB_HASH);

    if (i_dbf->OpenReadWrite(indexname, 0664) != OK)
	return NOTOK;

    h_dbf = Database::getDatabaseInstance(DB_HASH);

    if (h_dbf->OpenReadWrite(headname, 0664) != OK)
	return NOTOK;

    dbf = Database::getDatabaseInstance(DB_HASH);
	
    if (dbf->OpenReadWrite(filename, 0664) == OK)
    {
	String		data;
	int             specialRecordNumber = NEXT_DOC_ID_RECORD;
	String          key((char *) &specialRecordNumber,
			    sizeof specialRecordNumber);
	if (dbf->Get(key, data) == OK)
	{
	    memcpy(&nextDocID, data.get(), sizeof nextDocID);
	}

	isopen = 1;
	return OK;
    }
    else
	return NOTOK;
}


//*****************************************************************************
// int DocumentDB::Read(char *filename, char *indexname, char *headname)
//   We will attempt to open up an existing document database,
//   and accompanying index database and excerpt database
//
int DocumentDB::Read(char *filename, char *indexname, char *headname)
{
    // If the database is already open, we'll close it
    // We might be opening this object with a new filename, so we'll be safe
    if (isopen)
      Close();

    dbf = 0;
    i_dbf = 0;
    h_dbf = 0;

    if (indexname)
    {
	i_dbf = Database::getDatabaseInstance(DB_HASH);

	if (i_dbf->OpenRead(indexname) != OK)
	    return NOTOK;
    }

    if (headname)
      {
	h_dbf = Database::getDatabaseInstance(DB_HASH);
	
	if (h_dbf->OpenRead(headname) != OK)
	  return NOTOK;
      }

    dbf = Database::getDatabaseInstance(DB_HASH);
	
    if (dbf->OpenRead(filename) == OK)
    {
	isopen = 1;
	isread = 1;
	return OK;
    }
    else
	return NOTOK;
}


//*****************************************************************************
// int DocumentDB::Close()
//   Close the database.  Before we close it, we first need to update
//   the special record which keeps track our nextDocID variable.
//
int DocumentDB::Close()
{
    if (!isread)
    {
	int specialRecordNumber = NEXT_DOC_ID_RECORD;
	String key((char *) &specialRecordNumber,
		   sizeof specialRecordNumber);
	String data((char *) &nextDocID, sizeof nextDocID);

	dbf->Put(key, data);
    }

    if (i_dbf)
    {
	i_dbf->Close();
	delete i_dbf;
	i_dbf = 0;
    }
    if (h_dbf)
      {
	h_dbf->Close();
	delete h_dbf;
	h_dbf = 0;
      }

    dbf->Close();
    delete dbf;
    dbf = 0;
    isopen = 0;
    isread = 0;
    return OK;
}


//*****************************************************************************
// int DocumentDB::Add(DocumentRef &doc)
//
int DocumentDB::Add(DocumentRef &doc)
{
    int docID = doc.DocID();

    temp = 0;
    doc.Serialize(temp);

    String key((char *) &docID, sizeof docID);
    dbf->Put(key, temp);

    if (h_dbf)
      {
	if (doc.DocHeadIsSet())
	  {
	    temp = HtZlibCodec::instance()->encode(doc.DocHead());
	    h_dbf->Put(key, temp);
	  }
      }
    else
      // If there was no excerpt index when we write, something is wrong.
      return NOTOK;

    if (i_dbf)
    {
	temp = doc.DocURL();
	i_dbf->Put(HtURLCodec::instance()->encode(temp), key);
	return OK;
    }
    else
      // If there was no index when we write, something is wrong.
      return NOTOK;
}


//*****************************************************************************
// int DocumentDB::ReadExcerpt(DocumentRef &ref)
// We will attempt to access the excerpt for this ref
//
int DocumentDB::ReadExcerpt(DocumentRef &ref)
{
    String	data;
    int		docID = ref.DocID();
    String	key((char *) &docID, sizeof docID);

    if (!h_dbf)
      return NOTOK;
    if (h_dbf->Get(key, data) == NOTOK)
      return NOTOK;

    ref.DocHead(HtZlibCodec::instance()->decode(data));

    return OK;
}

//*****************************************************************************
// DocumentRef *DocumentDB::operator [] (int docID)
//
DocumentRef *DocumentDB::operator [] (int docID)
{
    String			data;
    String			key((char *) &docID, sizeof docID);

    if (dbf->Get(key, data) == NOTOK)
      return 0;

    DocumentRef		*ref = new DocumentRef;
    ref->Deserialize(data);
    return ref;
}


//*****************************************************************************
// DocumentRef *DocumentDB::operator [] (char *u)
//
DocumentRef *DocumentDB::operator [] (char *u)
{
    String			data;
    String			docIDstr;

    // If there is no index db, then just give up 
    // (do *not* construct a list and traverse it).
    if (i_dbf == 0)
      return 0;
    else
    {
	String url(u);
  
	if (i_dbf->Get(HtURLCodec::instance()->encode(url), docIDstr) == NOTOK
	    && (! myTryUncoded || i_dbf->Get(url, docIDstr) == NOTOK))
	  return 0;
    }

    if (dbf->Get(docIDstr, data) == NOTOK)
      return 0;

    DocumentRef		*ref = new DocumentRef;
    ref->Deserialize(data);
    return ref;
}

//*****************************************************************************
// int DocumentDB::Exists(int docID)
//
int DocumentDB::Exists(int docID)
{
    String key((char *) &docID, sizeof docID);
    return dbf->Exists(key);
}

//*****************************************************************************
// int DocumentDB::Delete(int docID)
//
int DocumentDB::Delete(int docID)
{
    String key((char*) &docID, sizeof docID);
    String data;
  
    if (i_dbf == 0 || dbf->Get(key, data) == NOTOK)
      return NOTOK;
  
    DocumentRef		*ref = new DocumentRef;
    ref->Deserialize(data);
    String url = ref->DocURL();
    delete ref;
  
    if (i_dbf->Delete(HtURLCodec::instance()->encode(url)) == NOTOK
	&& (! myTryUncoded || i_dbf->Delete(url) == NOTOK))
      return 0;
  
    if (h_dbf == 0 || h_dbf->Delete(key) == NOTOK)
      return NOTOK;

    return dbf->Delete(key);
}

//*****************************************************************************
// int DocumentDB::CreateSearchDB(char *filename)
//   Create an extract from our database which can be used by an
//   external application. The extract will consist of lines with fields
//   separated by tabs. 
//
//   The extract will be sorted by docID.
//
int DocumentDB::CreateSearchDB(char *filename)
{
    DocumentRef	        *ref;
    List		*descriptions, *anchors;
    char		*strkey;
    String		data;
    FILE		*fl;
    String		docKey(sizeof(int));

    if((fl = fopen(filename, "w")) == 0) {
      perror(form("DocumentDB::CreateSearchDB: opening %s for writing", filename));
      return NOTOK;
    }

    dbf->Start_Get();
    while ((strkey = dbf->Get_Next()))
    {
	int docID;
	memcpy(&docID, strkey, sizeof docID);

	docKey = 0;
	docKey.append((char *) &docID, sizeof docID);

	dbf->Get(docKey, data);

	if (docID != NEXT_DOC_ID_RECORD)
	{
	    ref = new DocumentRef;
	    ref->Deserialize(data);
	    if (h_dbf)
	      {
		h_dbf->Get(docKey,data);
		ref->DocHead(data);
	      }
	    fprintf(fl, "%d", ref->DocID());
	    fprintf(fl, "\tu:%s", ref->DocURL());
	    fprintf(fl, "\tt:%s", ref->DocTitle());
	    fprintf(fl, "\ta:%d", ref->DocState());
	    fprintf(fl, "\tm:%d", (int) ref->DocTime());
	    fprintf(fl, "\ts:%d", ref->DocSize());
	    fprintf(fl, "\th:%s", ref->DocHead());
	    fprintf(fl, "\th:%s", ref->DocMetaDsc());
	    fprintf(fl, "\tl:%d", (int) ref->DocAccessed());
	    fprintf(fl, "\tL:%d", ref->DocLinks());
	    fprintf(fl, "\tI:%d", ref->DocImageSize());
	    fprintf(fl, "\td:");
	    descriptions = ref->Descriptions();
	    String	*description;
	    descriptions->Start_Get();
	    int		first = 1;
	    while ((description = (String *) descriptions->Get_Next()))
	    {
		if (!first)
		    fprintf(fl, "\001");
		first = 0;
		fprintf(fl, "%s", description->get());
	    }
	    fprintf(fl, "\tA:");
	    anchors = ref->DocAnchors();
	    String	*anchor;
	    anchors->Start_Get();
	    first = 1;
	    while ((anchor = (String *) anchors->Get_Next()))
	    {
		if (!first)
		    fprintf(fl, "\001");
		first = 0;
		fprintf(fl, "%s", anchor->get());
	    }
	    fprintf(fl, "\n");
    	    delete ref;
	}
    }

    fclose(fl);

    return OK;
}


//*****************************************************************************
// List *DocumentDB::URLs()
//   Return a list of all the URLs in the database
//   Only available when there's an URL -> DocID index db handy.
//
List *DocumentDB::URLs()
{
    List	*list = new List;
    char	*coded_key;

    if (i_dbf == 0)
	return 0;

    i_dbf->Start_Get();
    while ((coded_key = i_dbf->Get_Next()))
    {
	String *key = new String(HtURLCodec::instance()->decode(coded_key));
	list->Add(key);
    }
    return list;
}


//*****************************************************************************
// List *DocumentDB::DocIDs()
//   Return a list of all the DocIDs in the database
//
List *DocumentDB::DocIDs()
{
    List	*list = new List;
    char	*key;

    dbf->Start_Get();
    while ((key = dbf->Get_Next()))
    {
	int	    docID;
	memcpy (&docID, key, sizeof docID);

	if (docID != NEXT_DOC_ID_RECORD)
	    list->Add(new IntObject(docID));
    }
    return list;
}


// End of DocumentDB.cc
