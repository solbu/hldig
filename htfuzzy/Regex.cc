//
// Regex.cc
//
// Regex: A fuzzy to match input regex against the word database.
//        Based on the substring fuzzy
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Regex.cc,v 1.7.2.2 2000/05/06 20:46:38 loic Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include <fcntl.h>

#include "Regex.h"
#include "htString.h"
#include "List.h"
#include "StringMatch.h"
#include "HtConfiguration.h"

//*****************************************************************************
// Regex::Regex(const HtConfiguration& config_arg)
//
Regex::Regex(const HtConfiguration& config_arg) :
  Fuzzy(config_arg)
{
    name = "regex";
}


//*****************************************************************************
// Regex::~Regex()
//
Regex::~Regex()
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
Regex::getWords(char *, List &words)
{
    HtRegex	regexMatch;
    String	stripped;

    // First we have to strip the necessary punctuation
    stripped.remove("^.[]$()|*+?{},-\\");

    // Anchor the string to be matched
    regexMatch.set(String("^") + stripped);

    HtWordList    wordDB(config);
    List        *wordList;
    String	*key;
    wordDB.Open(config["word_db"], O_RDONLY);
    wordList = wordDB.Words();

    int         wordCount = 0;
    int         maximumWords = config.Value("regex_max_words", 25);

    wordList->Start_Get();
    while (wordCount < maximumWords && (key = (String *) wordList->Get_Next()))
      {
        if (regexMatch.match(*key, 0, 0) != 0)
	  {
            words.Add(new String(*key));
            wordCount++;
	  }
      }
    if (wordList) {
      wordList->Destroy();
      delete wordList;
    }
    wordDB.Close();
}


//*****************************************************************************
int
Regex::openIndex()
{
  return 0;
}


//*****************************************************************************
void
Regex::generateKey(char *, String &)
{
}


//*****************************************************************************
void
Regex::addWord(char *)
{
}




