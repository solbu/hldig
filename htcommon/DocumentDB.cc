//
// DocumentDB.cc
//
// Implementation of DocumentDB
//
//
//

#include "DocumentDB.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <fstream.h>
#include "Database.h"
#include "HtURLCodec.h"


//*****************************************************************************
// DocumentDB::DocumentDB()
//
DocumentDB::DocumentDB()
{
    isopen = 0;
    isread = 0;
    nextDocID = 0;
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
// int DocumentDB::Open(char *filename)
//   We will attempt to open up an existing document database.  If it
//   doesn't exist, we'll create a new one.  If we are succesfull in
//   opening the database, we need to look for our special record
//   which contains the next document ID to use.
//
int DocumentDB::Open(char *filename)
{
    dbf = Database::getDatabaseInstance();
	
    if (dbf->OpenReadWrite(filename, 0664) == OK)
    {
	String		data;
	if (dbf->Get("nextDocID", data) == OK)
	{
	    nextDocID = atoi(data);
	}
	isopen = 1;
	return OK;
    }
    else
	return NOTOK;
}


//*****************************************************************************
// int DocumentDB::Read(char *filename)
//   We will attempt to open up an existing document database.
//
int DocumentDB::Read(char *filename)
{
    dbf = Database::getDatabaseInstance();
	
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
    String	data;

    if (!isread)
    {
	data << nextDocID;
	dbf->Put("nextDocID", data.get(), data.length() + 1);
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
    String	url;
    url = doc.DocURL();
    // Why would we want to lowercase the URL before storing?
    // URLs can be case sensitive!
    //    url.lowercase();
    temp = 0;
    doc.Serialize(temp);

    // If in compatibility-mode, there may be an unencoded url
    // that has to be deleted to avoid duplicate URLs.
    if (myTryUncoded)
      dbf->Delete(url);

    dbf->Put(HtURLCodec::instance()->encode(url), temp);
    return OK;
}


//*****************************************************************************
// DocumentRef *DocumentDB::operator [] (char *u)
//
DocumentRef *DocumentDB::operator [] (char *u)
{
    String			data;
    String			url = u;
    // Why would we lowercase the URL before using it?
    // URLs can be case sensitive!
    // url.lowercase();

    if (dbf->Get(HtURLCodec::instance()->encode(url), data) == NOTOK
        && (! myTryUncoded || dbf->Get(url, data) == NOTOK))
	return 0;

    DocumentRef		*ref = new DocumentRef;
    ref->Deserialize(data);
    return ref;
}


//*****************************************************************************
// int DocumentDB::Exists(char *u)
//
int DocumentDB::Exists(char *u)
{
    String			url = u;
    // Why would we lowercase, URLs can be case-sensitive!
    //    url.lowercase();

    return dbf->Exists(HtURLCodec::instance()->encode(url))
      || (myTryUncoded && dbf->Exists(url));
}


//*****************************************************************************
// int DocumentDB::Delete(char *u)
//
int DocumentDB::Delete(char *u)
{
    String			url = u;
    // Why would we lowercase the URL, they can be case-sensitive!
    // url.lowercase();
    int delete_stat = dbf->Delete(HtURLCodec::instance()->encode(url));

    // If the deletion was not successful (maybe the item did
    // not exist) delete the unencoded URL if we should be compatible.
    return (delete_stat != 0 && myTryUncoded)
      ? dbf->Delete(url) : delete_stat;
}


//*****************************************************************************
// int DocumentDB::CreateSearchDB(char *filename)
//   Create an extract from our database which can be used by the
//   search engine.  The extract will consist of lines with fields
//   separated by tabs.  The fields are:
//        docID
//        docURL
//        docTime
//        docHead
//        docMetaDsc
//        descriptions (separated by tabs)
//
//   The extract will be sorted by docID.
//
int DocumentDB::CreateSearchDB(char *filename)
{
    DocumentRef	        *ref;
    List		*descriptions, *anchors;
    char		*key;
    String		data;
    FILE		*fl;
    String		command = SORT_PROG;
    String		tmpdir = getenv("TMPDIR");

    command << " -n -o" << filename;
    if (tmpdir.length())
    {
	command << " -T " << tmpdir;
    }
    fl = popen(command, "w");

    dbf->Start_Get();
    while ((key = dbf->Get_Next()))
    {
	dbf->Get(key, data);
	if (strncmp(HtURLCodec::instance()->decode(key), "http:", 5) == 0)
	{
	    ref = new DocumentRef;
	    ref->Deserialize(data);
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

    int	sortRC = pclose(fl);
    if (sortRC)
    {
	cerr << "Document sort failed\n\n";
	exit(1);
    }
    return 0;
}


//*****************************************************************************
// List *DocumentDB::URLs()
//   Return a list of all the URLs in the database
//
List *DocumentDB::URLs()
{
    List	*list = new List;
    char	*coded_key;

    dbf->Start_Get();
    while ((coded_key = dbf->Get_Next()))
    {
	String key = HtURLCodec::instance()->decode(coded_key);
	if (mystrncasecmp(key, "http:", 5) == 0)
	{
	    DocumentRef	*ref = (*this)[key];
	    if (ref)
	    	list->Add(new String(ref->DocURL()));
            delete ref;
	}
    }
    return list;
}


