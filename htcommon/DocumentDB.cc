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
// $Id: DocumentDB.cc,v 1.28.2.7 2000/05/05 21:55:11 loic Exp $
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
#include <iostream.h>
#include <fstream.h>
#include <errno.h>

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
}


//*****************************************************************************
// DocumentDB::~DocumentDB()
//
DocumentDB::~DocumentDB()
{
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
int DocumentDB::Open(const String& filename, const String& indexfilename, const String& headname)
{
  // If the database is already open, we'll close it
  // We might be opening this object with a new filename, so we'll be safe
  Close();

  dbf = 0;
  i_dbf = 0;
  h_dbf = 0;

  i_dbf = Database::getDatabaseInstance(DB_HASH);

  if (i_dbf->OpenReadWrite(indexfilename, 0666) != OK) {
    cerr << "DocumentDB::Open: " << indexfilename << " " << strerror(errno) << "\n";
    return NOTOK;
  }

  h_dbf = Database::getDatabaseInstance(DB_HASH);

  if (h_dbf->OpenReadWrite(headname, 0666) != OK) {
    cerr << "DocumentDB::Open: " << headname << " " << strerror(errno) << "\n";
    return NOTOK;
  }

  dbf = Database::getDatabaseInstance(DB_HASH);
	
  if (dbf->OpenReadWrite(filename, 0666) == OK)
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
  else {
    cerr << "DocumentDB::Open: " << filename << " " << strerror(errno) << "\n";
    return NOTOK;
  }
}


//*****************************************************************************
// int DocumentDB::Read(char *filename, char *indexname, char *headname)
//   We will attempt to open up an existing document database,
//   and accompanying index database and excerpt database
//
int DocumentDB::Read(const String& filename, const String& indexfilename = 0, const String& headfilename = 0)
{
    // If the database is already open, we'll close it
    // We might be opening this object with a new filename, so we'll be safe
    Close();

    dbf = 0;
    i_dbf = 0;
    h_dbf = 0;

    if (!indexfilename.empty())
    {
	i_dbf = Database::getDatabaseInstance(DB_HASH);

	if (i_dbf->OpenRead(indexfilename) != OK)
	    return NOTOK;
    }

    if (!headfilename.empty())
      {
	h_dbf = Database::getDatabaseInstance(DB_HASH);
	
	if (h_dbf->OpenRead(headfilename) != OK)
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
    if (!isopen) return OK;

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

    String temp = 0;

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

    ref.DocHead((char*)HtZlibCodec::instance()->decode(data));

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
// DocumentRef *DocumentDB::operator [] (const String& u)
//
DocumentRef *DocumentDB::operator [] (const String& u)
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
  
	if (i_dbf->Get(HtURLCodec::instance()->encode(url), docIDstr) == NOTOK)
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
  
    // We have to be really careful about deleting by URL, we might
    // have a newer "edition" with the same URL and different DocID
    String		docIDstr;
    String		encodedURL = HtURLCodec::instance()->encode(url);
    if (i_dbf->Get(encodedURL, docIDstr) == NOTOK)
      return NOTOK;

    // Only delete if we have a match between what we want to delete
    // and what's in the database
    if (key == docIDstr && i_dbf->Delete(encodedURL) == NOTOK)
	return NOTOK;
  
    if (h_dbf == 0 || h_dbf->Delete(key) == NOTOK)
      return NOTOK;

    return dbf->Delete(key);
}

//*****************************************************************************
// int DocumentDB::DumpDB(char *filename, int verbose)
//   Create an extract from our database which can be used by an
//   external application. The extract will consist of lines with fields
//   separated by tabs. 
//
//   The extract will likely not be sorted by anything in particular
//
int DocumentDB::DumpDB(const String& filename, int verbose)
{
    DocumentRef	        *ref;
    List		*descriptions, *anchors;
    char		*strkey;
    String		data;
    FILE		*fl;
    String		docKey(sizeof(int));

    if((fl = fopen(filename, "w")) == 0) {
      perror(form("DocumentDB::DumpDB: opening %s for writing",
		  (const char*)filename));
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
		ref->DocHead((char*)HtZlibCodec::instance()->decode(data));
	      }
	    fprintf(fl, "%d", ref->DocID());
	    fprintf(fl, "\tu:%s", ref->DocURL());
	    fprintf(fl, "\tt:%s", ref->DocTitle());
	    fprintf(fl, "\ta:%d", ref->DocState());
	    fprintf(fl, "\tm:%d", (int) ref->DocTime());
	    fprintf(fl, "\ts:%d", ref->DocSize());
	    fprintf(fl, "\tH:%s", ref->DocHead());
	    fprintf(fl, "\th:%s", ref->DocMetaDsc());
	    fprintf(fl, "\tl:%d", (int) ref->DocAccessed());
	    fprintf(fl, "\tL:%d", ref->DocLinks());
	    fprintf(fl, "\tb:%d", ref->DocBackLinks());
	    fprintf(fl, "\tc:%d", ref->DocHopCount());
	    fprintf(fl, "\tg:%d", ref->DocSig());
	    fprintf(fl, "\te:%s", ref->DocEmail());
	    fprintf(fl, "\tn:%s", ref->DocNotification());
	    fprintf(fl, "\tS:%s", ref->DocSubject());
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
// int DocumentDB::LoadDB(const String &filename, int verbose)
//   Load an extract to our database from an ASCII file
//   The extract will consist of lines with fields separated by tabs. 
//   The lines need not be sorted in any fashion.
//
int DocumentDB::LoadDB(const String& filename, int verbose)
{
    FILE	*input;
    String	docKey(sizeof(int));
    DocumentRef ref;
    StringList	descriptions, anchors;
    char	*token, field;
    String	data;

    if((input = fopen(filename, "r")) == 0) {
      perror(form("DocumentDB::LoadDB: opening %s for reading", 
		  (const char*)filename));
      return NOTOK;
    }

    while (data.readLine(input))
    {
	token = strtok(data, "\t");
	if (token == NULL)
	  continue;

	ref.DocID(atoi(token));
	
	if (verbose)
	  cout << "\t loading document ID: " << ref.DocID() << endl;

	while ( (token = strtok(0, "\t")) )
	  {
	    field = *token;
	    token += 2;

	    if (verbose > 2)
		cout << "\t field: " << field;

	    switch(field)
	      {
	        case 'u': // URL
		  ref.DocURL(token);
		  break;
	        case 't': // Title
		  ref.DocTitle(token);
		  break;
	        case 'a': // State
		  ref.DocState(atoi(token));
		  break;
	        case 'm': // Modified
		  ref.DocTime(atoi(token));
		  break;
	        case 's': // Size
		  ref.DocSize(atoi(token));
		  break;
	        case 'H': // Head
		  ref.DocHead(token);
		  break;
	        case 'h': // Meta Description
		  ref.DocMetaDsc(token);
		  break;
	        case 'l': // Accessed
		  ref.DocAccessed(atoi(token));
		  break;
	        case 'L': // Links
		  ref.DocLinks(atoi(token));
		  break;
	        case 'b': // BackLinks
		  ref.DocBackLinks(atoi(token));
		  break;
	        case 'c': // HopCount
		  ref.DocHopCount(atoi(token));
		  break;
	        case 'g': // Signature
		  ref.DocSig(atoi(token));
		  break;
	        case 'e': // E-mail
		  ref.DocEmail(token);
		  break;
	        case 'n': // Notification
		  ref.DocNotification(token);
		  break;
	        case 'S': // Subject
		  ref.DocSubject(token);
		  break;
	        case 'd': // Descriptions
		  descriptions.Create(token, '\001');
		  ref.Descriptions(descriptions);
		  break;
	        case 'A': // Anchors
		  anchors.Create(token, '\001');
		  ref.DocAnchors(anchors);
		  break;
	        default:
		  break;
	      }

	  }
	

	// We must be careful if the document already exists
	// So we'll delete the old document and add the new one
	if (Exists(ref.DocID()))
	  {
	    Delete(ref.DocID());
	  }
	Add(ref);

	// If we add a record with an ID past nextDocID, update it
	if (ref.DocID() > nextDocID)
	  nextDocID = ref.DocID() + 1;

	descriptions.Destroy();
	anchors.Destroy();
    }

    fclose(input);
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

//*****************************************************************************
// private
// int readLine(FILE *in, String &line)
//
int readLine(FILE *in, String &line)
{
    char	buffer[2048];
    int		length;
    
    line = 0;
    while (fgets(buffer, sizeof(buffer), in))
    {
	length = strlen(buffer);
	if (buffer[length - 1] == '\n')
	{
	    //
	    // A full line has been read.  Return it.
	    //
	    line << buffer;
	    line.chop('\n');
	    return 1;
	}
	else
	{
	    //
	    // Only a partial line was read.  Append it to the line
	    // and read some more.
	    //
	    line << buffer;
	}
    }
    return line.length() > 0;
}

// End of DocumentDB.cc
