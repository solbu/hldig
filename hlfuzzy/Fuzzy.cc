//
// Fuzzy.cc
//
// Fuzzy: This is the base class for all the different types of fuzzy searches.
//        We only define the interface.
//
// There are two main uses of classes derived from this class:
//    1) Creation of a fuzzy index
//    2) Searching for a word using the fuzzy index
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Fuzzy.cc,v 1.20 2004/05/28 13:15:20 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "hlconfig.h"
#endif /* HAVE_CONFIG_H */

#include <fcntl.h>

#include "Fuzzy.h"
#include "hlfuzzy.h"
#include "HtConfiguration.h"
#include "List.h"
#include "StringList.h"
#include "Endings.h"
#include "Exact.h"
#include "Metaphone.h"
#include "Soundex.h"
#include "Synonym.h"
#include "Substring.h"
#include "Prefix.h"
#include "Regexp.h"
#include "Speling.h"
#include "Accents.h"

//*****************************************************************************
// Fuzzy::Fuzzy(const HtConfiguration& config)
//
Fuzzy::Fuzzy (const HtConfiguration & config_arg):
config (config_arg)
{
  dict = 0;
  index = 0;
}


//*****************************************************************************
// Fuzzy::~Fuzzy()
//
Fuzzy::~Fuzzy ()
{
  if (index)
  {
    index->Close ();
    delete index;
    index = 0;
  }
  delete dict;
}


//*****************************************************************************
// void Fuzzy::getWords(char *word, List &words)
//
void
Fuzzy::getWords (char *word, List & words)
{
  if (!index)
    return;
  if (!word || !*word)
    return;

  //
  // Convert the word to a fuzzy key
  //
  String fuzzyKey;
  String data;
  String stripped = word;
  HtStripPunctuation (stripped);
  generateKey (stripped, fuzzyKey);
  if (debug > 2)
    cout << "\n\tkey: " << fuzzyKey << endl;

  words.Destroy ();

  if (index->Get (fuzzyKey, data) == OK)
  {
    //
    // Found the entry
    //
    char *token = strtok (data.get (), " ");
    while (token)
    {
      if (mystrcasecmp (token, word) != 0)
      {
        words.Add (new String (token));
      }
      token = strtok (0, " ");
    }
  }
  else
  {
    //
    // The key wasn't found.
    //
  }
}


//*****************************************************************************
// int Fuzzy::openIndex(const HtConfiguration &config)
//
int
Fuzzy::openIndex ()
{
  String var = name;
  var << "_db";
  const String filename = config[var];

  index = Database::getDatabaseInstance (DB_HASH);
  if (index->OpenRead (filename) == NOTOK)
  {
    delete index;
    index = 0;
    return NOTOK;
  }

  return OK;
}


//*****************************************************************************
// int Fuzzy::writeDB(HtConfiguration &config)
//
int
Fuzzy::writeDB ()
{
  String var = name;
  var << "_db";
  const String filename = config[var];

  index = Database::getDatabaseInstance (DB_HASH);
  if (index->OpenReadWrite (filename, 0664) == NOTOK)
    return NOTOK;

  String *s;
  char *fuzzyKey;

  int count = 0;

  dict->Start_Get ();
  while ((fuzzyKey = dict->Get_Next ()))
  {
    s = (String *) dict->Find (fuzzyKey);
    index->Put (fuzzyKey, *s);

    if (debug > 1)
    {
      cout << "hlfuzzy: '" << fuzzyKey << "' ==> '" << s->get () << "'\n";
    }
    count++;
    if ((count % 100) == 0 && debug == 1)
    {
      cout << "hlfuzzy: keys: " << count << '\n';
      cout.flush ();
    }
  }
  if (debug == 1)
  {
    cout << "hlfuzzy:Total keys: " << count << "\n";
  }
  return OK;
}


//*****************************************************************************
// Fuzzy algorithm factory.
//
Fuzzy *
Fuzzy::getFuzzyByName (char *name, const HtConfiguration & config)
{
  if (mystrcasecmp (name, "exact") == 0)
    return new Exact (config);
  else if (mystrcasecmp (name, "soundex") == 0)
    return new Soundex (config);
  else if (mystrcasecmp (name, "metaphone") == 0)
    return new Metaphone (config);
  else if (mystrcasecmp (name, "accents") == 0)
    return new Accents (config);
  else if (mystrcasecmp (name, "endings") == 0)
    return new Endings (config);
  else if (mystrcasecmp (name, "synonyms") == 0)
    return new Synonym (config);
  else if (mystrcasecmp (name, "substring") == 0)
    return new Substring (config);
  else if (mystrcasecmp (name, "prefix") == 0)
    return new Prefix (config);
  else if (mystrcasecmp (name, "regex") == 0)
    return new Regexp (config);
  else if (mystrcasecmp (name, "speling") == 0)
    return new Speling (config);
  else
    return 0;
}

//*****************************************************************************
int
Fuzzy::createDB (const HtConfiguration &)
{
  return OK;
}

void
Fuzzy::generateKey (char *, String &)
{
}


void
Fuzzy::addWord (char *)
{
}
