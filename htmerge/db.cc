//
// db.cc
//
// Implementation of merging databases. Uses two config files to specify which
// sets of databases to merge. Only adds the data in, assumes mergeWords and 
// convertDocs are performed to ensure database integrity.
//

#include "htmerge.h"

//*****************************************************************************
// void mergeDB()
//
void
mergeDB()
{
    DocumentDB	merge_db, db;
    List	*urls;
    Dictionary  merge_dup_ids, db_dup_ids; // Lists of DocIds to ignore
    char        *doc_db, *merge_doc_db;
    char        *doc_index, *merge_doc_index;
    char	*doc_excerpt, *merge_doc_excerpt;
    int         docIDOffset;

    // Check "uncompressed"/"uncoded" urls at the price of time
    // (extra DB probes).
    db.SetCompatibility(config.Boolean("uncoded_db_compatible", 1));

    doc_index = config["doc_index"];
    if (access(doc_index, R_OK) < 0)
    {
	reportError(form("Unable to open document index '%s'", doc_index));
    }
    doc_excerpt = config["doc_excerpt"];
    if (access(doc_excerpt, R_OK) < 0)
    {
	reportError(form("Unable to open document excerpts '%s'", doc_excerpt));
    }
    doc_db = config["doc_db"];    
    if (db.Open(doc_db, doc_index, doc_excerpt) < 0)
    {
	reportError(form("Unable to open/create document database '%s'",
			 doc_db));
    }

    merge_db.
      SetCompatibility(merge_config.Boolean("uncoded_db_compatible", 1));

    merge_doc_index = merge_config["doc_index"];    
    if (access(merge_doc_index, R_OK) < 0)
    {
	reportError(form("Unable to open document index '%s'", merge_doc_index));
    }
    merge_doc_excerpt = merge_config["doc_excerpt"];    
    if (access(merge_doc_excerpt, R_OK) < 0)
    {
	reportError(form("Unable to open document excerpts '%s'", merge_doc_excerpt));
    }
    merge_doc_db = merge_config["doc_db"];
    if (merge_db.Open(merge_doc_db, merge_doc_index, merge_doc_excerpt) < 0)
    {
	reportError(form("Unable to open document database '%s'",
			 merge_doc_db));
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
	    // it's a new URL, just add it
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

    //
    // Now we go through the original wordlist and remove deleted docIDs
    //

    char       *wordtmp = config["word_list"];
    FILE       *wordlist = fopen(form("%s.new", wordtmp), "w");
    FILE       *dbwords = fopen(wordtmp, "r");
    char        buffer[1000];
    String      word;
    char       *sid;
    char       *name, *value, *pair;
    WordRecord  wr;

    // Check for file access errors
    if (!wordlist)
      {
        reportError(form("Unable to create temporary word file '%s.new'",
                         wordtmp));
      }
    if (!dbwords)
      {
        reportError(form("Unable to open word list file '%s'", wordtmp));
      }

    // Read it in a line at a time...
    while (fgets(buffer, sizeof(buffer), dbwords))
      {
	// Split the line up into the word, count, location, and
	// document id, just like in words.cc(mergeWords).
	word = good_strtok(buffer, '\t');
	pair = good_strtok(NULL, '\t');
	wr.Clear();   // Reset count to 1, anchor to 0, and all that
	sid = "-";
	while (pair && *pair)
	  {
	    name = strtok(pair, ":");
	    value = strtok(0, "\n");
	    if (name && *name && value && *value)
	      {
		switch (*name)
		  {
#ifndef NO_WORD_COUNT
		  case 'c':
		    wr.count = atoi(value);
		    break;
#endif
		  case 'l':
		    wr.location = atoi(value);
		    break;
		  case 'i':
		    sid = value;
		    wr.id = atoi(value);
		    break;
		  case 'w':
		    wr.weight = atoi(value);
		    break;
		  case 'a':
		    wr.anchor = atoi(value);
		    break;
		  }
	      }
	    pair = good_strtok(NULL, '\t');
	  }

	// OK, now we have to check if this word was from a doc we discarded.
	if (db_dup_ids.Exists(sid))
	  {
	    if (verbose > 1)
	      {
		cout << "htmerge: Discarding duplicate " << word << " in doc #"
		     << sid << "     \n";
		cout.flush();
	      }
	    continue;
	  }

	// Record the word in the new file
	fprintf(wordlist, "%s", word.get());
#ifndef NO_WORD_COUNT
	if (wr.count != 1)
	  {
	    fprintf(wordlist, "\tc:%d", wr.count);
	  }
#endif
	fprintf(wordlist, "\tl:%d\ti:%d\tw:%d",
		wr.location,
		wr.id,
		wr.weight);
	if (wr.anchor != 0)
	  {
	    fprintf(wordlist, "\ta:%d", wr.anchor);
	  }
	putc('\n', wordlist);
      }
    fclose(dbwords);
    db_dup_ids.Destroy(); // Save some memory

    // Now wordlist is at the end of its current stream, so we're set to write
    // the new merged data.

    FILE *mergewords = fopen(merge_config["word_list"], "r");

    // Check for file access errors
    if (!mergewords)
      {
        reportError(form("Unable to open word list file '%s'", merge_config["word_list"]));
      }

    // Read it in a line at a time...
    while (fgets(buffer, sizeof(buffer), mergewords))
      {
if (*buffer == '+') {
	    // This will prevent removing documents from main
	    // config db with id of bad documents from merged config
	    // It was happened when I run htmerge -m test1.conf -c test.conf
	    // without previous runned htmerge -c test1.conf, which
	    // normally must remove strings +ID, -ID, !ID from wordlist
	    //
 fprintf(wordlist, "+%d\n", atoi(strtok(buffer + 1, "\n"))+docIDOffset);
} else if (*buffer == '-') {
 fprintf(wordlist, "-%d\n", atoi(strtok(buffer + 1, "\n"))+docIDOffset);
} else if (*buffer == '!') {
 fprintf(wordlist, "!%d\n", atoi(strtok(buffer + 1, "\n"))+docIDOffset);
} else {
	// Split the line up into the word, count, location, and
	// document id, just like in words.cc(mergeWords).
	word = good_strtok(buffer, '\t');
	pair = good_strtok(NULL, '\t');
	wr.Clear();   // Reset count to 1, anchor to 0, and all that
	sid = "-";
	while (pair && *pair)
	  {
	    name = strtok(pair, ":");
	    value = strtok(0, "\n");
	    if (name && *name && value && *value)
	      {
		switch (*name)
		  {
#ifndef NO_WORD_COUNT
		  case 'c':
		    wr.count = atoi(value);
		    break;
#endif
		  case 'l':
		    wr.location = atoi(value);
		    break;
		  case 'i':
		    sid = value;
		    wr.id = atoi(value);
		    break;
		  case 'w':
		    wr.weight = atoi(value);
		    break;
		  case 'a':
		    wr.anchor = atoi(value);
		    break;
		  }
	      }
	    pair = good_strtok(NULL, '\t');
	  }

	// OK, now we have to check if this word was from a doc we discarded.
	if (merge_dup_ids.Exists(sid))
	  {
	    if (verbose > 1)
	      {
		cout << "htmerge: Discarding merged duplicate " << word << " in doc #"
		     << sid << "     \n";
		cout.flush();
	      }
	    continue;
	  }

	// Record the word in the new file
	fprintf(wordlist, "%s", word.get());
#ifndef NO_WORD_COUNT
	if (wr.count != 1)
	  {
	    fprintf(wordlist, "\tc:%d", wr.count);
	  }
#endif
	fprintf(wordlist, "\tl:%d\ti:%d\tw:%d",
		wr.location,
		wr.id + docIDOffset,
		wr.weight);
	if (wr.anchor != 0)
	  {
	    fprintf(wordlist, "\ta:%d", wr.anchor);
	  }
	putc('\n', wordlist);
      }
      }
    fclose(mergewords);

    // Deal with the new wordlist file.  We need to replace the old file with
    // the new one.
    fclose(wordlist);
    unlink(wordtmp);
    link(form("%s.new", wordtmp), wordtmp);
    unlink(form("%s.new", wordtmp));
}
