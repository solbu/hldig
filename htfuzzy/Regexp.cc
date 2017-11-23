//
// Regexp.cc
//
// Regexp: A fuzzy to match input regex against the word database.
//        Based on the substring fuzzy
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later 
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: Regexp.cc,v 1.5 2004/05/28 13:15:20 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <fcntl.h>

#include "Regexp.h"
#include "htString.h"
#include "List.h"
#include "StringMatch.h"
#include "HtConfiguration.h"

//*****************************************************************************
// Regexp::Regexp(const HtConfiguration& config_arg)
//
Regexp::Regexp (const HtConfiguration & config_arg):
Fuzzy (config_arg)
{
  name = "regex";
}


//*****************************************************************************
// Regexp::~Regexp()
//
Regexp::~Regexp ()
{
}


//*****************************************************************************
// A very simplistic and inefficient regex search.  For every word
// that is looked for we do a complete linear search through the word
// database.
// Maybe a better method of doing this would be to mmap a list of words
// to memory and then run the regex on it.  It would still be a
// linear search, but with much less overhead.
//
void
Regexp::getWords (char *pattern, List & words)
{
  HtRegex regexMatch;
  String stripped (pattern);

  // First we have to strip the necessary punctuation
// Why??  lha
//    stripped.remove("^.[]$()|*+?{},-\\");

  // Anchor the string to be matched
  regexMatch.set (String ("^") + stripped);

  HtWordList wordDB (config);
  List *wordList;
  String *key;
  wordDB.Open (config["word_db"], O_RDONLY);
  wordList = wordDB.Words ();

  int wordCount = 0;
  int maximumWords = config.Value ("regex_max_words", 25);

  wordList->Start_Get ();
  while (wordCount < maximumWords && (key = (String *) wordList->Get_Next ()))
  {
    if (regexMatch.match (*key, 0, 0) != 0)
    {
      words.Add (new String (*key));
      wordCount++;
    }
  }
  if (wordList)
  {
    wordList->Destroy ();
    delete wordList;
  }
  wordDB.Close ();
}


//*****************************************************************************
int
Regexp::openIndex ()
{
  return 0;
}


//*****************************************************************************
void
Regexp::generateKey (char *, String &)
{
}


//*****************************************************************************
void
Regexp::addWord (char *)
{
}
