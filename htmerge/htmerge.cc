//
// htmerge.cc
//
// htmerge: Merges two databases and/or updates databases to remove 
//          old documents and ensures the databases are consistent.
//          Calls db.cc, docs.cc, and/or words.cc as necessary
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: htmerge.cc,v 1.17 1999/10/08 12:59:56 loic Exp $
//

#include "htmerge.h"

// If we have this, we probably want it.
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

//
// This hash is used to keep track of all the document IDs which have to be
// discarded.
// This is generated from the doc database and is used to prune words
// from the word db
//
Dictionary	discard_list;

// This config is used for merging multiple databses
Configuration    merge_config;

int		verbose = 0;
int		stats = 0;

void usage();
void reportError(char *msg);

//*****************************************************************************
// int main(int ac, char **av)
//
int main(int ac, char **av)
{
    int			do_words = 1;
    int			do_docs = 1;
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
		do_docs = 0;
		break;
	    case 'w':
		do_words = 0;
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
		stats++;
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
    
    if (merge_configfile.length())
    {
	// Merge the databases specified in merge_configfile into the current
	// databases. Do this first then update the other databases as usual
	// Note: We don't have to specify anything, it's all in the config vars

	mergeDB();
    }
    if (do_docs)
    {
        // Update the document database, removing broken URLs, documents
        // with no stored information, etc.
	convertDocs();
    }
    if (do_words)
    {
        // Now that we have a list of deleted documents, remove the words
        // that were indexed from those documents
	mergeWords();
    }
    return 0;
}


//*****************************************************************************
// void usage()
//   Display program usage information
//
void usage()
{
    cout << "usage: htmerge [-v][-d][-w][-c configfile][-m merge_configfile]\n";
    cout << "This program is part of ht://Dig " << VERSION << "\n\n";
    cout << "Options:\n";
    cout << "\t-v\tVerbose mode.  This increases the verbosity of the\n";
    cout << "\t\tprogram.  Using more than 2 is probably only useful\n";
    cout << "\t\tfor debugging purposes.  The default verbose mode\n";
    cout << "\t\tgives a progress on what it is doing and where it is.\n\n";
    cout << "\t-d\tDo NOT merge the document database.\n\n";
    cout << "\t-w\tDo NOT merge the word database.\n\n";
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
