//
// htfuzzy.cc
//
// Create one or more ``fuzzy'' indexes into the main word database.
// These indexes can be used by htsearch to perform a search that uses
// other algorithms than exact word match.
//
// This program is meant to be run after htmerge has created the word
// database.
//
// For each fuzzy algorithm, there will be a separate database.  Each
// database is simply a mapping from the fuzzy key to a list of words
// in the main word database.
//
// $Log: htfuzzy.cc,v $
// Revision 1.4  1998/09/18 02:38:08  ghutchis
//
// Bug fixes for 3.1.0b2
//
// Revision 1.3  1997/06/23 02:26:15  turtle
// Added version info the usage output
//
// Revision 1.2  1997/03/24 04:33:19  turtle
// Renamed the String.h file to htString.h to help compiling under win32
//
// Revision 1.1.1.1  1997/02/03 17:11:12  turtle
// Initial CVS
//
//
#if RELEASE
static char RCSid[] = "$Id: htfuzzy.cc,v 1.4 1998/09/18 02:38:08 ghutchis Exp $";
#endif

#include "htfuzzy.h"
#include "Fuzzy.h"
#include "Soundex.h"
#include "Endings.h"
#include "Metaphone.h"
#include "Synonym.h"
#include <htString.h>
#include <List.h>
#include <Dictionary.h>
#include <defaults.h>

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

    //
    // Determine what algorithms to use
    //
    List	wordAlgorithms;
    List	noWordAlgorithms;
    for (i = optind; i < ac; i++)
    {
	if (mystrcasecmp(av[i], "soundex") == 0)
	{
	    wordAlgorithms.Add(new Soundex);
	}
	else if (mystrcasecmp(av[i], "metaphone") == 0)
	{
	    wordAlgorithms.Add(new Metaphone);
	}
	else if (mystrcasecmp(av[i], "endings") == 0)
	{
	    noWordAlgorithms.Add(new Endings);
	}
	else if (mystrcasecmp(av[i], "synonyms") == 0)
	{
	    noWordAlgorithms.Add(new Synonym);
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
    config.Defaults(&defaults[0]);
    if (access(configFile, R_OK) < 0)
    {
	reportError(form("Unable to find configuration file '%s'",
			 configFile.get()));
    }
    config.Read(configFile);

    Fuzzy	*fuzzy;
    if (wordAlgorithms.Count() > 0)
    {
        //
        // Open the word database so that we can grab the words from it.
        //
        String  wordFile = config["word_db"];
        if (access(wordFile, R_OK) < 0)
        {
            reportError(form("Unable to read word database file '%s'\nDid you run htmerge?",
                             wordFile.get()));
        }
	Database *worddb = Database::getDatabaseInstance();
	if (worddb->OpenRead(wordFile) == OK)
	{
	    //
	    // Go through all the words in the database
	    //
	    char		*key;
	    Fuzzy		*fuzzy = 0;
	    String		word, fuzzyKey;
	    int			count = 0;

	    worddb->Start_Get();
	    while ((key = worddb->Get_Next()))
	    {
		word = key;
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
		fuzzy->writeDB(config);
	    }
	    worddb->Close();
	}
	else
	{
	    reportError("Unable to open word database");
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
	    fuzzy->createDB(config);
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
    cout << "This program is part of ht://Dig " << HTDIG_VERSION << "\n\n";
    cout << "Supported algorithms:\n";
    cout << "\tsoundex\n";
    cout << "\tmetaphone\n";
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


