//
// docs.cc
//
// Do sanity checking in "doc_db", remove insane documents.
//
// $Id: docs.cc,v 1.20 1999/07/19 02:03:47 ghutchis Exp $
//
//

#include "htmerge.h"


//*****************************************************************************
// void convertDocs()
//
void
convertDocs()
{
    char		*doc_db = config["doc_db"];
    char		*doc_index = config["doc_index"];
    char		*doc_excerpt = config["doc_excerpt"];
    int			remove_unused = config.Boolean("remove_bad_urls");
    DocumentDB		db;
    List		*IDs;
    int			document_count = 0;
    unsigned long	docdb_size = 0;

    if (access(doc_db, R_OK) < 0)
    {
	reportError(form("Unable to open document database '%s'", doc_db));
    }

    // These don't need to be fatal since we could make do otherwise...
    // It is (very) nice to have the URL around for messages though!
    if (access(doc_index, R_OK) < 0)
    {
	reportError(form("Unable to open document index '%s'", doc_index));
    }
    if (access(doc_excerpt, R_OK) < 0)
    {
	reportError(form("Unable to open document excerpts '%s'", doc_excerpt));
    }

    // Check "uncompressed"/"uncoded" urls at the price of time
    // (extra DB probes).
    db.SetCompatibility(config.Boolean("uncoded_db_compatible", 1));

    //
    // Start the conversion by going through all the URLs that are in
    // the document database
    //
    db.Open(doc_db, doc_index, doc_excerpt);
    IDs = db.DocIDs();
	
    IDs->Start_Get();
    IntObject		*id;
    String		idStr;
    String		url;
    while ((id = (IntObject *) IDs->Get_Next()))
    {
	DocumentRef	*ref = db[id->Value()];

	if (!ref)
	    continue;

	db.ReadExcerpt(*ref);
	url = ref->DocURL();
	idStr = id->Value();

	if (ref->DocState() == Reference_noindex)
	  {
	    // This document either wasn't found or shouldn't be indexed.
	    db.Delete(ref->DocID());
            if (verbose)
              cout << "Deleted, noindex: " << id->Value() << " URL: "
                   << url << endl;
	    discard_list.Add(idStr.get(), NULL);
	  }
	else if (remove_unused && ref->DocState() == Reference_not_found)
	  {
	    // This document wasn't actually found
	    db.Delete(ref->DocID());
            if (verbose)
              cout << "Deleted, noindex: " << id->Value() << " URL: "
                   << url << endl;
	    discard_list.Add(idStr.get(), NULL);
	  }
	else if (remove_unused && strlen(ref->DocHead()) == 0)
	  {
	    // For some reason, this document doesn't have an excerpt
	    // (probably because of a noindex directive, or disallowed
	    // by robots.txt or server_max_docs). Remove it
	    db.Delete(ref->DocID());
            if (verbose)
              cout << "Deleted, no excerpt: " << id->Value() << " URL:  "
                   << url << endl;
	    discard_list.Add(idStr.get(), NULL);
	  }
	else
	  {
	    // This is a valid document. Let's keep stats on it.
            if (verbose > 1)
              cout << "" << id->Value() << "/" << url << endl;

	    document_count++;
	    docdb_size += ref->DocSize();
	    if (verbose && document_count % 10 == 0)
	    {
		cout << "htmerge: " << document_count << '\n';
		cout.flush();
	    }
	  }
        delete ref;
    }
    if (verbose)
	cout << "\n";
    if (stats)
      {
	cout << "htmerge: Total documents: " << document_count << endl;
	cout << "htmerge: Total doc db size (in K): ";
	cout << docdb_size / 1024 << endl;
      }

    delete IDs;
    db.Close();
}


