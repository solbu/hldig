//
// Speling.h
//
// Speling: (sic) Performs elementary (one-off) spelling correction for ht://Dig
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1999 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Public License version 2 or later 
// <http://www.gnu.org/copyleft/gpl.html>
//
// $Id: Speling.cc,v 1.5 1999/09/24 10:29:02 loic Exp $
//

#include "Speling.h"
#include "htString.h"
#include "List.h"
#include "StringMatch.h"
#include "Configuration.h"

#include <fstream.h>
#include <stdio.h>

//*****************************************************************************
// Speling::Speling(const Configuration& config_arg)
//
Speling::Speling(const Configuration& config_arg) :
  Fuzzy(config_arg)
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
// A fairly efficient one-off spelling checker
// This generates the small list of possibilities and
// checks to see if they exist...
//
void
Speling::getWords(char *w, List &words)
{
    if ((int)strlen(w) < config.Value("minimum_speling_length",5))
	return;

    WordList	wordDB(config);
    if (wordDB.Open(config["word_db"], O_RDONLY) == NOTOK)
      return;

    String	initial = w;
    String	stripped = initial;
    HtStripPunctuation(stripped);
    String	tail;
    int		max_length = stripped.length() - 1;

    for (int pos = 0; pos < max_length; pos++)
    {
      // First transposes
      // (these are really common)
      initial = stripped;
      char	temp = initial[pos];
      initial[pos] = initial[pos+1];
      initial[pos+1] = temp;
      
      if (!wordDB.Exists(initial))   // Seems weird, but this is correct
	words.Add(new String(initial));

      // Now let's do deletions
      initial = stripped;
      tail = initial.sub(pos+1);
      if (pos > 0)
	{
	  initial = initial.sub(0, pos);
	  initial += tail;
	}
      else
	initial = tail;

      if (!wordDB.Exists(initial))   // Seems weird, but this is correct
	words.Add(new String(initial));
    }

    // One last deletion -- check the last character!
    initial = stripped;
    initial = initial.sub(0, initial.length() - 1);
    
    if (!wordDB.Exists(initial))   // Seems weird, but this is correct
      words.Add(new String(initial));    
    
    wordDB.Close();
}


//*****************************************************************************
int
Speling::openIndex()
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




