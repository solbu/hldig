//
// Regex.cc
//
// Implementation of the Regex fuzzy algorithm.
// Matches input regex against the word database. (Based on Substring.cc)
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Regex.cc,v 1.1 1999/05/05 00:38:53 ghutchis Exp $
//
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

    regexMatch.set(w);

    Database	*dbf = Database::getDatabaseInstance(DB_BTREE);
    dbf->OpenRead(config["word_db"]);

    int		wordCount = 0;
    int		maximumWords = config.Value("regex_max_words", 25);
    
    dbf->Start_Get();
    while (wordCount < maximumWords && (w = dbf->Get_Next()))
    {
        if (regexMatch.match(w, 0, 0) != 0)
	{
	    words.Add(new String(w));
	    wordCount++;
	}
    }
    dbf->Close();
    delete dbf;
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




