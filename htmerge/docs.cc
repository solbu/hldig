//
// docs.cc
//
// docs: Do sanity checking in "doc_db", remove insane documents.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: docs.cc,v 1.27.2.3 2000/05/10 18:23:45 loic Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "htmerge.h"


//*****************************************************************************
// void convertDocs()
//
void
convertDocs()
{
    const String	doc_db = config["doc_db"];
    const String	doc_index = config["doc_index"];
    const String	doc_excerpt = config["doc_excerpt"];
    int			remove_unused = config.Boolean("remove_bad_urls");
    int			remove_unretrieved = config.Boolean("remove_unretrieved_urls");
    DocumentDB		db;
    List		*IDs;
    int			document_count = 0;
    unsigned long	docdb_size = 0;

    //
    // Start the conversion by going through all the URLs that are in
    // the document database
    //
    if(db.Open(doc_db, doc_index, doc_excerpt) != OK)
      return;
    
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
	idStr = 0;
	idStr << id->Value();

	if (ref->DocState() == Reference_noindex)
	  {
	    // This document either wasn't found or shouldn't be indexed.
	    db.Delete(ref->DocID());
            if (verbose)
              cout << "Deleted, noindex ID: " << idStr << " URL: "
                   << url << endl;
	    discard_list.Add(idStr.get(), NULL);
	  }
	else if (ref->DocState() == Reference_obsolete)
	  {
	    // This document was replaced by a newer one
	    db.Delete(ref->DocID());
            if (verbose)
              cout << "Deleted, obsolete ID: " << idStr << " URL: "
                   << url << endl;
	    discard_list.Add(idStr.get(), NULL);
	  }
	else if (remove_unused && ref->DocState() == Reference_not_found)
	  {
	    // This document wasn't actually found
	    db.Delete(ref->DocID());
            if (verbose)
              cout << "Deleted, not found ID: " << idStr << " URL: "
                   << url << endl;
	    discard_list.Add(idStr.get(), NULL);
	  }
	else if (remove_unused && strlen(ref->DocHead()) == 0 
		 && ref->DocAccessed() != 0)
	  {
	    // For some reason, this document was retrieved, but doesn't
	    // have an excerpt (probably because of a noindex directive)
	    db.Delete(ref->DocID());
            if (verbose)
              cout << "Deleted, no excerpt ID: " << idStr << " URL:  "
                   << url << endl;
	    discard_list.Add(idStr.get(), NULL);
	  }
	else if (remove_unretrieved && ref->DocAccessed() == 0)
	  {
	    // This document has not been retrieved
	    db.Delete(ref->DocID());
            if (verbose)
              cout << "Deleted, never retrieved ID: " << idStr << " URL:  "
                   << url << endl;
	    discard_list.Add(idStr.get(), NULL);
	  }
	else
	  {
	    // This is a valid document. Let's keep stats on it.
            if (verbose > 1)
              cout << "ID: " << idStr << " URL: " << url << endl;

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


