//
// Prefix.cc
//
// Prefix: The prefix fuzzy algorithm. Performs a O(log n) search on for words
//         matching the *prefix* specified--thus significantly faster than a full
//         substring search.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Prefix.cc,v 1.17 2004/05/28 13:15:20 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <fcntl.h>

#include "Prefix.h"
#include "htString.h"
#include "List.h"
#include "StringMatch.h"
#include "HtConfiguration.h"


//*****************************************************************************
// Prefix::Prefix(const HtConfiguration& config_arg)
//
Prefix::Prefix(const HtConfiguration& config_arg) :
  Fuzzy(config_arg)
{
    name = "prefix";
}


//*****************************************************************************
// Prefix::~Prefix()
//
Prefix::~Prefix()
{
}


//*****************************************************************************
//
//  Prefix search
//
void
Prefix::getWords(char *w, List &words)
{
    if (w == NULL || w[0] == '\0')
  return;

    String  stripped = w;
    HtStripPunctuation(stripped);
    w = stripped.get();

    const String  prefix_suffix = config["prefix_match_character"];
    int     prefix_suffix_length = prefix_suffix.length();
    int     minimum_prefix_length = config.Value("minimum_prefix_length");

    if (debug)
         cerr << " word=" << w << " prefix_suffix=" << prefix_suffix 
    << " prefix_suffix_length=" << prefix_suffix_length
    << " minimum_prefix_length=" << minimum_prefix_length << "\n";

    if ((int)strlen(w) < minimum_prefix_length + prefix_suffix_length)
  return;

    //  A null prefix character means that prefix matching should be 
    //  applied to every search word; otherwise return if the word does 
    //  not end in the prefix character(s).
    //
    if (prefix_suffix_length > 0
      && strcmp(prefix_suffix, w+strlen(w)-prefix_suffix_length)) 
  return;

    HtWordList  wordDB(config);
    if (wordDB.Open(config["word_db"], O_RDONLY) == NOTOK)
      return;

    int    wordCount = 0;
    int    maximumWords = config.Value("max_prefix_matches", 1000);
    String  s;
    int    len = strlen(w) - prefix_suffix_length;
    
    // Strip the prefix character(s)
    char w2[8192];
    strncpy(w2, w, sizeof(w2) - 1);
    w2[sizeof(w2) - 1] = '\0';
    w2[strlen(w2) - prefix_suffix_length] = '\0';
    String w3(w2);
    w3.lowercase();
    List    *wordList = wordDB.Prefix(w3.get());
    WordReference  *word_ref;
    String    last_word;

    wordList->Start_Get();
    while (wordCount < maximumWords && (word_ref = (WordReference *) wordList->Get_Next() ))
    {
  s = word_ref->Key().GetWord();

  // If we're somehow past the original word, we're done
  if (mystrncasecmp(s.get(), w, len))
      break;

  // If this is a duplicate word, ignore it
  if (last_word.length() != 0 && last_word == s)
      continue;

  last_word = s;
  words.Add(new String(s));
  wordCount++;
    }
    if (wordList) {
      wordList->Destroy();
      delete wordList;
    }
    wordDB.Close();
}


//*****************************************************************************
int
Prefix::openIndex()
{
  return 0;
}


//*****************************************************************************
void
Prefix::generateKey(char *, String &)
{
}


//*****************************************************************************
void
Prefix::addWord(char *)
{
}




