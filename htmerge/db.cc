//
// db.cc
//
// db: Implementation of merging databases. Uses two config files to 
//     specify which sets of databases to merge. Only adds the data in, 
//     assumes mergeWords and convertDocs are performed to ensure 
//     database integrity.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: db.cc,v 1.22.2.1 2000/02/14 06:05:23 ghutchis Exp $
//

#include "htmerge.h"
#include "good_strtok.h"

//*****************************************************************************
// void mergeDB()
//
void
mergeDB()
{
    DocumentDB	merge_db, db;
    List	*urls;
    Dictionary  merge_dup_ids, db_dup_ids; // Lists of DocIds to ignore
    int         docIDOffset;

    const String doc_index = config["doc_index"];
    if (access(doc_index, R_OK) < 0)
    {
	reportError(form("Unable to open document index '%s'", (const char*)doc_index));
    }
    const String doc_excerpt = config["doc_excerpt"];
    if (access(doc_excerpt, R_OK) < 0)
    {
	reportError(form("Unable to open document excerpts '%s'", (const char*)doc_excerpt));
    }
    const String doc_db = config["doc_db"];    
    if (db.Open(doc_db, doc_index, doc_excerpt) < 0)
    {
	reportError(form("Unable to open/create document database '%s'",
			 (const char*)doc_db));
    }


    const String merge_doc_index = merge_config["doc_index"];    
    if (access(merge_doc_index, R_OK) < 0)
    {
	reportError(form("Unable to open document index '%s'", (const char*)merge_doc_index));
    }
    const String merge_doc_excerpt = merge_config["doc_excerpt"];    
    if (access(merge_doc_excerpt, R_OK) < 0)
    {
	reportError(form("Unable to open document excerpts '%s'", (const char*)merge_doc_excerpt));
    }
    const String merge_doc_db = merge_config["doc_db"];
    if (merge_db.Open(merge_doc_db, merge_doc_index, merge_doc_excerpt) < 0)
    {
	reportError(form("Unable to open document database '%s'",
			 (const char*)merge_doc_db));
    }

    // Start the merging by going through all the URLs that are in
    // the database to be merged
        
    urls = merge_db.URLs();
    // This ensures that every document added from merge_db has a unique ID
    // in the new database
    docIDOffset = db.NextDocID();

    urls->Start_Get();
    String		*url;
    String		id;
    while ((url = (String *) urls->Get_Next()))
    {
	DocumentRef	*ref = merge_db[url->get()];
	DocumentRef     *old_ref = db[url->get()];
	if (!ref)
	    continue;

	if (old_ref)
	  {
	    // Oh well, we knew this would happen. Let's get the duplicate
	    // And we'll only use the most recent date.

	    if ( old_ref->DocTime() > ref->DocTime() )
	      {
		// Cool, the ref we're merging is too old, just ignore it
		char        str[20];
		sprintf(str, "%d", ref->DocID());
		merge_dup_ids.Add(str, 0);
		
		if (verbose > 1)
		  {
		    cout << "htmerge: Duplicate, URL: " << url << " ignoring merging copy   \n";
		    cout.flush();
		  }
	      }
	    else
	      {
		// The ref we're merging is newer, delete the old one and add
		char        str[20];
		sprintf(str, "%d", old_ref->DocID());
		db_dup_ids.Add(str, 0);
		db.Delete(old_ref->DocID());
		ref->DocID(ref->DocID() + docIDOffset);
		db.Add(*ref);
                if (verbose > 1)
                  {
                    cout << "htmerge: Duplicate, URL: ";
		    cout << url->get() << " ignoring destination copy   \n";
                    cout.flush();
                  }
	      }
	  }
	else
	  {
	    // It's a new URL, just add it, making sure to load the excerpt
	    merge_db.ReadExcerpt(*ref);
	    ref->DocID(ref->DocID() + docIDOffset);
	    db.Add(*ref);
	    if (verbose > 1)
	      {
		cout << "htmerge: Merged URL: " << url->get() << "    \n";
		cout.flush();
	      }
	  }
        delete ref;
	delete old_ref;
    }    
    delete urls;
    
    // As reported by Roman Dimov, we must update db.NextDocID()
    // because of all the added records...
    db.IncNextDocID( merge_db.NextDocID() );
    merge_db.Close();
    db.Close();

    // OK, after merging the doc DBs, we do the same for the words

    HtWordList	mergeWordDB(config), wordDB(config);
    List	*words;
    String	docIDKey;

    if (wordDB.Open(config["word_db"], O_RDWR) < 0)
    {
	reportError(form("Unable to open/create document database '%s'",
			 (const char*)config["word_db"]));
    }

    if (mergeWordDB.Open(merge_config["word_db"], O_RDONLY) < 0)
    {
	reportError(form("Unable to open document database '%s'",
			 (const char *)merge_config["word_db"]));
    }

    // Start the merging by going through all the URLs that are in
    // the database to be merged
        
    words = mergeWordDB.WordRefs();

    words->Start_Get();
    HtWordReference	*word;
    while ((word = (HtWordReference *) words->Get_Next()))
    {
      docIDKey = word->DocID();
      if (merge_dup_ids.Exists(docIDKey))
	continue;

      word->DocID(word->DocID() + docIDOffset);
      wordDB.Override(*word);
    }
    delete words;

    words = wordDB.WordRefs();
    words->Start_Get();
    while ((word = (HtWordReference *) words->Get_Next()))
    {
      docIDKey = word->DocID();
      if (db_dup_ids.Exists(docIDKey))
	wordDB.Delete(*word);
    }
    delete words;
    
    // Cleanup--just close the two word databases
    mergeWordDB.Close();
    wordDB.Close();
}
