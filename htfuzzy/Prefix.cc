//
// Prefix.cc
//
//: The prefix fuzzy algorithm. Performs a O(log n) search on for words
//  matching the *prefix* specified--thus significantly faster than a full
//  substring search.
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Prefix.cc,v 1.10 1999/09/10 01:37:39 ghutchis Exp $
//

#include "Prefix.h"
#include "htString.h"
#include "List.h"
#include "StringMatch.h"
#include "Configuration.h"

extern Configuration	config;


//*****************************************************************************
// Prefix::Prefix()
//
Prefix::Prefix()
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

    String	stripped = w;
    HtStripPunctuation(stripped);
    w = stripped.get();

    char 	*prefix_suffix = config["prefix_match_character"];
    int 	prefix_suffix_length = prefix_suffix == NULL 
					? 0 : strlen(prefix_suffix);
    int 	minimum_prefix_length = config.Value("minimum_prefix_length");

    if (debug)
         cerr << " word=" << w << " prefix_suffix=" << prefix_suffix 
		<< " prefix_suffix_length=" << prefix_suffix_length
		<< " minimum_prefix_length=" << minimum_prefix_length << "\n";

    if (strlen(w) < minimum_prefix_length + prefix_suffix_length)
	return;

    //  A null prefix character means that prefix matching should be 
    //  applied to every search word; otherwise return if the word does 
    //	not end in the prefix character(s).
    //
    if (prefix_suffix_length > 0
	    && strcmp(prefix_suffix, w+strlen(w)-prefix_suffix_length)) 
	return;

    WordList	wordDB;
    if (wordDB.Read(config["word_db"]) == NOTOK)
      return;

    int		wordCount = 0;
    int		maximumWords = config.Value("max_prefix_matches", 1000);
    String	*s;
    int		len = strlen(w) - prefix_suffix_length;
    
    // Strip the prefix character(s)
    char w2[8192];
    strncpy(w2, w, sizeof(w2) - 1);
    w2[sizeof(w2) - 1] = '\0';
    w2[strlen(w2) - prefix_suffix_length] = '\0';
    String w3(w2);
    w3.lowercase();
    List	*wordList = wordDB[w3.get()];

    while (wordCount < maximumWords && (s = (String *) wordList->Get_Next()))
    {
	if (mystrncasecmp(s->get(), w, len))
	    break;
	words.Add(new String(*s));
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
Prefix::openIndex(Configuration &)
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




