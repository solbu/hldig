//
// htfuzzy.cc
//
// htfuzzy: Create one or more ``fuzzy'' indexes into the main word database.
//          These indexes can be used by htsearch to perform a search that uses
//          other algorithms than exact word match.
//
//  This program is meant to be run after htmerge has created the word
//  database.
//
//  For each fuzzy algorithm, there will be a separate database.  Each
//  database is simply a mapping from the fuzzy key to a list of words
//  in the main word database.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2000 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: htfuzzy.cc,v 1.17 2002/02/01 22:49:33 ghutchis Exp $
//
#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "htfuzzy.h"
#include "Fuzzy.h"
#include "Accents.h"
#include "Soundex.h"
#include "Endings.h"
#include "Metaphone.h"
#include "Synonym.h"
#include "htString.h"
#include "List.h"
#include "Dictionary.h"
#include "defaults.h"
#include "HtWordList.h"
#include "WordContext.h"

// If we have this, we probably want it.
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

int		debug = 0;

void usage();


//*****************************************************************************
// int main(int ac, char **av)
//
int
main(int ac, char **av)
{
    int			c, i;
    extern char	*optarg;
    extern int	optind;
    String		configFile = DEFAULT_CONFIG_FILE;

    //
    // Parse command line arguments
    //
    while ((c = getopt(ac, av, "c:v")) != -1)
    {
	switch (c)
	{
	    case 'c':
		configFile = optarg;
		break;
				
	    case 'v':
		debug++;
		break;
				
	    default:
		usage();
	}
    }

	HtConfiguration* config= HtConfiguration::config();
    //
    // Determine what algorithms to use
    //
    List	wordAlgorithms;
    List	noWordAlgorithms;
    for (i = optind; i < ac; i++)
    {
	if (mystrcasecmp(av[i], "soundex") == 0)
	{
	    wordAlgorithms.Add(new Soundex(*config));
	}
	else if (mystrcasecmp(av[i], "metaphone") == 0)
	{
	    wordAlgorithms.Add(new Metaphone(*config));
	}
	else if (mystrcasecmp(av[i], "accents") == 0)
	{
	    wordAlgorithms.Add(new Accents(*config));
	}
	else if (mystrcasecmp(av[i], "endings") == 0)
	{
	    noWordAlgorithms.Add(new Endings(*config));
	}
	else if (mystrcasecmp(av[i], "synonyms") == 0)
	{
	    noWordAlgorithms.Add(new Synonym(*config));
	}
	else
	{
	    reportError(form("'%s' is not a supported algorithm",
			     av[i]));
	}
    }
    if (wordAlgorithms.Count() == 0 && noWordAlgorithms.Count() == 0)
    {
	cout << "htfuzzy: No algorithms specified\n";
	usage();
    }
	
    //
    // Find and parse the configuration file.
    //
    config->Defaults(&defaults[0]);
    if (access((char*)configFile, R_OK) < 0)
    {
	reportError(form("Unable to find configuration file '%s'",
			 configFile.get()));
    }
    config->Read(configFile);

    // Initialize htword library (key description + wordtype...)
    WordContext::Initialize(*config);

    Fuzzy	*fuzzy;
    if (wordAlgorithms.Count() > 0)
    {
        //
        // Open the word database so that we can grab the words from it.
        //
        HtWordList	worddb(*config);
	if (worddb.Open(config->Find("word_db"), O_RDONLY) == OK)
	  {
	    //
	    // Go through all the words in the database
	    //
	    List		*words = worddb.Words();
	    String		*key;
	    Fuzzy		*fuzzy = 0;
	    String		word, fuzzyKey;
	    int			count = 0;
	    
	    words->Start_Get();
	    while ((key = (String *) words->Get_Next()))
	      {
		word = *key;
		wordAlgorithms.Start_Get();
		while ((fuzzy = (Fuzzy *) wordAlgorithms.Get_Next()))
		  {
		    fuzzy->addWord(word);
		  }
		count++;
		if ((count % 100) == 0 && debug)
		  {
		    cout << "htfuzzy: words: " << count << '\n';
		    cout.flush();
		  }
	      }	
	    if (debug)
	      {
		cout << "htfuzzy: total words: " << count << "\n";
		cout << "htfuzzy: Writing index files...\n";
	      }
	    
	    //
	    // All the information is now in memory.
	    // Write all of it out to the individual databases
	    //
	    wordAlgorithms.Start_Get();
	    while ((fuzzy = (Fuzzy *) wordAlgorithms.Get_Next()))
	      {
		fuzzy->writeDB();
	      }
	    worddb.Close();
	    words->Destroy();
	    delete words;
	    if (fuzzy)
	      delete fuzzy;
	  }
	else
	  {
	    reportError(form("Unable to open word database %s", config->Find("word_db").get()));
	  }
    }
    if (noWordAlgorithms.Count() > 0)
    {
	noWordAlgorithms.Start_Get();
	while ((fuzzy = (Fuzzy *) noWordAlgorithms.Get_Next()))
	{
	    if (debug)
	    {
		cout << "htfuzzy: Selected algorithm: " << fuzzy->getName()
		     << endl;
	    }
	    if (fuzzy->createDB(*config) == NOTOK)
	      {
		cout << "htfuzzy: Could not create database for algorithm: "
		     << fuzzy->getName() << endl;
	      }
	}
    }
	
    if (debug)
    {
	cout << "htfuzzy: Done.\n";
    }
	
    return 0;
}


//*****************************************************************************
// void usage()
//
void
usage()
{
    cout << "usage: htfuzzy [-c configfile][-v] algorithm ...\n";
    cout << "This program is part of ht://Dig " << VERSION << "\n\n";
    cout << "Supported algorithms:\n";
    cout << "\tsoundex\n";
    cout << "\tmetaphone\n";
    cout << "\taccents\n";
    cout << "\tendings\n";
    cout << "\tsynonyms\n";
    cout << "\n";
	
    cout << "Options:\n";

    cout << "\t-c configfile\n";
    cout << "\t\tUse the specified configuration file instead of the\n";
    cout << "\t\tdefault.\n\n";

    cout << "\t-v\tVerbose mode.  This increases the verbosity of the\n";
    cout << "\t\tprogram.  Using more than 2 is probably only useful\n";
    cout << "\t\tfor debugging purposes.\n\n";

    exit(0);
}


//*****************************************************************************
// void reportError(char *msg)
//
void
reportError(char *msg)
{
    cout << "htfuzzy: " << msg << "\n\n";
    exit(1);
}


