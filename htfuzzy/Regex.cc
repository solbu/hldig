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
// $Id: Regex.cc,v 1.4 1999/09/10 17:22:25 ghutchis Exp $
//

#include "Regex.h"
#include "htString.h"
#include "List.h"
#include "StringMatch.h"
#include "Configuration.h"

extern Configuration	config;


//*****************************************************************************
// Regex::Regex()
//
Regex::Regex()
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
Regex::getWords(char *w, List &words)
{
    HtRegex	regexMatch;
    String	stripped;

    // Anchor the string to be matched
    stripped << '^' << w;

    // First we have to strip the necessary punctuation
    // So add regex reserved characters to extra_word_chars
    // (which we will restore later)

    String  configValue, savedConfig;
    configValue = config["extra_word_chars"];
    savedConfig = configValue;
    configValue << "^.[]$()|*+?{},-\\";
    config.Add("extra_word_chars", configValue);

    // Now we can strip anything remaining
    // and restore the saved extra_word_chars
    HtStripPunctuation(stripped);
    config.Add("extra_word_chars", savedConfig);

    regexMatch.set(stripped);

    WordList    wordDB;
    List        *wordList;
    String	*key;
    wordDB.Read(config["word_db"]);
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
Regex::openIndex(Configuration &)
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




