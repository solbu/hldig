//
// Speling.h
//
// Interface to the Speling [sic] fuzzy algorithm.
// Performs elementary (one-off) spelling correction for ht://Dig
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Speling.cc,v 1.1 1999/05/15 15:38:47 ghutchis Exp $
//
//
#include "Speling.h"
#include "htString.h"
#include "List.h"
#include "StringMatch.h"
#include "Configuration.h"
#include <fstream.h>
#include <stdio.h>

extern Configuration	config;


//*****************************************************************************
// Speling::Speling()
//
Speling::Speling()
{
    name = "speling";
}


//*****************************************************************************
// Speling::~Speling()
//
Speling::~Speling()
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
Speling::getWords(char *w, List &words)
{
    if (strlen(w) < config.Value("minimum_speling_length",5))
	return;

    Database	*dbf = Database::getDatabaseInstance(DB_BTREE);
    dbf->OpenRead(config["word_db"]);

    String	initial(w);
    String	tail;
    int		max_length = initial.length() - 1;

    for (int pos = 0; pos < max_length; pos++)
    {
      // First transposes
      // (these are really common)
      initial = w;
      char	temp = initial[pos];
      initial[pos] = initial[pos+1];
      initial[pos+1] = temp;
      
      if (!dbf->Exists(initial))   // Seems weird, but this is correct
	words.Add(new String(initial));

      // Now let's do deletions
      initial = w;
      tail = initial.sub(pos+1);
      if (pos > 0)
	{
	  initial = initial.sub(0, pos);
	  initial += tail;
	}
      else
	initial = tail;

      if (!dbf->Exists(initial))   // Seems weird, but this is correct
	words.Add(new String(initial));
    }

    // One last deletion -- check the last character!
    initial = w;
    initial = initial.sub(0, initial.length() - 1);
    
    if (!dbf->Exists(initial))   // Seems weird, but this is correct
      words.Add(new String(initial));    
    
    dbf->Close();
    delete dbf;
}


//*****************************************************************************
int
Speling::openIndex(Configuration &)
{
  return 0;
}


//*****************************************************************************
void
Speling::generateKey(char *, String &)
{
}


//*****************************************************************************
void
Speling::addWord(char *)
{
}




