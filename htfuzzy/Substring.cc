//
// Substring.cc
//
// Substring: The substring fuzzy algorithm. Currently a rather slow, naive approach
//            that checks the substring against every word in the word db.
//            It does not generate a separate database.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Substring.cc,v 1.8 1999/09/10 17:22:25 ghutchis Exp $
//

#include "Substring.h"
#include "htString.h"
#include "List.h"
#include "StringMatch.h"
#include "Configuration.h"

extern Configuration	config;


//*****************************************************************************
// Substring::Substring()
//
Substring::Substring()
{
    name = "substring";
}


//*****************************************************************************
// Substring::~Substring()
//
Substring::~Substring()
{
}


//*****************************************************************************
// A very simplistic and inefficient substring search.  For every word
// that is looked for we do a complete linear search through the word
// database.
// Maybe a better method of doing this would be to mmap a list of words
// to memory and then run the StringMatch on it.  It would still be a
// linear search, but with much less overhead.
//
void
Substring::getWords(char *w, List &words)
{
    // First strip the punctuation
    String	stripped = w;
    HtStripPunctuation(stripped);

    // Now set up the StringMatch object
    StringMatch	match;
    match.Pattern(stripped);

    // And get the list of all possible words
    WordList	wordDB;
    List	*wordList;
    String	*key;
    wordDB.Read(config["word_db"]);
    wordList = wordDB.Words();

    int		wordCount = 0;
    int		maximumWords = config.Value("substring_max_words", 25);

    wordList->Start_Get();
    while (wordCount < maximumWords && (key = (String *) wordList->Get_Next()))
    {
	if (match.FindFirst(*key) >= 0)
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
Substring::openIndex(Configuration &)
{
  return 0;
}


//*****************************************************************************
void
Substring::generateKey(char *, String &)
{
}


//*****************************************************************************
void
Substring::addWord(char *)
{
}




