//
// htmerge.cc
//
// htmerge: Merges two databases and/or updates databases to remove 
//          old documents and ensures the databases are consistent.
//          Calls db.cc, docs.cc, and/or words.cc as necessary
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU General Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: htmerge.cc,v 1.1.2.2 2000/06/21 01:27:52 ghutchis Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "WordContext.h"
#include "good_strtok.h"
#include "defaults.h"
#include "DocumentDB.h"
#include "HtURLCodec.h"
#include "HtWordList.h"
#include "HtWordReference.h"
#include "htString.h"

#include <fstream.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

// If we have this, we probably want it.
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

class CallbackData : public Object
{
public:
  CallbackData(HtWordList * w, Dictionary * d, int o)
        { word_db = w; dup_ids = d; docIDOffset = o; }

  HtWordList	*word_db;
  Dictionary	*dup_ids;
  int		docIDOffset;
};

int
OverrideCallback(WordList * wl,
                 WordDBCursor &,
                 const WordReference * w,
                 Object & d)
{
  CallbackData		&data = ((CallbackData &)d);
  HtWordReference	*ht_wr = (HtWordReference *)w;
  String		docIDKey;

  docIDKey << ht_wr->DocID();
  if (!((data.dup_ids)->Exists(docIDKey)))
    {
      ht_wr->DocID(ht_wr->DocID() + data.docIDOffset);
      (data.word_db)->Override(*ht_wr);
    }
    
  return OK;
}

int
DeleteCallback(WordList * wl,
               WordDBCursor &,
               const WordReference * w,
               Object & d)
{
  CallbackData		&data = ((CallbackData &)d);
  HtWordReference	*ht_wr = (HtWordReference *)w;
  String		docIDKey;

  docIDKey << ht_wr->DocID();
  if ((data.dup_ids)->Exists(docIDKey))
    (data.word_db)->Delete(*ht_wr);
    
  return OK;
}


// This config is used for merging multiple databses
HtConfiguration    merge_config;

int		verbose = 0;
int		stats = 0;

// Component procedures
void mergeDB();
void usage();
void reportError(char *msg);

//*****************************************************************************
// int main(int ac, char **av)
//
int main(int ac, char **av)
{
    int			alt_work_area = 0;
    String		configfile = DEFAULT_CONFIG_FILE;
    String              merge_configfile = 0;
    int			c;
    extern char		*optarg;

    while ((c = getopt(ac, av, "svm:c:dwa")) != -1)
    {
	switch (c)
	{
	    case 'd':
		break;
	    case 'w':
		break;
	    case 'c':
		configfile = optarg;
		break;
	    case 'm':
	      	merge_configfile = optarg;
	      	break;
	    case 'v':
		verbose++;
		break;
	    case 's':
		break;
	    case 'a':
		alt_work_area++;
		break;
	    case '?':
		usage();
		break;
	}
    }

    config.Defaults(&defaults[0]);

    if (access((char*)configfile, R_OK) < 0)
    {
	reportError(form("Unable to find configuration file '%s'",
			 configfile.get()));
    }
	
    config.Read(configfile);

    //
    // Check url_part_aliases and common_url_parts for
    // errors.
    String url_part_errors = HtURLCodec::instance()->ErrMsg();

    if (url_part_errors.length() != 0)
      reportError(form("Invalid url_part_aliases or common_url_parts: %s",
                       url_part_errors.get()));

    if (merge_configfile.length())
    {
    	merge_config.Defaults(&defaults[0]);
	if (access((char*)merge_configfile, R_OK) < 0)
    	{
	reportError(form("Unable to find configuration file '%s'",
			 merge_configfile.get()));
    	}
	merge_config.Read(merge_configfile);
    }

    if (alt_work_area != 0)
    {
	String	configValue;

	configValue = config["word_db"];
	if (configValue.length() != 0)
	{
	    configValue << ".work";
	    config.Add("word_db", configValue);
	}

	configValue = config["doc_db"];
	if (configValue.length() != 0)
	{
	    configValue << ".work";
	    config.Add("doc_db", configValue);
	}

	configValue = config["doc_index"];
	if (configValue.length() != 0)
	{
	    configValue << ".work";
	    config.Add("doc_index", configValue);
	}

	configValue = config["doc_excerpt"];
	if (configValue.length() != 0)
	{
	    configValue << ".work";
	    config.Add("doc_excerpt", configValue);
	}
    }

    WordContext::Initialize(config);

    if (merge_configfile.length())
    {
	// Merge the databases specified in merge_configfile into the current
	// databases. Do this first then update the other databases as usual
	// Note: We don't have to specify anything, it's all in the config vars

	mergeDB();
    }

    return 0;
}

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
        
    CallbackData data(&wordDB, &merge_dup_ids, docIDOffset);
    WordSearchDescription description(OverrideCallback, (Object *)&data);
    mergeWordDB.Walk(description);

    CallbackData data(&wordDB, &db_dup_ids, 0);
    WordSearchDescription description(DeleteCallback, (Object *)&data);
    wordDB.Walk(description);

    // Cleanup--just close the two word databases
    mergeWordDB.Close();
    wordDB.Close();
}


//*****************************************************************************
// void usage()
//   Display program usage information
//
void usage()
{
    cout << "usage: htmerge [-v][-c configfile][-m merge_configfile]\n";
    cout << "This program is part of ht://Dig " << VERSION << "\n\n";
    cout << "Options:\n";
    cout << "\t-v\tVerbose mode.  This increases the verbosity of the\n";
    cout << "\t\tprogram.  Using more than 2 is probably only useful\n";
    cout << "\t\tfor debugging purposes.  The default verbose mode\n";
    cout << "\t\tgives a progress on what it is doing and where it is.\n\n";
    cout << "\t-m merge_configfile\n";
    cout << "\t\tMerge the databases specified into the databases specified\n";
    cout << "\t\tby -c or the default.\n\n";
    cout << "\t-c configfile\n";
    cout << "\t\tUse the specified configuration file instead on the\n";
    cout << "\t\tdefault.\n\n";
    cout << "\t-a\tUse alternate work files.\n";
    cout << "\t\tTells htmerge to append .work to database files causing\n";
    cout << "\t\ta second copy of the database to be built.  This allows\n";
    cout << "\t\toriginal files to be used by htsearch during the indexing\n";
    cout << "\t\trun.\n\n";
    exit(0);
}


//*****************************************************************************
// Report an error and die
//
void reportError(char *msg)
{
    cout << "htmerge: " << msg << "\n\n";
    exit(1);
}
