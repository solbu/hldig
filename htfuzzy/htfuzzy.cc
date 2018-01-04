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
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: htfuzzy.cc,v 1.20 2004/05/28 13:15:20 lha Exp $
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
#include "messages.h"

// If we have this, we probably want it.
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#elif HAVE_GETOPT_LOCAL
#include <getopt_local.h>
#endif

int    debug = 0;

void usage();


//*****************************************************************************
// int main(int ac, char **av)
//
int
main(int ac, char **av)
{
    int      c, i;
    extern char  *optarg;
    extern int  optind;
    String    configFile = DEFAULT_CONFIG_FILE;

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
    List  wordAlgorithms;
    List  noWordAlgorithms;
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
      reportError(form(_("'%s' is not a supported algorithm"),
           av[i]));
  }
    }
    if (wordAlgorithms.Count() == 0 && noWordAlgorithms.Count() == 0)
    {
  cout << _("hlfuzzy: No algorithms specified\n");
  usage();
    }

    //
    // Find and parse the configuration file.
    //
    config->Defaults(&defaults[0]);
    if (access((char*)configFile, R_OK) < 0)
    {
  reportError(form(_("Unable to find configuration file '%s'"),
       configFile.get()));
    }
    config->Read(configFile);

    // Initialize htword library (key description + wordtype...)
    WordContext::Initialize(*config);

    Fuzzy  *fuzzy;
    if (wordAlgorithms.Count() > 0)
    {
        //
        // Open the word database so that we can grab the words from it.
        //
        HtWordList  worddb(*config);
  if (worddb.Open(config->Find("word_db"), O_RDONLY) == OK)
    {
      //
      // Go through all the words in the database
      //
      List    *words = worddb.Words();
      String    *key;
      Fuzzy    *fuzzy = 0;
      String    word, fuzzyKey;
      int      count = 0;

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
        cout << "hlfuzzy: words: " << count << '\n';
        cout.flush();
      }
        }
      if (debug)
        {
    cout << _("hlfuzzy: total words: ") << count << "\n";
    cout << _("hlfuzzy: Writing index files...\n");
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
      reportError(form(_("Unable to open word database %s"), config->Find("word_db").get()));
    }
    }
    if (noWordAlgorithms.Count() > 0)
    {
  noWordAlgorithms.Start_Get();
  while ((fuzzy = (Fuzzy *) noWordAlgorithms.Get_Next()))
  {
      if (debug)
      {
    cout << _("hlfuzzy: Selected algorithm: ") << fuzzy->getName()
         << endl;
      }
      if (fuzzy->createDB(*config) == NOTOK)
        {
    cout << _("hlfuzzy: Could not create database for algorithm: ")
         << fuzzy->getName() << endl;
        }
  }
    }

    if (debug)
    {
  cout << _("hlfuzzy: Done.\n");
    }

    return 0;
}


//*****************************************************************************
// void usage()
//
void
usage()
{
    Usage help;
    cout << _("usage:");
    cout << " hlfuzzy [-c configfile][-v] algorithm ...\n";
    printf (_("This program is part of hl://Dig %s\n\n"), VERSION);
    cout << _("Supported algorithms:\n");
    cout << "\tsoundex\n";
    cout << "\tmetaphone\n";
    cout << "\taccents\n";
    cout << "\tendings\n";
    cout << "\tsynonyms\n";
    cout << "\n";

    cout << _("Options:\n");

    help.config ();
    help.verbose ();

    exit(0);
}


//*****************************************************************************
// void reportError(char *msg)
//
void
reportError(char *msg)
{
    cout << "hlfuzzy: " << msg << "\n\n";
    exit(1);
}


