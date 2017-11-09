//
// WeightWord.cc
//
// WeightWord: Contains the information necessary for a particular search word
//             including the resulting weight (scaling factor) and 
//             whether the word should be hidden (ignored).
//
// Part of the ht://Dig package   <http://www.htdig.org/>
// Copyright (c) 1995-2004 The ht://Dig Group
// For copyright details, see the file COPYING in your distribution
// or the GNU Library General Public License (LGPL) version 2 or later
// <http://www.gnu.org/copyleft/lgpl.html>
//
// $Id: WeightWord.cc,v 1.10 2004/05/28 13:15:24 lha Exp $
//

#ifdef HAVE_CONFIG_H
#include "htconfig.h"
#endif /* HAVE_CONFIG_H */

#include "WeightWord.h"

#ifdef HAVE_STD
#include <fstream>
#ifdef HAVE_NAMESPACES
using namespace std;
#endif
#else
#include <fstream.h>
#endif /* HAVE_STD */

//***************************************************************************
// WeightWord::WeightWord()
//
WeightWord::WeightWord()
{
    weight = 1;
    records = 0;
    isExact = 0;
    isHidden = 0;
    isIgnore = 0;

    flags = FLAGS_MATCH_ONE;
}


//***************************************************************************
// WeightWord::WeightWord(WeightWord *ww)
//
WeightWord::WeightWord(WeightWord *ww)
{
    weight = ww->weight;
    records = ww->records;
    isExact = ww->isExact;
    isHidden = ww->isHidden;
    flags = ww->flags;
    word = ww->word;
    isIgnore = 0;
}


//***************************************************************************
// WeightWord::WeightWord(char *word, double weight)
//
WeightWord::WeightWord(char *word, double weight)
{
    records = 0;
    isExact = 0;
    isHidden = 0;
    isIgnore = 0;

    // allow a match with any field
    flags = FLAGS_MATCH_ONE;

    set(word);
    this->weight = weight;
}

//***************************************************************************
// WeightWord::WeightWord(char *word, double weight, unsigned int f)
//
WeightWord::WeightWord(char *word, double weight, unsigned int f)
{
    records = 0;

    flags = f;
    // if no fields specified, allow a match with any field
    if (!(flags & FLAGS_MATCH_ONE))
	flags ^= FLAGS_MATCH_ONE;

    // ideally, these flags should all just be stored in a uint...
    isExact = ((flags & FLAG_EXACT) != 0);
    isHidden = ((flags & FLAG_HIDDEN) != 0);
    isIgnore = ((flags & FLAG_IGNORE) != 0);

    set(word);
    this->weight = weight;
}


//***************************************************************************
// WeightWord::~WeightWord()
//
WeightWord::~WeightWord()
{
}


//***************************************************************************
// void WeightWord::set(char *word)
//
void WeightWord::set(char *word)
{
#if 0
    isExact = 0;
    isHidden = 0;
    while (strchr(word, ':'))
    {
	//
	// This word contains modifiers.
	//
	if (mystrncasecmp(word, "exact:", 6) == 0)
	{
	    word += 6;
	    isExact = 1;
	}
	else if (mystrncasecmp(word, "hidden:", 7) == 0)
	{
	    word += 7;
	    isHidden = 1;
	}
	else
	{
	    //
	    // There is a ':' but not a valid attribute.  It must be part
	    // of the word we are searching for.
	    //
	    break;
	}
		
    }
#endif
    this->word = word;
    this->word.lowercase();
}

